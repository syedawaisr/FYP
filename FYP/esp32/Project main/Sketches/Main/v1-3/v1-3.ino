// v1.3
// added shashke

#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#define INCLUDE_GAMEPAD_MODULE
#define INCLUDE_NOTIFICATION_MODULE

#include <Dabble.h>
#include <L298NX2.h>
#include <avr/wdt.h>

#define ENA 5
#define ENB 6
#define IN1 4
#define IN2 7
#define IN3 8
#define IN4 12

#define forwardIRPin 11

#define upperThresh 9.5
#define lowerThresh 0

L298N leftMotors(ENA, IN1, IN2);
L298N rightMotors(ENB, IN3, IN4);
bool upButton = false;
bool downButton = false;
bool leftButton = false;
bool rightButton = false;
bool squareButton = false;
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

void reset()
{
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}    
}

void communications()
{
  Dabble.processInput();
  if (GamePad.isStartPressed()) {reset();}
  yaxis = Sensor.getAccelerometerYaxis();
  xaxis = Sensor.getAccelerometerXaxis();
  zaxis = Sensor.getAccelerometerZaxis();
  upButton = GamePad.isUpPressed();
  downButton = GamePad.isDownPressed();
  rightButton = GamePad.isRightPressed();
  leftButton = GamePad.isLeftPressed();
  squareButton = GamePad.isSquarePressed();
}

void speedControl()
{
  double diff = zaxis - xaxis;
  speed = 30 * diff;
  if (speed < 70) {speed = 0;}
  if (speed > 255) {speed = 255;}
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
  
  // check if leftSpeed and rightSpeed are not out of range
  if (leftSpeed < 70) {leftSpeed = 70;}
  else if (leftSpeed > 255) {leftSpeed = 255;}
  if (rightSpeed < 70){rightSpeed = 70;}
  else if (rightSpeed > 255) {rightSpeed = 255;}

  // if car has to go backwards, reverse the left and right directions
  if (downButton && !upButton)
  {
    if (leftDirection == L298N::BACKWARD) {leftDirection = L298N::FORWARD;} else {leftDirection = L298N::BACKWARD;}
    if (rightDirection == L298N::BACKWARD) {rightDirection = L298N::FORWARD;} else {rightDirection = L298N::BACKWARD;}
  }
}

void runMotors()
{
  leftMotors.setSpeed(leftSpeed);
  rightMotors.setSpeed(rightSpeed);
  leftMotors.run(leftDirection);
  rightMotors.run(rightDirection);
}

void stopMotors()
{
  leftMotors.stop();
  rightMotors.stop();  
}

void turn(int delayTime)
{
  runMotors();
  delay(delayTime);
  stopMotors();
}

void manageManoeuvre()
{
  if (squareButton) {
    leftSpeed = 255;
    rightSpeed = 255;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD; 
    if (upButton && !downButton) {turn(800);}
    else {turn(600);}
  }
  else if (leftButton) {
    leftSpeed = 255;
    rightSpeed = 255;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD;
    if (upButton && !downButton) {turn(400);}
    else {turn(300);}
  }
  else if (rightButton) {
    leftSpeed = 255;
    rightSpeed = 255;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::BACKWARD;
    if (upButton && !downButton) {turn(400);}
    else {turn(300);}
  }
}

void motorsControl()
{  
  if (upButton && !downButton)
  {
    if (!stop) {runMotors();}
    else if (stop) {
      stopMotors();
      Notification.notifyPhone(String("Obstruction infront."));
    }
  }
  else if (!upButton && downButton) {runMotors();}
  else {stopMotors();}
}

void setup()
{
  Dabble.begin(9600);
  pinMode(forwardIRPin, INPUT);

  Notification.clear();
  Notification.setTitle("Obstacle");
}

void loop()
{
  squareButton = false;
  digitalRead(forwardIRPin) == HIGH? stop = false: stop = true;
  communications();
  manageManoeuvre();
  directionControl();
  motorsControl();
  speedControl();
}
