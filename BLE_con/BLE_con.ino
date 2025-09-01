#include <Arduino.h>
#include <Arduino_BHY2.h>
#include <ArduinoBLE.h>
#include "Nicla_System.h"
#include <MemoryFree.h>

#define PRINT(X) Serial.println(F(X));
#define PRINT_INT(X) Serial.println(X);

#define PRINT_2(X, Y) Serial.print(F(X)); Serial.println(Y);
#define PRINT_3_HEX(X,Y,Z) Serial.print(X,16); Serial.print(F(Y)); Serial.println(Z,16);

//=================================UUIDS===============================================
const char* envServiceUuid =                      "0000181A-0000-1000-8000-00805F9B34FB";
const char* otherServiceUuid =                    "00001000-0000-1000-8000-00805F9B34FB";

const char* temp_CharacteristicUuid =             "00002A6E-0000-1000-8000-00805F9B34FB";
const char* Humidity_CharacteristicUuid =         "00002A6F-0000-1000-8000-00805F9B34FB";
const char* IAQ_CharacteristicUuid =              "00002AF2-0000-1000-8000-00805F9B34FB";
const char* bVOC_CharacteristicUuid =             "00002BE7-0000-1000-8000-00805F9B34FB";
const char* CO2_CharacteristicUuid =              "00002B8C-0000-1000-8000-00805F9B34FB";
const char* Pressure_CharacteristicUuid =         "00002780-0000-1000-8000-00805F9B34FB";
const char* Step_CharacteristicUuid =             "000027BA-0000-1000-8000-00805F9B34FB";
const char* messageReceiver_CharacteristicUuid =  "00001001-0000-1000-8000-00805F9B34FB";


//=================================Sensors=====================================================
Sensor temp(SENSOR_ID_TEMP);
Sensor humidity(SENSOR_ID_HUM);
Sensor pressure(SENSOR_ID_BARO);
SensorActivity activity(SENSOR_ID_AR);
SensorBSEC bsec(SENSOR_ID_BSEC);
Sensor stepCounter(SENSOR_ID_STC);

//=================================BLE-Service-Characteristics=====================================================
BLEService envService(envServiceUuid);

BLEShortCharacteristic temp_Characteristic(temp_CharacteristicUuid, BLERead | BLENotify);
BLEShortCharacteristic Humidity_Characteristic(Humidity_CharacteristicUuid, BLERead | BLENotify);
BLEShortCharacteristic IAQ_Characteristic(IAQ_CharacteristicUuid, BLERead | BLENotify);
BLEShortCharacteristic CO2_Characteristic(CO2_CharacteristicUuid,BLERead | BLENotify);
BLEShortCharacteristic Pressure_Characteristic(Pressure_CharacteristicUuid,BLERead | BLENotify);

BLEService otherService(otherServiceUuid);

BLEShortCharacteristic bVOC_Characteristic(bVOC_CharacteristicUuid, BLERead | BLENotify);
BLEShortCharacteristic Step_Characteristic(Step_CharacteristicUuid, BLERead | BLENotify);
BLEShortCharacteristic messageReceiver(messageReceiver_CharacteristicUuid, BLEWrite | BLENotify);


//=======================variable definitions==================================
bool run = false;
bool advertising = false;

const short sensorWaitms = 1000;
const short blinkWaitms = 300;

unsigned long lastSensorUpdate = 0;
unsigned long lastBlink_run = 0;
unsigned long lastBlink_advertise = 0;

//=======================function definitions==================================
void start();
void stop();
void restart();
void LED_setColor(RGBColors);

//=======================CallBacks==================================

//for when a central device connects.
void onBleConnected(BLEDevice central) {
  PRINT_2("Connected to central: ",central.address());
  advertising = false;
  LED_setColor(blue);
}
void onBleDisconnected(BLEDevice central) {

  PRINT_2("Disconnected from central: ",central.address());
  stop();
  LED_setColor(red);
  BLE.advertise();
  advertising = true;
}

