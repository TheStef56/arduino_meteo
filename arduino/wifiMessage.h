#include "common.h"
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void shutDownWifi() {
  while (WiFi.status() == WL_CONNECTED) {
    IF_SERIAL_DEBUG(Serial.print("Attempting to shutdown WIFI..."));
    WiFi.end();
    delay(5000);
    IF_SERIAL_DEBUG(Serial.println("Wifi stopped!"));
  }
}

void setupWifi() {
  if (WiFi.status() == WL_NO_MODULE) {
    IF_SERIAL_DEBUG(Serial.println("Communication with WiFi module failed!"));
    while (true) {
      ledPrint("    Communication with WiFi module failed!", true);
    };
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    IF_SERIAL_DEBUG(Serial.println("Please upgrade the firmware"));
    ledPrint("    Please upgrade the firmware", true);
  }

  int status = WiFi.status();
  while (status != WL_CONNECTED) {
    IF_SERIAL_DEBUG(Serial.print("Attempting to connect to SSID: "));
    IF_LED_DEBUG(ledPrint("    Attempting to connect to WIFI...", true));
    IF_SERIAL_DEBUG(Serial.println(WIFI_SSID));
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
  }
  IF_SERIAL_DEBUG(printWifiStatus());
}

int sendWifiMessage(const char *ip, int port, uint8_t *message, size_t size) {
  while (1) {  
    WiFiClient client;  
    int res = client.connect(ip, port);
    if (res) {
      IF_SERIAL_DEBUG(Serial.println("Connected to socket!"));
      IF_LED_DEBUG(ledPrint("    Connection to socket!", true));
      client.write(message, size);
      client.stop();
      return 1;
    } else {
      IF_SERIAL_DEBUG(Serial.print("Connection to socket failed: "));
      IF_SERIAL_DEBUG(Serial.println(res));
      IF_SERIAL_DEBUG(printWifiStatus());
      IF_LED_DEBUG(ledPrint("    Connection to socket failed!", true));
      shutDownWifi();
      setupWifi();
    }
  }
}

int sendWifiMessageEnc(const char *ip, int port, uint8_t *message, size_t size, uint8_t *aes_key, size_t aes_size) {
  AES128 aes;          // AES-128 (16-byte key)
  GCM<AES128> gcm;     // GCM mode
  
  uint8_t nonce[12];
  uint8_t authTag[16];
  uint8_t encrypted[size];
  
  for(int i=0;i<12;i++) nonce[i] = random(0, 256);

  gcm.clear();
  gcm.setKey(aes_key, aes_size);
  gcm.setIV(nonce, sizeof(nonce)); // Set the nonce/IV
  gcm.addAuthData(NULL, 0);    // Optional, no extra authenticated data
  gcm.encrypt((uint8_t*)&encrypted, message, size); // Encrypt payload
  gcm.computeTag(authTag, sizeof(authTag)); 
  
  uint8_t fullPayload[sizeof(encrypted) + sizeof(nonce) + sizeof(authTag)];
  memcpy(fullPayload                                     , &encrypted,  sizeof(encrypted));
  memcpy(fullPayload + sizeof(encrypted)                 , &nonce     , sizeof(nonce));
  memcpy(fullPayload + sizeof(encrypted) +  sizeof(nonce), &authTag  ,  sizeof(authTag));

  return sendWifiMessage(ip, port, (byte*)(void*)&fullPayload, sizeof(fullPayload));
}