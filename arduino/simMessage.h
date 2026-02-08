#ifndef SIM_MESSAGE_H
#define SIM_MESSAGE_H

#include "ctx.h"
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>

bool wakeSimModule(char *buf) {
  Serial1.println("AT");
  delay(200);
  Serial1.println("ATE0");
  delay(200);
  Serial1.println("AT+CFUN=1");
  delay(500);
  snprintf(buf, 128, "AT+CGDCONT=1,\"IP\",\"internet.wind\"");
  Serial1.println(buf);
  delay(500);
  return true;
}

bool openLTE(unsigned long timeoutMs) {
  Serial1.println("AT+NETOPEN");
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial1.available()) {
      String line = Serial1.readStringUntil('\n');
      line.trim();
      if (line.startsWith("+NETOPEN: 0")) return true;
    }
  }
  IF_SERIAL_DEBUG(Serial.println("NETOPEN failed"));
  IF_LED_DEBUG(ledPrint("NETOPEN failed", true));
  return false;
}

bool openTCP(const char *ip, int port, char *buf, unsigned long timeoutMs) {
  snprintf(buf, 128, "AT+CIPOPEN=0,\"TCP\",\"%s\",%d", ip, port);
  Serial1.println(buf);

  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial1.available()) {
      String line = Serial1.readStringUntil('\n');
      line.trim();
      if (line.startsWith("+CIPOPEN: 0,0")) return true;
      if (line.indexOf("ERROR") >= 0) break;
    }
  }
  IF_SERIAL_DEBUG(Serial.println("Socket open failed"));
  IF_LED_DEBUG(ledPrint("Socket open failed", true));
  Serial1.println("AT+NETCLOSE");
  delay(200);
  return false;
}

bool sendSimData(uint8_t *message, size_t size, char *buf) {
  snprintf(buf, 128, "AT+CIPSEND=0,%u", (unsigned int)size);
  Serial1.println(buf);

  unsigned long start = millis();
  bool prompt = false;
  while (millis() - start < 5000) {
    if (Serial1.available() && Serial1.read() == '>') {
      prompt = true;
      break;
    }
  }
  if (!prompt) {
    IF_SERIAL_DEBUG(Serial.println("No > prompt"));
    IF_LED_DEBUG(ledPrint("No > prompt", true));
    Serial1.println("AT+CIPCLOSE=0");
    Serial1.println("AT+NETCLOSE");
    delay(200);
    return false;
  }

  Serial1.write(message, size);
  Serial1.write(0x1A); // Ctrl+Z

  start = millis();
  while (millis() - start < 5000) {
    if (Serial1.available()) {
      String line = Serial1.readStringUntil('\n');
      line.trim();
      if (line == "SEND OK") return true;
    }
  }
  IF_SERIAL_DEBUG(Serial.println("SEND failed"));
  IF_LED_DEBUG(ledPrint("SEND failed", true));
  Serial1.println("AT+CIPCLOSE=0");
  Serial1.println("AT+NETCLOSE");
  delay(200);
  return false;
}

bool waitServerAck(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial1.available()) {
      String line = Serial1.readStringUntil('\n');
      line.trim();
      if (line.indexOf("ACK") >= 0) return true;
    }
  }
  IF_SERIAL_DEBUG(Serial.println("No server ACK"));
  IF_LED_DEBUG(ledPrint("No server ACK", true));
  Serial1.println("AT+CIPCLOSE=0");
  Serial1.println("AT+NETCLOSE");
  delay(200);
  return false;
}

int sendSimMessage(const char *ip, int port, uint8_t *message, size_t size, unsigned long timeoutMs = 10000) {
  char buf[128];
  bool success = false;

  while (!success) {
    wakeSimModule(buf);

    if (!openLTE(timeoutMs)) continue;
    if (!openTCP(ip, port, buf, timeoutMs)) continue;
    if (!sendSimData(message, size, buf)) continue;
    if (!waitServerAck(timeoutMs)) continue;

    success = true;
    Serial1.println("AT+CIPCLOSE=0");
    delay(200);
    Serial1.println("AT+NETCLOSE");
    delay(200);
    Serial1.println("AT+CSCLK=1");
  }
  return 0;
}

void sendSimMessageEnc(const char *ip, int port, uint8_t *message, size_t size, uint8_t *aes_key, size_t aes_size) {
  GCM<AES256> gcm;

  uint8_t nonce[12];
  uint8_t authTag[16];
  uint8_t encrypted[size];

  // generate random nonce
  for (int i = 0; i < 12; i++) nonce[i] = random(0, 256);

  gcm.clear();
  gcm.setKey(aes_key, aes_size);
  gcm.setIV(nonce, sizeof(nonce));      // set nonce
  gcm.addAuthData(NULL, 0);             // optional AAD
  gcm.encrypt(encrypted, message, size); // encrypt payload
  gcm.computeTag(authTag, sizeof(authTag));

  // combine encrypted + nonce + authTag
  uint8_t fullPayload[sizeof(encrypted) + sizeof(nonce) + sizeof(authTag)];
  memcpy(fullPayload, encrypted, sizeof(encrypted));
  memcpy(fullPayload + sizeof(encrypted), nonce, sizeof(nonce));
  memcpy(fullPayload + sizeof(encrypted) + sizeof(nonce), authTag, sizeof(authTag));

  // send via SIM7600
  sendSimMessage(ip, port, fullPayload, sizeof(fullPayload));
}

#endif // SIM_MESSAGE_H
