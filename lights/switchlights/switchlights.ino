#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WS2812FX.h>
#include "Timer.h"

#define LED_COUNT 150
#define LED_PIN 14
#define INPUT_TRIGGER_PIN 5

WiFiManager wifiManager;
Timer checkStateResetTimer(500);
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void configureOTA() {
  // Make sure the flash size matches. For ESP8266 12F it should be 4MB.
  ArduinoOTA.setHostname("wonkalights");
  
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

  pinMode(INPUT_TRIGGER_PIN, INPUT);

  ws2812fx.init();
  ws2812fx.setBrightness(50);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(ORANGE);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  ws2812fx.setCustomMode(oscillate);

  ws2812fx.setSegment(0, 0, 25, FX_MODE_CUSTOM, ORANGE, 1000, false);
  
  connectToWifi();
  configureOTA();
}

typedef struct Oscillator2 {
  int16_t pos;
  int8_t  size;
  int8_t  dir;
  int8_t  speed;
} oscillator;

uint16_t oscillate(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  int startFirstArrow = seg->stop - 9;
  int endFirstArrow = seg->stop - 5;

  int startSecondArrow = seg->stop - 4;
  int endSecondArrow = seg->stop;
  
  int seglen = seg->stop - seg->start + 1 - 10;

  static Oscillator2 oscillator = {0, 2, 1, 1};

  oscillator.pos += oscillator.dir * oscillator.speed;
  if ((oscillator.pos >= (seglen - 1))) {
    oscillator.pos = 0;
    oscillator.speed = 1;

    for (int x = 0; x < 5; x++) {
      ws2812fx.setPixelColor(startFirstArrow + x, BLACK);
      ws2812fx.setPixelColor(startSecondArrow + x, BLACK);
    }

    ws2812fx.setCycle();
  }

  for (int8_t i=0; i < seglen; i++) {
    uint32_t color = BLACK;
    
    if(i >= oscillator.pos - oscillator.size && i <= oscillator.pos + oscillator.size) {
      color = (color == BLACK) ? seg->colors[0] : ws2812fx.color_blend(color, seg->colors[0], 128);
    }

    int numberOn = oscillator.pos == 0 ? 0 : (oscillator.pos - (seglen - 1 - 5 - oscillator.size));

    if (numberOn > 0) {
      for (int x = 0; x < numberOn; x++) {
        ws2812fx.setPixelColor(endFirstArrow - x, seg->colors[0]);
        ws2812fx.setPixelColor(endSecondArrow - x, seg->colors[0]);
      }
    }
      
    ws2812fx.setPixelColor(seg->start + i, color);
  }
  
  return(seg->speed / 8);
}

#define TIMER_MS 15000
unsigned long last_change = 0;
unsigned long now = 0;

void loop() {
  ArduinoOTA.handle();

  ws2812fx.service();

  if (!checkStateResetTimer.IsRunning() && digitalRead(INPUT_TRIGGER_PIN) == HIGH) {
    checkStateResetTimer.Start();
    ws2812fx.stop();
  }
  
  if (checkStateResetTimer.Check() && digitalRead(INPUT_TRIGGER_PIN) != HIGH) {
    checkStateResetTimer.Stop();
    ws2812fx.start();
  }
}
