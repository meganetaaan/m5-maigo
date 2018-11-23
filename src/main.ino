#include <M5Stack.h>
#include "Free_Fonts.h"
#include <Wire.h>
#include <AquesTalkTTS.h>
#include <Avatar.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <tasks/LipSync.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool lost = false;
double t0, t1;

using namespace m5avatar;

Avatar *avatar;
int count = 0;
float f = 0;
float last = 0;

void talkLoop(void *args)
{
  int i = 0;
  for(;;)
  {
    if (lost && TTS.getLevel() == 0)
    {
      if (i % 3 == 0)
      {
        TTS.play("o'-i,to'-chanwa/do'ko?", 90);
      }
      else if (i % 3 == 1)
      {
        TTS.play("ta'roukunn.to'-channo/saga_shite?", 90);
      }
      else
      {
        TTS.play("isshoni/to-cha-nn/tte/yondemite", 90);
      }
    }
    i++;
    delay(6000);
  }
}

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      // M5.Lcd.fillRect(0, 0, 240, 30, TFT_BLACK);
      // M5.Lcd.drawString("connected", 10, 220);
      avatar->setExpression(Expression::Happy);
      TTS.play("ta'roukunn.to'-channno/_chika'kude/asobo'-.", 80);
      delay(3000);
      avatar->setExpression(Expression::Neutral);
      deviceConnected = true;
      lost = false;
    };

    void onDisconnect(BLEServer* pServer) {
      // M5.Lcd.fillRect(0, 0, 240, 30, TFT_BLACK);
      // M5.Lcd.drawString("disconnected", 10, 220);
      avatar->setExpression(Expression::Sad);
      TTS.play("to'-chan,do'kka/icchatta'yo-.", 100);
      deviceConnected = false;
      lost = true;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      double fValue;
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);
        Serial.println();
        Serial.println("*********");
        fValue = ::atof(rxValue.c_str());
        if (fValue > 0.0) {
          Serial.println("Changed seaLevel to " + String(fValue));
        }
      }
    }
};

void setup() {
  M5.begin();
  // delay(1000);
  int iret;
  iret = TTS.create(NULL);
  avatar = new Avatar();
  avatar->init();
  avatar->addTask(lipSync, "lipSync");
  xTaskCreate(
                    talkLoop,     /* Function to implement the task */
                    "talkLoop",   /* Name of the task */
                    4096,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    1,         /* Priority of the task */
                    NULL);//,      /* Task handle. */
                    // 0);        /* Core where the task should run */
  M5.Lcd.setBrightness(30);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setFreeFont(FSSB12);
  // Select the font: FreeMono18pt7b – see Free_Fonts.h
  M5.Lcd.setFreeFont(FSS9); // FreeSans9pt7b
  // Create the BLE Device
  BLEDevice::init("Mai5");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  t1 = millis() - t0;
  if (t1 > 4999) {
    t0 = millis();
  }
  M5.update(); // 好importantですね！
  // If not the buttons status is not updated lo.
}
