#include "arduino_secrets.h"
//FOR IR
#include <IRremote.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

//For LDR
#include <Arduino.h>

const int ADC_PIN = 34;

const int RECV_PIN = 13;

IRrecv irrecv(RECV_PIN);

decode_results results;

// FOR LCD
#include <LiquidCrystal_I2C.h>
// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// FOR SERVO
#include <ESP32Servo.h>

static const int servoPin1 = 12;
Servo servo1;
static const int servoPin2 = 27;
Servo servo2;
/* 
  https://create.arduino.cc/cloud/things/4db1d4cf-68f2-436a-bf12-3526d9d106d1 

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  int floor1Spots;
  int floor2Spots;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include "thingProperties.h"
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void openBoom1() {
  for (int posDegrees = 90; posDegrees >= 0; posDegrees--) { // Open counterclockwise (start at 90, end at 0)
    servo1.write(posDegrees);
    //Serial.println(posDegrees);
    delay(20);
  }  
}

void openBoom2() {
  for (int posDegrees = 90; posDegrees >= 0; posDegrees--) { // Open counterclockwise (start at 90, end at 0)
    servo2.write(posDegrees);
    //Serial.println(posDegrees);
    delay(20);
  }  
}

void closeBoom1() {
  for (int posDegrees = 0; posDegrees <= 90; posDegrees++) { // Close clockwise (start at 0, end at 90)
    servo1.write(posDegrees);
    //Serial.println(posDegrees);
    delay(20);
  }
}

void closeBoom2() {
  for (int posDegrees = 0; posDegrees <= 90; posDegrees++) { // Close clockwise (start at 0, end at 90)
    servo2.write(posDegrees);
    //Serial.println(posDegrees);
    delay(20);
  }
}



void printTokenNo(int token)
{
  lcd.setCursor(7,0);
  lcd.print("Token: ");
  lcd.print(token);
}

void printPrice(int price)
{
  lcd.setCursor(7,1);
  lcd.print("Price: ");
  lcd.print(price);
}

void parkingFull()
{
  lcd.setCursor(7,0);
  lcd.print("PARKING");
  lcd.setCursor(7,1);
  lcd.print("FULL");
}

void updateLCD()
{
  lcd.setCursor(0, 0);
  lcd.print("L1: ");
  lcd.print(floor1Spots);
  delay(1000);
  // set cursor to first column, second row
  lcd.setCursor(0,1);
  lcd.print("L2: ");
  lcd.print(floor2Spots);
  delay(1000);
}

void readUS()
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // debug codes
  // Serial.println("distance");
  // Serial.println(distanceCm);
  
  delay(200);
}

String hexValue;

int readStoreIR()
{
  if (irrecv.decode(&results))
  {
      hexValue = String(results.value, HEX);  // Convert to hex and save as string
      Serial.print("Received Hex Value: ");
      Serial.println(hexValue);              // Print the hex value
      irrecv.resume();   
  }
  int num;

  if(hexValue == "f710ef")
    num = 1;
  else if (hexValue == "f7906f")
    num = 2;
  else if (hexValue == "f750af")
    num = 3;
  else if (hexValue == "f730cf")
    num = 4;
  else if (hexValue == "f7b04f")
    num = 5;
  else if (hexValue == "f7708f")
    num = 6;
  else if (hexValue == "f708f7")
    num = 7;
  else if (hexValue == "f78877")
    num = 8;
  else if (hexValue == "f748b7")
    num = 9;
  else if (hexValue == "f728d7")
    num = 10;
  else if (hexValue == "f7a857")
    num = 11;
  else if (hexValue == "f76897")
    num = 12;
  else
    num = 0;

  return num;
}

int adcValue;
void readADC()
{
  adcValue = analogRead(ADC_PIN);  // Read ADC value
  //Serial.println(adcValue);           // Print ADC value to the Serial Monitor
  delay(500);
}

int parkArray[12] = {0};
long entryTime[12] = {0};
long exitTime[12] = {0};
int parkSlots = 12;

int assignToken(int parkToken[], long eTime[], int &slots, int &slot1, int &slot2){
  int tokenNum;
  
  for(int i = 0; i<12; i++){
    if(parkToken[i] == 0){
      parkToken[i] = 1;
      tokenNum = (i+1);
      eTime[i] = millis();
      // debug prints
      // Serial.println("entry time");
      // Serial.println(eTime[i]);
      slots--;
      break;
    }
  }

  int occ1 = 0;
  int occ2 = 0;
  for(int i = 0; i<6; i++){
    if(parkToken[i] == 1)
      occ1++;
  }
  for(int j = 6; j<12; j++){
    if(parkToken[j] == 1)
      occ2++;
  }

  slot1 = (6-occ1);
  slot2 = (6-occ2);
  
  return tokenNum;
}

long int revokeToken(int parkToken[], int tokenNumber, long exTime[], long eTime[], int &slots, int &slot1, int &slot2){
  Serial.println("revokeToken was called");
  long time;
  Serial.println(tokenNumber);
  parkToken[(tokenNumber-1)] = 0;
  exTime[(tokenNumber-1)] = millis();
  time = (exTime[(tokenNumber-1)] - eTime[(tokenNumber-1)])/1000;
  // debug prints
  // Serial.println("Exit time");
  // Serial.println(exTime[(tokenNumber-1)]);
  // Serial.println(time);
  exTime[(tokenNumber-1)] = 0;
  eTime[(tokenNumber-1)] = 0;
  slots++;

  int occ1 = 0;
  int occ2 = 0;
  for(int i = 0; i<6; i++){
    if(parkToken[i] == 1)
      occ1++;
  }
  for(int j = 6; j<12; j++){
    if(parkToken[j] == 1)
      occ2++;
  }

  slot1 = (6-occ1);
  slot2 = (6-occ2);
  
  return time;
}
  

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  // For US 1
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // for 16*2 LCD
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  // for servo
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);

  //for IR
  irrecv.enableIRIn(); // Start the receiver

  //FOR LDR
  pinMode(ADC_PIN, INPUT);

  floor1Spots = 6;
  floor2Spots = 6;
}

void loop() {
  ArduinoCloud.update();
  /*
    ################################
    SHOW AVAILABLE SLOTS ON LCD
    ################################
  */
  updateLCD();


  /*
    ################################
    Entry
    ################################
  */
  readADC();
  if (adcValue <= 50)
  {
    if (parkSlots == 0)
    {
      parkingFull();
      delay(5000);
    }
    else
    {
      int token = assignToken(parkArray, entryTime, parkSlots, floor1Spots, floor2Spots);
      printTokenNo(token);
      openBoom1();
      delay(6000);
      closeBoom1();
      lcd.clear();
    }
  }
  updateLCD();

  /*
    ################################
    Exit
    ################################
  */
  //Serial.println("distanceCm");
  //Serial.println(distanceCm);
  distanceCm = 0;
  if (distanceCm <= 5)
  {
    int token = readStoreIR();
    
    if (token != 0 && parkArray[token-1] == 1)
    {
      int time = revokeToken(parkArray, token, exitTime, entryTime, parkSlots, floor1Spots, floor2Spots);
      printPrice(time);
      openBoom2();
      delay(6000);
      closeBoom2();
      lcd.clear();
      token = 0;
    }
  }
  updateLCD();
  
  
}






