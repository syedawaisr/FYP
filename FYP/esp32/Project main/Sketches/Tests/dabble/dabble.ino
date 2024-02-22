#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#include <DabbleESP32.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Dabble.begin("MyESP32");
}

void loop() {
  // put your main code here, to run repeatedly:
  Dabble.processInput();
  Serial.println(Sensor.getAccelerometerYaxis());
  delay(100);
}

// #include "esp_bt_main.h"
// #include "esp_bt_device.h"
 
// bool initBluetooth()
// {
//   if (!btStart()) {
//     Serial.println("Failed to initialize controller");
//     return false;
//   }
 
//   if (esp_bluedroid_init() != ESP_OK) {
//     Serial.println("Failed to initialize bluedroid");
//     return false;
//   }
 
//   if (esp_bluedroid_enable() != ESP_OK) {
//     Serial.println("Failed to enable bluedroid");
//     return false;
//   }
 
// }
 
// void printDeviceAddress() {
 
//   const uint8_t* point = esp_bt_dev_get_address();
 
//   for (int i = 0; i < 6; i++) {
 
//     char str[3];
 
//     sprintf(str, "%02X", (int)point[i]);
//     Serial.print(str);
 
//     if (i < 5){
//       Serial.print(":");
//     }
 
//   }
// }
 
// void setup() {
//   Serial.begin(115200);
 
//   initBluetooth();
//   printDeviceAddress();
// }
 
// void loop() {
//   printDeviceAddress();
//   delay(2000);
// }
