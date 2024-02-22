// v1.6
// added OLED code.

#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#define INCLUDE_GAMEPAD_MODULE
#define INCLUDE_NOTIFICATION_MODULE

#include <Dabble.h>
#include <L298NX2.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// pins
#define ENA 5
#define ENB 6
#define IN1 4
#define IN2 7
#define IN3 8
#define IN4 12
#define forwardIRPin 11
#define backwardIRPin 10
#define buzzer A0
#define ResetPin A1

// thresholds
#define maxSpeed 255
#define minSpeed 80
#define upperThresh 9.5
#define lowerThresh 0

// motor controllers
L298N leftMotors(ENA, IN1, IN2);
L298N rightMotors(ENB, IN3, IN4);

// flags
bool upButton = false;
bool downButton = false;
bool leftButton = false;
bool rightButton = false;
bool squareButton = false;
bool circleButton = false;
bool selectButton = false;
bool IROff = true;
bool forwardStop = false;
bool backwardStop = false;

int speed = 0;
double yaxis = 0; 
double xaxis = 0;
double zaxis = 0;
int rightSpeed = 0;
int leftSpeed = 0;
L298N::Direction leftDirection;
L298N::Direction rightDirection;

// reset the arduino
void reset() {pinMode(ResetPin, OUTPUT);}

// display a single character on the OLED
void singleCharDisplay(char c)
{
  if (leftButton || rightButton) {return;}
  display.clearDisplay();
  display.setCursor(56, 5);
  display.setTextColor(WHITE);
  display.setTextSize(4);
  display.print(c);
}

// display an arrow on the OLED
void arrowDisplay(bool right)
{
  display.clearDisplay();
  display.setCursor(32, 5);
  display.setTextColor(WHITE);
  display.setTextSize(4);
  right? display.print("<--"): display.print("-->");
}

// get button states and accelerometer values from Dabble app
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

  if (GamePad.isStartPressed()) {reset();}

  if (GamePad.isSelectPressed()) {
    IROff = !IROff;
    delay(200);
    Notification.clear();
    Notification.notifyPhone(String("Toggled"));
  }
}

// calculate speed from phone's accelerometer values
void speedControl()
{
  double diff = zaxis - xaxis;
  speed = 30 * diff;
  if (speed < minSpeed) {speed = 0;}
  if (speed > maxSpeed) {speed = maxSpeed;}
}

// calculate individual speeds of left and right motors depending on the phone's accelerometer values
void directionControl()
{
  double offset = 0;
  leftSpeed = speed;
  rightSpeed = speed;
  double y = yaxis;

  // rotate the car to left if left button is pressed
  if (leftButton) {
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD;
  }
  // rotate  the car to the right if right button is pressed
  else if (rightButton) {
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::BACKWARD;
  }  
  else if (y > lowerThresh && y < upperThresh)
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

// run the motors
void runMotors()
{
  leftMotors.setSpeed(leftSpeed);
  rightMotors.setSpeed(rightSpeed);
  leftMotors.run(leftDirection);
  rightMotors.run(rightDirection);
}

// stop the motors
void stopMotors()
{
  leftMotors.stop();
  rightMotors.stop();  
}

// turn the car to a certain position using a delay value
void turn(int delayTime)
{
  runMotors();
  delay(delayTime);
  stopMotors();
}

// manages all the manoeuvres that our car can perform
void manageManoeuvre()
{
  if (squareButton) {
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD; 
    if (upButton && !downButton) {turn(1000);}
    else {turn(700);}
  }
}

// manages movements of car if without the contraints of IR sensor
void motorsControlWithoutIR()
{
  if (rightButton) {
    arrowDisplay(true);
    runMotors();
  }
  else if (leftButton) {
    arrowDisplay(false);
    runMotors();
  }
  else if (upButton && !downButton) {
    runMotors();
    singleCharDisplay('F');
  }
  else if (!upButton && downButton) {
    runMotors();
    singleCharDisplay('R');
  }
  else {
    stopMotors();
    singleCharDisplay('N');
  }  
}

// manages movement of car with obstacle detection turned on
void motorsControlWithIR()
{
  if (rightButton) {
    arrowDisplay(true);
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::FORWARD;
    rightDirection = L298N::BACKWARD;
    runMotors();
  }
  else if (leftButton) {
    arrowDisplay(false);
    leftSpeed = maxSpeed;
    rightSpeed = maxSpeed;
    leftDirection = L298N::BACKWARD;
    rightDirection = L298N::FORWARD;
    runMotors();
  }
  else if (upButton && !downButton)
  {
    if (!forwardStop) {
      runMotors();
      singleCharDisplay('F');
    }
    else if (forwardStop) 
    {
      stopMotors();
      Notification.notifyPhone(String("Obstruction infront."));
    }
  }
  else if (!upButton && downButton) {
    if (!backwardStop) {
      runMotors();
      singleCharDisplay('R');
    }
    else if (backwardStop) 
    {
      stopMotors();
      Notification.notifyPhone(String("Obstruction at the back."));
    }
  }
  else {
    stopMotors();
    singleCharDisplay('N');
  }
}

void setup()
{
  Serial.begin(9600);

  // initialize the display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(100);
  singleCharDisplay('N');
  display.display();

  Dabble.begin(9600); // initialize dabble
  pinMode(forwardIRPin, INPUT);
  pinMode(backwardIRPin, INPUT);
  pinMode(buzzer, OUTPUT);

  Notification.clear();
  Notification.setTitle("Obstacle Detection");
}

void loop()
{
  squareButton = false;
  digitalRead(forwardIRPin) == HIGH? forwardStop = false: forwardStop = true;
  digitalRead(backwardIRPin) == HIGH? backwardStop = false: backwardStop = true;
  communications();
  circleButton? digitalWrite(buzzer, HIGH): digitalWrite(buzzer, LOW); // turn buzzer on if circle button is pressed on phone
  directionControl();
  speedControl();
  manageManoeuvre();
  IROff? motorsControlWithoutIR(): motorsControlWithIR();
  display.display();
}
