#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WS2812FX.h>
#include <Servo.h>
#include "Timer.h"

#define LED_COUNT 150
#define LED_PIN 14
#define INPUT_PIN 13
#define OUTPUT_PIN 5
#define SERVO_PIN 12
#define EFFECT_DURATION 5000

WiFiManager wifiManager;
Servo servo;
Timer resetTimer(EFFECT_DURATION);
Timer outputResetTimer(EFFECT_DURATION + 3000);
Timer servoResetTimer(1000);
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void configureOTA() {
  // Make sure the flash size matches. For ESP8266 12F it should be 4MB.
  ArduinoOTA.setHostname("wonkatrigger");
  
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

  pinMode(INPUT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);

  ws2812fx.init();
  ws2812fx.setBrightness(50);
  ws2812fx.setSpeed(EFFECT_DURATION);
  ws2812fx.setColor(GREEN);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW_WHITE);
  
  servo.attach(SERVO_PIN);
  servo.write(0);
  
  connectToWifi();
  configureOTA();
}

void loop() {
  ArduinoOTA.handle();

  ws2812fx.service();

  if (!resetTimer.IsRunning() && digitalRead(INPUT_PIN) == HIGH) {
    ws2812fx.start();

    digitalWrite(OUTPUT_PIN, HIGH);

    resetTimer.Start();
    outputResetTimer.Start();
  }

  if (servoResetTimer.Check()) {
    servoResetTimer.Stop();
    servo.write(0);
  }

  if (resetTimer.Check()) {
    ws2812fx.stop();

    resetTimer.Stop();
    servoResetTimer.Start();
    servo.write(180);
  }

  if (outputResetTimer.Check()) {
    digitalWrite(OUTPUT_PIN, LOW);
    outputResetTimer.Stop();
  }
}
