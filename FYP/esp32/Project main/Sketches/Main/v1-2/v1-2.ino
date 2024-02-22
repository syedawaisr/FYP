// v1.2
// added obstacle detection using Ultrasonic Sensor and Bumper Triggers

#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>
#include <L298NX2.h>

TaskHandle_t Comms;

#define ENA 2
#define ENB 4
#define IN1 16
#define IN2 17
#define IN3 5
#define IN4 18

#define upperThresh 8.5
#define lowerThresh 0

L298N leftMotors(ENA, IN1, IN2);
L298N rightMotors(ENB, IN3, IN4);
int forward = 0;
int backward = 0;
int speed = 0;
double yaxis = 0;
double xaxis = 0;
double zaxis = 0;

int rightSpeed = 0;
int leftSpeed = 0;
L298N::Direction leftDirection;
L298N::Direction rightDirection;

void speedControl()
{
  double diff = zaxis - xaxis;
  speed = 30 * diff;
  if (speed < 70)
  {
    speed = 0;
  }
  if (speed > 255)
  {
    speed = 255;    
  }
}

void communications(void *){
  for(;;)
  {
    Dabble.processInput();
    yaxis = Sensor.getAccelerometerYaxis();
    xaxis = Sensor.getAccelerometerXaxis();
    zaxis = Sensor.getAccelerometerZaxis();
    forward = int(GamePad.isUpPressed());
    backward = int(GamePad.isDownPressed());
    speedControl();
  }
}

void directionControl(){

  double offset = 0;
  leftSpeed = speed;
  rightSpeed = speed;
  double y = yaxis;
  
  if (y > lowerThresh && y < upperThresh)
  {
    offset = 30 * (y - lowerThresh);
    leftSpeed += offset;
    rightSpeed -= offset;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::FORWARD; 
  }
  else if (y > -upperThresh && y < -lowerThresh)
  {
    offset = 30 * (y + lowerThresh);
    leftSpeed += offset;
    rightSpeed -= offset;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::FORWARD;    
  }
  else if (y > upperThresh)
  {
    leftSpeed = 255;
    rightSpeed = 128;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::BACKWARD;
  }
  else if (y < -upperThresh)
  {
    leftSpeed = 128;
    rightSpeed = 255;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD;
  }
  
  if (leftSpeed < 70){
    leftSpeed = 70;    
  }
  else if (leftSpeed > 255){
    leftSpeed = 255;
  }
  
  if (rightSpeed < 70){
    rightSpeed = 70;    
  }
  else if (rightSpeed > 255){
    rightSpeed = 255;
  }
  if (backward == 1 && forward == 0)
  {
    if (leftDirection == L298N::BACKWARD) {leftDirection = L298N::FORWARD;} else {leftDirection = L298N::BACKWARD;}
    if (rightDirection == L298N::BACKWARD) {rightDirection = L298N::FORWARD;} else {rightDirection = L298N::BACKWARD;}
  }
}

void motorControl(){
  if (forward == 1 && backward == 0)
  {
    rightMotors.setSpeed(rightSpeed);
    leftMotors.setSpeed(leftSpeed);
    rightMotors.run(leftDirection);
    leftMotors.run(rightDirection);
  }
  else if (forward == 0 && backward == 1)
  {
    rightMotors.setSpeed(rightSpeed);
    leftMotors.setSpeed(leftSpeed);
    rightMotors.run(leftDirection);
    leftMotors.run(rightDirection);
  }
  else
  {
    leftMotors.stop();
    rightMotors.stop();
  }
}

void setup() {
  Dabble.begin("NeoOctane");

  xTaskCreatePinnedToCore(communications, "Comms", 10000, NULL, 0, &Comms, 0);
}

void loop() {
  directionControl();
  motorControl();
}
