#ifndef SIM_MESSAGE_H
#define SIM_MESSAGE_H

#include "ctx.h"
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>

bool sendATCommand(const char *cmd, unsigned long timeoutMs = 2000) {
    Serial1.println(cmd);
    unsigned long start = millis();
    bool okReceived = false;

    while (millis() - start < timeoutMs) {
        while (Serial1.available()) {
            String line = Serial1.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;
            IF_SERIAL_DEBUG(Serial.println(line));

            if (line == "OK") okReceived = true;
            if (line == "ERROR") return false;
        }
    }
    return okReceived;
}

int sendSimMessage(const char *ip, int port, uint8_t *message, size_t size, unsigned long timeoutMs = 10000) {
    char buf[128];
    bool success = false;

    sendATCommand("AT");                                            // wake module
    sendATCommand("ATE0");                                          // disable echo
    sendATCommand("AT+CFUN=1");                                     // full functionality
    snprintf(buf, 128, "AT+CGDCONT=1,\"IP\",\"internet.wind\"");    // set APN
    sendATCommand(buf);
    sendATCommand("AT+CSQ");                                        // signal quality
    sendATCommand("AT+CREG?");                                      // network registration
    sendATCommand("AT+CIPMODE=1");                                  // transparentmode
    sendATCommand("AT+NETOPEN");                                    // start tcpcip socket mode
    snprintf(buf, 128, "AT+CIPOPEN=0,\"TCP\",\"%s\",%d", ip, port);
    sendATCommand(buf);                                             // connect to endpoint
    Serial1.write(message, size);                                   // send data
    Serial1.flush();                                                     
    delay(1000); 
    sendATCommand("AT+CIPCLOSE=0");                                 // close socket
    sendATCommand("AT+NETCLOSE");                                   // stop tcpip socket mode
    sendATCommand("AT+CSCLK=1");                                    // go to sleep
    return 0;
}

// --- Encrypted send ---
void sendSimMessageEnc(const char *ip, int port, uint8_t *message, size_t size, uint8_t *aes_key, size_t aes_size) {
    GCM<AES256> gcm;

    uint8_t nonce[12];
    uint8_t authTag[16];
    uint8_t encrypted[size];

    for (int i = 0; i < 12; i++) nonce[i] = random(0, 256);

    gcm.clear();
    gcm.setKey(aes_key, aes_size);
    gcm.setIV(nonce, sizeof(nonce));
    gcm.addAuthData(NULL, 0);
    gcm.encrypt(encrypted, message, size);
    gcm.computeTag(authTag, sizeof(authTag));

    uint8_t fullPayload[sizeof(encrypted) + sizeof(nonce) + sizeof(authTag)];
    memcpy(fullPayload, encrypted, sizeof(encrypted));
    memcpy(fullPayload + sizeof(encrypted), nonce, sizeof(nonce));
    memcpy(fullPayload + sizeof(encrypted) + sizeof(nonce), authTag, sizeof(authTag));

    sendSimMessage(ip, port, fullPayload, sizeof(fullPayload));
}

#endif // SIM_MESSAGE_H
