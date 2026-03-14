/*
  JoyStick

  Determines what input is coming from a dual-axis potentiometer joystick

*/

int xPin = 4;        
int yPin = 15;
int xValue = 0; 
int yValue = 0; 
bool isXTilted = false;
bool isYTilted = false;
bool isRight = false;
bool isUp = false;
int tiltStart = 0;
int pressDuration = 0;

void setup() {

  Serial.begin(115200);
}

void loop() {
  // read the value from the sensor:
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);
  buttonPoll(xValue, isXTilted, true);
  buttonPoll(yValue, isYTilted, false);
  delay(200);
}

void buttonPoll(int value, bool isTilted, bool x){
  if ((value > 2500 || value < 1200) && isTilted == false){
    tiltStart = millis();
    if (x){ // Checks if input is from X Axis or Y Axis
      isXTilted = true;
      isRight = value > 2000;
    } else {
      isYTilted = true;
      isUp = value > 2000;
    }
  }

  if ((value > 1800 && value < 1900) && isTilted == true){
    if (x){
      printInput(checkInput(isRight, millis() - tiltStart, x)); 
      isXTilted = false;
      isRight = false; 
    } else {
      printInput(checkInput(isUp, millis() - tiltStart, x)); 
      isYTilted = false;
      isUp = false;
    }
    tiltStart = 0;

  }
}

int checkInput(bool isRight, int tiltDuration, bool x){
  int input = 0;
  if (x){
    if (tiltDuration > 800){
      if (isRight){
        input = 1; // long right
      } else {
        input = 2; // long left
      }
    } else {
      if (isRight){
        input = 3; // short right
      } else {
        input = 4; // short left
      }
    }
  } else {
    if (tiltDuration > 800){
      if (isRight){
        input = 5; // long up
      } else {
        input = 6; // long down
      }
    } else {
      if (isRight){
        input = 7; // short up
      } else {
        input = 8; // short down
      }
    }
  }
  return input;
}

void printInput(int input){
  switch (input) {
    case 1:
      Serial.println("Long Right");
      break;
    case 2: 
      Serial.println("Long Left");
      break;
    case 3: 
      Serial.println("Short Right");
      break;
    case 4:
      Serial.println("Short Left");
      break;
    case 5:
      Serial.println("Long Up");
      break;
    case 6:
      Serial.println("Long Down");
      break;
    case 7:
      Serial.println("Short Up");
      break;
    case 8:
      Serial.println("Short Down");
      break;
    default: 
      Serial.println("No input");
  }
}