// callback for message received
void onMessageReceived(BLEDevice central, BLECharacteristic characteristic) {
  if (characteristic.written()) {
    switch (messageReceiver.value()) {
      case 1: start(); break;
      case 2: stop(); break;
      case 3: restart(); break;
      default: PRINT_2("Unrecognized message.... : ", messageReceiver.value()); break;
    }
    PRINT_2("Message Received: ",messageReceiver.value());
  }
}

//==========================Run functions==============================================
void start(){
  run = true;
}
void stop(){
  run = false;
  LED_setColor(blue);
}
void restart(){
  stop();
  // TODO restart stuff
  start();
}

void blink(RGBColors color) {
  static bool state = false;
  if (state) {
    LED_setColor(off);
    state = !state;
  } else {
    LED_setColor(color);
    state = !state;
  }
}
void LED_setColor(RGBColors color){
  nicla::leds.setColor(color);
}

//=======================Setup and loop==================================

void setup() {
  Serial.begin(115200);
  while (!Serial);
  PRINT("Waiting");
  delay(3000); // wait for device to connect through USB

  Serial.println("-------------------------------------------");
  printThreadStacks();
  Serial.println("-------------------------------------------");

  PRINT("Nicla LED init...."); 
  nicla::begin();
  nicla::leds.begin();
  LED_setColor(red);
  printFreeRAM();

  PRINT("Sensors...."); printFreeRAM();
  BHY2.begin(NICLA_BLE);printFreeRAM();
  temp.begin();printFreeRAM();
  humidity.begin();printFreeRAM();
  pressure.begin();printFreeRAM();
  activity.begin();printFreeRAM();
  bsec.begin();printFreeRAM();
  stepCounter.begin();printFreeRAM();

  PRINT("BLE....");
  if (!BLE.begin()) {
    PRINT("Starting BLE failed!");
    while (1);
  }
    
  BLE.setLocalName("Nicla");printFreeRAM();
  
  envService.addCharacteristic(temp_Characteristic);printFreeRAM();
  envService.addCharacteristic(Humidity_Characteristic);printFreeRAM();
  envService.addCharacteristic(IAQ_Characteristic);printFreeRAM();
  envService.addCharacteristic(CO2_Characteristic);printFreeRAM();
  envService.addCharacteristic(Pressure_Characteristic);printFreeRAM();

  otherService.addCharacteristic(bVOC_Characteristic);printFreeRAM();
  otherService.addCharacteristic(Step_Characteristic);printFreeRAM();
  otherService.addCharacteristic(messageReceiver);printFreeRAM();

  messageReceiver.setEventHandler(BLEWritten, onMessageReceived);
  BLE.setEventHandler(BLEConnected, onBleConnected);
  BLE.setEventHandler(BLEDisconnected, onBleDisconnected);
  
  PRINT("Adding Services");printFreeRAM();
  BLE.addService(envService);printFreeRAM();
  BLE.addService(otherService);printFreeRAM();
  BLE.setAdvertisedService(envService);printFreeRAM();
  
  // Start advertising the BLE service.
  BLE.advertise();
  advertising = true;
  PRINT("=== Nicla Sense ME Memory Monitor ===");
}

void loop() {
  BLE.poll();
  BHY2.update();
  // blinking for advertising 
  if(advertising && millis() - lastBlink_advertise >= blinkWaitms){
    lastBlink_advertise = millis();
    blink(blue);
  }

  if (run && BLE.connected() ) {
    // blinking for transfering data
    if (millis() - lastBlink_run >= blinkWaitms){ 
      lastBlink_run = millis();
      blink(green);
    }

    if (millis() - lastSensorUpdate >= sensorWaitms) {
      lastSensorUpdate = millis();

      temp_Characteristic.writeValue(temp.value()*100);
      Humidity_Characteristic.writeValue(humidity.value());
      Pressure_Characteristic.writeValue(pressure.value());
      IAQ_Characteristic.writeValue(bsec.iaq());
      bVOC_Characteristic.writeValue(bsec.b_voc_eq()*100);
      CO2_Characteristic.writeValue(bsec.co2_eq());
      Step_Characteristic.writeValue(stepCounter.value());

      PRINT_2("temp: ",temp.value());
    }
  }

}
