#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WS2812FX.h>

WiFiManager wifiManager;

#define LED_COUNT 150
#define LED_PIN 14

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void configureOTA() {
  // Make sure the flash size matches. For ESP8266 12F it should be 4MB.
  ArduinoOTA.setHostname("wonkaambient");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void connectToWifi() {
  wifiManager.autoConnect("wonkamobile");
}

void setup() {
  Serial.begin(115200);

  ws2812fx.init();
  ws2812fx.setBrightness(100                                                                                                                                                                                                             );
  ws2812fx.setSpeed(3000);
  ws2812fx.setColor(WHITE);
  ws2812fx.setMode(FX_MODE_TWINKLE_FADE);
  ws2812fx.start();
  
  connectToWifi();
  configureOTA();
}

//#define TIMER_MS 15000
//unsigned long last_change = 0;
//unsigned long now = 0;

void loop() {
  ArduinoOTA.handle();

  //now = millis();

  ws2812fx.service();

  //if(now - last_change > TIMER_MS) {
  //  ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
  //  last_change = now;
  //}
}
