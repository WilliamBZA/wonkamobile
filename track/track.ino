#include <Unistep2.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WS2812FX.h>
#include "Timer.h"

const int stepsPerRevolution = 4096;
#define LED_COUNT 60
#define INPUT_PIN 13
#define OUTPUT_PIN 12
#define LED_PIN 14

// Wiring diagram http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/boards.html

Timer waitAtBottomTimer(5000);
WiFiManager wifiManager;
Unistep2 stepper(5, 4, 2, 0, stepsPerRevolution, 1250); // pins for IN1, IN2, IN3, IN4, steps per rev, step delay(in micros)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  pinMode(INPUT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);

  ws2812fx.init();
  ws2812fx.setBrightness(50);
  ws2812fx.setSpeed(25000);
  ws2812fx.setColor(WHITE);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW_WHITE);
  
  connectToWifi();
  configureOTA();
}

void configureOTA() {
  // Make sure the flash size matches. For ESP8266 12F it should be 4MB.
  ArduinoOTA.setHostname("wonkalift");
  
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

bool isMovingDown = false;
bool isMovingUp = false;

void loop() {
  ArduinoOTA.handle();
  stepper.run();
  ws2812fx.service();

  if (!isMovingDown && !isMovingUp && digitalRead(INPUT_PIN) == HIGH) {
    ws2812fx.setSpeed(25000);
    ws2812fx.setColor(WHITE);
    ws2812fx.setMode(FX_MODE_CHASE_RAINBOW_WHITE);
    ws2812fx.start();

    digitalWrite(OUTPUT_PIN, HIGH);

    isMovingDown = true;
    Serial.println("Moving down");
    stepper.move(-20000);
  }

  if (isMovingDown && stepper.stepsToGo() == 0 && !waitAtBottomTimer.IsRunning()) {
    waitAtBottomTimer.Start();
    ws2812fx.setColor(PINK);
    ws2812fx.setSpeed(1);
    ws2812fx.setMode(FX_MODE_BREATH);
    ws2812fx.start();
    
    Serial.println("At bottom, waiting...");
  }

  if (waitAtBottomTimer.Check()) {
    waitAtBottomTimer.Stop();
    stepper.move(20000);
    isMovingDown = false;
    isMovingUp = true;
    ws2812fx.stop();
    Serial.println("Moving up");
  }

  if (isMovingUp && stepper.stepsToGo() == 0) {
    isMovingUp = false;
    Serial.println("At top, waiting");
    digitalWrite(OUTPUT_PIN, LOW);
    ws2812fx.stop();
  }
}
