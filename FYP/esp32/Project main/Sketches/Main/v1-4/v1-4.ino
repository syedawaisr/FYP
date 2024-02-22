// v1.3
// added shashke

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
#define maxSpeed 255
#define minSpeed 80

#define forwardIRPin 11
#define backwardIRPin 10
#define buzzer A0

#define upperThresh 9.5
#define lowerThresh 0

L298N leftMotors(ENA, IN1, IN2);
L298N rightMotors(ENB, IN3, IN4);

bool upButton = false;
bool downButton = false;
bool leftButton = false;
bool rightButton = false;
bool squareButton = false;
bool circleButton = false;
bool selectButton = false;

bool IROff = true;

int speed = 0;
double yaxis = 0; 
double xaxis = 0;
double zaxis = 0;

int rightSpeed = 0;
int leftSpeed = 0;
L298N::Direction leftDirection;
L298N::Direction rightDirection;
bool forwardStop = false;
bool backwardStop = false;

int dists[10] = {0};
int distCount = 0;

void communications()
{
  Dabble.processInput();
  yaxis = Sensor.getAccelerometerYaxis();
  xaxis = Sensor.getAccelerometerXaxis();
  zaxis = Sensor.getAccelerometerZaxis();
  upButton = GamePad.isUpPressed();
  downButton = GamePad.isDownPressed();
  rightButton = GamePad.isRightPressed();
  leftButton = GamePad.isLeftPressed();
  squareButton = GamePad.isSquarePressed();
  circleButton = GamePad.isCirclePressed();
  if (GamePad.isSelectPressed();) {IROff = !IROff;}
}

void speedControl()
{
  double diff = zaxis - xaxis;
  speed = 30 * diff;
  if (speed < minSpeed) {speed = 0;}
  if (speed > maxSpeed) {speed = maxSpeed;}
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
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed / 2;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::BACKWARD;
  }
  else if (y < -upperThresh)
  {
    leftSpeed = maxSpeed / 2;
    rightSpeed = maxSpeed;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD;
  }
  
  // check if leftSpeed and rightSpeed are not out of range
  if (leftSpeed < minSpeed) {leftSpeed = minSpeed;}
  else if (leftSpeed > maxSpeed) {leftSpeed = maxSpeed;}
  if (rightSpeed < minSpeed){rightSpeed = minSpeed;}
  else if (rightSpeed > maxSpeed) {rightSpeed = maxSpeed;}

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
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD; 
    if (upButton && !downButton) {turn(800);}
    else {turn(600);}
  }
  else if (leftButton) {
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD;
    if (upButton && !downButton) {turn(50);}
    else {turn(50);}
  }
  else if (rightButton) {
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::BACKWARD;
    if (upButton && !downButton) {turn(50);}
    else {turn(50);}
  }
}

void motorsControl()
{  
  if (upButton && !downButton)
  {
    if (!forwardStop) {runMotors();}
    else if (forwardStop) 
    {
      stopMotors();
      Notification.notifyPhone(String("Obstruction infront."));
    }
  }
  else if (!upButton && downButton) {
    if (!backwardStop) {runMotors();}
    else if (backwardStop) 
    {
      stopMotors();
      Notification.notifyPhone(String("Obstruction at the back."));
    }
  }
  else {stopMotors();}
}

void setup()
{
  Serial.begin(9600);
  Dabble.begin(9600);
  pinMode(forwardIRPin, INPUT);
  pinMode(backwardIRPin, INPUT);
  pinMode(buzzer, OUTPUT);

  Notification.clear();
  Notification.setTitle("Obstacle Detection");
}

void printStatus()
{
  Serial.println(String("Right Speed: ") + rightSpeed);
  Serial.println(String("Left Speed: ") + leftSpeed);
}

void loop()
{
  squareButton = false;
  digitalRead(forwardIRPin) == HIGH? forwardStop = false: forwardStop = true;
  digitalRead(backwardIRPin) == HIGH? backwardStop = false: backwardStop = true;
  communications();
  circleButton? digitalWrite(buzzer, HIGH): digitalWrite(buzzer, LOW);
  manageManoeuvre();
  directionControl();
  speedControl();
  printStatus();  
  motorsControl();  
}
