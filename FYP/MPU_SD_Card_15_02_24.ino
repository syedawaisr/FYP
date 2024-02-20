#include "Wire.h"
#include <SD.h>

File dataFile;
const int chipSelect = 10;  // SD card module's chip select pin

int MPU_addr = 0x68;
int cal_gyro = 1;

float A_cal[6] = {0.0, 0.0, 0.0, 1.000, 1.000, 1.000};
float G_off[3] = {0.0, 0.0, 0.0};
#define gscale ((250.0 / 32768.0) * (PI / 180.0))

float q[4] = {1.0, 0.0, 0.0, 0.0};

float Kp = 30.0;
float Ki = 0.0;

unsigned long now_ms, last_ms = 0;
unsigned long print_ms = 200;  // Adjusted print interval
float yaw, pitch, roll;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed!");
    return;
  }

  Serial.println("SD card initialized successfully.");

  Wire.begin();
  Serial.println("Starting");

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}


void loop() {
  static unsigned int i = 0;
  static float deltat = 0;
  static unsigned long now = 0, last = 0;
  static long gsum[3] = {0};

  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  int16_t Tmp;

  float Axyz[3];
  float Gxyz[3];

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14);

  int t = Wire.read() << 8;
  ax = t | Wire.read();
  t = Wire.read() << 8;
  ay = t | Wire.read();
  t = Wire.read() << 8;
  az = t | Wire.read();
  t = Wire.read() << 8;
  Tmp = t | Wire.read();
  t = Wire.read() << 8;
  gx = t | Wire.read();
  t = Wire.read() << 8;
  gy = t | Wire.read();
  t = Wire.read() << 8;
  gz = t | Wire.read();

  i++;
  if (cal_gyro) {
    gsum[0] += gx;
    gsum[1] += gy;
    gsum[2] += gz;
    if (i == 500) {
      cal_gyro = 0;

      for (char k = 0; k < 3; k++) G_off[k] = static_cast<float>(gsum[k]) / 500.0;

      Serial.print("G_Off: ");
      Serial.print(G_off[0]);
      Serial.print(", ");
      Serial.print(G_off[1]);
      Serial.print(", ");
      Serial.print(G_off[2]);
      Serial.println();
    }
  } else {
    Axyz[0] = static_cast<float>(ax);
    Axyz[1] = static_cast<float>(ay);
    Axyz[2] = static_cast<float>(az);

    for (i = 0; i < 3; i++) Axyz[i] = (Axyz[i] - A_cal[i]) * A_cal[i + 3];

    Gxyz[0] = (static_cast<float>(gx) - G_off[0]) * gscale;
    Gxyz[1] = (static_cast<float>(gy) - G_off[1]) * gscale;
    Gxyz[2] = (static_cast<float>(gz) - G_off[2]) * gscale;

    now = micros();
    deltat = (now - last) * 1.0e-6;
    last = now;

    Mahony_update(Axyz[0], Axyz[1], Axyz[2], Gxyz[0], Gxyz[1], Gxyz[2], deltat);

    roll = atan2((q[0] * q[1] + q[2] * q[3]), 0.5 - (q[1] * q[1] + q[2] * q[2]));
    pitch = asin(2.0 * (q[0] * q[2] - q[1] * q[3]));
    yaw = -atan2((q[1] * q[2] + q[0] * q[3]), 0.5 - (q[2] * q[2] + q[3] * q[3]));

    yaw *= 180.0 / PI;
    if (yaw < 0) yaw += 360.0;
    pitch *= 180.0 / PI;
    roll *= 180.0 / PI;

    now_ms = millis();
    if (now_ms - last_ms >= print_ms) {
      last_ms = now_ms;
      Serial.print("Pitch: ");
      Serial.print(pitch, 2);  // Adjusted decimal places for better readability
      Serial.print(", Yaw: ");
      Serial.print(yaw, 2);
      Serial.print(", Roll: ");
      Serial.println(roll, 2);

      // Save to SD card
      saveDataToSD(pitch, yaw, roll);
    }
  }
  delay(10);  // Added delay to avoid overwhelming Serial Monitor
}

void saveDataToSD(float pitch, float yaw, float roll) {
  // Define the file name
  const char *fileName = "Euler.csv";

  // Define desired range for each angle
  const float desired_pitch_range_min = -90;
  const float desired_pitch_range_max = 90;
  const float desired_yaw_range_min = -180;
  const float desired_yaw_range_max = 180;
  const float desired_roll_range_min = -180;
  const float desired_roll_range_max = 180;

  // Define a small margin of error for angle comparisons
  const float angle_margin = 0.01;

  // Normalize yaw angle to be within the desired range
  while (yaw < desired_yaw_range_min - angle_margin) {
    yaw += 360.0;
  }
  while (yaw > desired_yaw_range_max + angle_margin) {
    yaw -= 360.0;
  }

  // Normalize roll angle to be within the desired range
  while (roll < desired_roll_range_min - angle_margin) {
    roll += 360.0;
  }
  while (roll > desired_roll_range_max + angle_margin) {
    roll -= 360.0;
  }

  // Check if any angle is outside the desired range with a margin of error
  if (pitch < (desired_pitch_range_min - angle_margin) || pitch > (desired_pitch_range_max + angle_margin) ||
      yaw < (desired_yaw_range_min - angle_margin) || yaw > (desired_yaw_range_max + angle_margin) ||
      roll < (desired_roll_range_min - angle_margin) || roll > (desired_roll_range_max + angle_margin)) {
    Serial.println("Error: Euler angles contain undesired values.");
    return;
  }

  // Open the file in write or append mode
  File dataFile = SD.open(fileName, FILE_WRITE | O_APPEND);

  if (dataFile) {
    // Check if the file is empty (i.e., newly created)
    if (dataFile.size() == 0) {
      // If it's empty, write headers
      dataFile.println("Pitch, Yaw, Roll");
    }

    // Write the data to the file
    dataFile.print(pitch);
    dataFile.print(", ");
    dataFile.print(yaw);
    dataFile.print(", ");
    dataFile.println(roll);

    // Close the file
    dataFile.close();

    Serial.println("Data saved to Euler.csv");
  } else {
    Serial.println("Error opening or writing to Euler.csv");
  }
}





void Mahony_update(float ax, float ay, float az, float gx, float gy, float gz, float deltat) {
  float recipNorm;
  float qa, qb, qc;

  deltat = 0.5 * deltat;
  gx *= deltat;
  gy *= deltat;
  gz *= deltat;
  qa = q[0];
  qb = q[1];
  qc = q[2];

  q[0] += (-qb * gx - qc * gy - q[3] * gz);
  q[1] += (qa * gx + qc * gz - q[3] * gy);
  q[2] += (qa * gy - qb * gz + q[3] * gx);
  q[3] += (qa * gz + qb * gy - qc * gx);

  recipNorm = 1.0 / sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  q[0] = q[0] * recipNorm;
  q[1] = q[1] * recipNorm;
  q[2] = q[2] * recipNorm;
  q[3] = q[3] * recipNorm;
}