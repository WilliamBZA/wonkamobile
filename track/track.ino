#include <Unistep2.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <Servo.h>

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

const int stepsPerRevolution = 4096;

// Wiring diagram http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/boards.html

WiFiManager wifiManager;
Unistep2 stepper(D1, D2, D4, D3, stepsPerRevolution, 1250); // pins for IN1, IN2, IN3, IN4, steps per rev, step delay(in micros)
Servo servo;

void setup() {
  servo.attach(D8);
  servo.write(90);
  
  Serial.begin(115200);
  
  connectToWifi();
  configureOTA();
}

void configureOTA() {
  // Make sure the flash size matches. For ESP8266 12F it should be 4MB.
  ArduinoOTA.setHostname("wonka");
  
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

bool isOpen = false;

void loop() {
  ArduinoOTA.handle();
  stepper.run();

  if (stepper.stepsToGo() == 0) { // If stepsToGo returns 0 the stepper is not moving
   stepper.move(10);
  }
}
