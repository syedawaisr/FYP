// v1.2
// added obstacle detection using Ultrasonic Sensor

#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#define INCLUDE_GAMEPAD_MODULE
#define INCLUDE_NOTIFICATION_MODULE

#include <Dabble.h>
#include <L298NX2.h>

#define ENA 5
#define ENB 6
#define IN1 4
#define IN2 7
#define IN3 8
#define IN4 12

#define TRIG 9
#define ECHO 10
#define distThresh 20

#define IRPin 11

#define upperThresh 9.5
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
bool stop = false;

int dists[10] = {0};
int distCount = 0;

int distanceUS()
{
  int distance = 0, duration = 0;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH, 10000);
  distance = (duration/2)/29.1;
  return distance;
}

bool shouldStop()
{
  dists[distCount] = distanceUS();
  ++distCount %= 10;
  int a = 0;
  for (int i = 0; i < 10; i++)
  {
    if (dists[i] < 20 && dists[i] != 0) {a += 1;}
  }
  return a > 6;
}

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

void communications()
{
  Dabble.processInput();
  yaxis = Sensor.getAccelerometerYaxis();
  xaxis = Sensor.getAccelerometerXaxis();
  zaxis = Sensor.getAccelerometerZaxis();
  forward = int(GamePad.isUpPressed());
  backward = int(GamePad.isDownPressed());
}

void directionControl()
{
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
    leftSpeed = 0;    
  }
  else if (leftSpeed > 255){
    leftSpeed = 255;
  }
  
  if (rightSpeed < 70){
    rightSpeed = 0;
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

void motorsControl()
{
  if (forward == 1 && backward == 0 && !stop)
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
  else if (forward == 1 && backward == 0 && stop)
  {
    leftMotors.stop();
    rightMotors.stop();
    Notification.notifyPhone(String("Obstruction infront."));
  }
  else
  {
    leftMotors.stop();
    rightMotors.stop();
  }
}

void readIR()
{
  if (digitalRead(IRPin) == HIGH) {
    // Serial.println("No obstacle detected.");
    stop = false;
  }
  else {
    // Serial.println("Obstacle detected.");
    stop = true;
  }
}

void setup()
{
  Dabble.begin(9600);
  // Dabble.waitForAppConnection();
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(IRPin, INPUT);

  Notification.clear();
  Notification.setTitle("Obstacle");
}

void loop()
{
  digitalRead(IRPin) == HIGH? stop = false: stop = true;
  // stop = shouldStop();
  // readIR();
  communications();
  directionControl();
  motorsControl();
  speedControl();
  
}
