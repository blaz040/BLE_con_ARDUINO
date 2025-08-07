/* * This sketch shows how a Nicla Sense ME can act as a BLE peripheral
 * to send sensor data to a central device (like an MIT App Inventor app).
 * * The temperature sensor is enabled, and its values are periodically
 * read and sent to the central device via a BLE characteristic.
 * * This version uses a non-blocking loop with callbacks for a more reliable
 * and responsive application.
 */

#include "Arduino.h"
#include <ArduinoBLE.h>
#include <Arduino_BHY2.h>
#include "Nicla_System.h"


#define BLE_base_UUID 0x1000800000805F9B34FB
// Initialize the temperature sensor.
Sensor temp(SENSOR_ID_TEMP);
Sensor hum(SENSOR_ID_HUM);

// Define the custom 128-bit UUIDs for your BLE service and characteristics.
// IMPORTANT: These UUIDs must exactly match the ones you use in your MIT App Inventor app.
const char* dataServiceUuid = "0000181A-0000-1000-8000-00805F9B34FB";
const char* tempCharacteristicUuid = "00002A6E-0000-1000-8000-00805F9B34FB";
const char* HumidityCharacteristicUuid = "00002A6F-0000-1000-8000-00805F9B34FB";

// Define the BLE service and characteristics.
// The dataService contains the tempCharacteristic.
BLEService dataService(dataServiceUuid); 

// The tempCharacteristic is set to be readable by the central device (BLERead)
// and can send notifications (BLENotify) when its value changes.
BLEShortCharacteristic tempCharacteristic(tempCharacteristicUuid, BLERead | BLENotify);
BLEShortCharacteristic HumidityCharacteristic(HumidityCharacteristicUuid, BLERead | BLENotify);

// --- CALLBACK FUNCTIONS ---
// These functions are called automatically when a BLE event occurs.

// Callback for when a central device connects.
void onBleConnected(BLEDevice central) {
  Serial.print("Connected to central: ");
  nicla::leds.setColor(blue); // Turn on the blue LED to show a connection
  Serial.println(central.address());
}

// Callback for when a central device disconnects.
void onBleDisconnected(BLEDevice central) {
  Serial.print("Disconnected from central: ");
  nicla::leds.setColor(red); // Turn on the red LED to show disconnection
  Serial.println(central.address());
}

void setup() {
  // Initialize Nicla's core systems and LEDs
  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(off); // Start with LEDs off

  // Initialize the BHY2 sensor system and the temperature sensor
  BHY2.begin();
  temp.begin();
  hum.begin();
  // Start serial communication for debugging
  Serial.begin(9600);
  while(!Serial);

  // Initialize BLE. If it fails, halt the program.
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // Configure the BLE peripheral device.
  BLE.setLocalName("Nicla2");
  BLE.setAdvertisedService(dataService);

  // Add the characteristic to the service.
  dataService.addCharacteristic(HumidityCharacteristic);
  dataService.addCharacteristic(tempCharacteristic);
  
  // Add the service to the BLE peripheral.
  BLE.addService(dataService);

  // Set the initial value of the characteristic.
  // This value will be read by the central device upon connection.
  tempCharacteristic.writeValue(22.0);
  
  // Set the event handlers (callbacks) for connections and disconnections.
  BLE.setEventHandler(BLEConnected, onBleConnected);
  BLE.setEventHandler(BLEDisconnected, onBleDisconnected);

  // Start advertising the BLE service.
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
  Serial.println("Blue LED = Connected, Red LED = Disconnected");
}

void loop() {
    // BLE.poll() allows the BLE library to process events (like connections)
  // and run our callback functions without blocking the main loop.
  BLE.poll(); 
 
  // Store the last time we printed and updated the characteristic.
  static unsigned long lastUpdate = 0;
  
  // Update the sensor data from the BHY2 system.
  // This needs to be done regularly to get the latest values.
  BHY2.update();
  
  // Update the characteristic and print to the serial monitor every 1 second.
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();

    // The 'BLE.connected()' check is a cleaner way to see if a central is connected.
    if(BLE.connected()) {
      // The central device will be notified of this change if it is subscribed.
      tempCharacteristic.writeValue(temp.value()*100);
      HumidityCharacteristic.writeValue(hum.value());
      
      // Print the value to the serial monitor for debugging.
      Serial.println(String("Temp C: ") + String(temp.value()));
      Serial.println(String("Humidity : ") + String(hum.value()));
    }
  }
  
  // //An alternative, simpler, but blocking way to handle connections (less ideal for
  // //continuous sensor reading):
  // //BLE.poll(); 
  // BLEDevice central = BLE.central(); // This line blocks until a connection is made
  // Serial.println("Waitting for connection");
  // if (central) {
  //   Serial.print("Connected to central: ");
  //   nicla::leds.setColor(blue);
  //   Serial.println(central.address());
  //   
  //   while (central.connected()) {
  //     BHY2.update();
  //     float tempF = 1.8 * temp.value() + 32.0;
  //     tempCharacteristic.writeValue(tempF);
  //     Serial.println("T: "+ String(tempF));
  //     delay(100);
  //   }
  //
  //   Serial.print("Disconnected from central: ");
  //   nicla::leds.setColor(red);
  //   Serial.println(central.address());
  // }
}