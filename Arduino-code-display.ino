////////////////////////////////////////////////////////////////////////////////
//  Program to test the Arduino UNO + NodeMCU V3 wifi module + I2C LCD dislplay
//
// This code is uploaded on the Arduino UNO
// It receives data from the NodeMCU module
//
//  Aim : Display daily coronavirus epidemiologic indicators
//
//  Daily data obtained through CoronavirusAPI-France :
//  https://github.com/florianzemma/CoronavirusAPI-France
//
// - One RGB LED will be green or red according to hospital stress
// - One yellow LED will flash at each contamination
// - One green LED will flash at each hospital discharge
// - One red LED will flash at each hospital death
//
////////////////////////////////////////////////////////////////////////////////


// --------- Define Arduino UNO I/O pins ---------------------------------------
const int greenHospled = 10;  // RGB LED, color will be be PMW-controlled
const int redHospled = 11;
int greenHospval = 0;         // Set the initial output value of each color
int redHospval = 0;

const int yellowCountLed = 2; // Three LED that will be ON/OFF controlled
const int greenCountLed = 3;
const int redCountLed = 4;

// --------- Define epidemic values --------------------------------------------
// (All initialized with unreal values to help debug)

float DailyCasesNb = 1;  // number of daily cases
float DailyDeathNb = 1;  // number of daily death
float HospitNb = 1;      // number of daily hospitalizations
float RadNb = 2;         // number of daily discharges

// TendanceHosp = HospitNb / RadNb.
// If value > 1 = hospital tension increases, if <1 hospital mass decreases.
// Will be calculated in loop() -> data-catcher
float TendanceHosp = 0.1;

// Set the number of miliseconds in 24h to ease readability
long MilisecPerDay = 86400000;

// --------- Library used for blinking LED without using delay function --------
#include <ezOutput.h>

// create ezOutput object that attach to each lED pin;
ezOutput Yled(yellowCountLed);
ezOutput Gled(greenCountLed);
ezOutput Rled(redCountLed);

// --------- Function to map a ranged float to another range  ------------------
float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }


// ---------  LCD Managing -----------------------------------------------------
#include <Wire.h> // To use Seeduino I2C LCD display
#include "rgb_lcd.h"
rgb_lcd lcd;


// Based on robtillaart script found here :
// https://forum.arduino.cc/t/cycling-lcd-display-without-using-delay-in-code-solved/58594/2
// It will cycle between 4 different screens during the loop()

long previousLCDMillis = 0;    // for LCD screen update
long lcdInterval = 2000;
int screen = 0;
int screenMax = 3;
bool screenChanged = true;   // initially we have a new screen,  by definition

// defines of the screens to show
#define CASES   0
#define HOSPITS   1
#define DEATHS      2
#define TENSIONS      3

// display functions
void showWelcome(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CovidMeter");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("initialising");
  delay(1200);
  lcd.print(".");
  delay(1200);
  lcd.print(".");
  delay(1200);
  lcd.print(".");
  delay(1200);
  lcd.print(".");
  delay(1200);
  }

void showCases(long C){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New cases :");
  lcd.setCursor(0, 1);
  lcd.print(C);}

void showHospits(long H){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New hospits :");
  lcd.setCursor(0, 1);
  lcd.print(H);}

void showDeaths(long D){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New deaths :");
  lcd.setCursor(0, 1);
  lcd.print(D);}

void showTensions(float T){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nb In / Nb Out :");
  lcd.setCursor(0, 1);
  lcd.print(T);}

void showError(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error");}


// ---------   Lib to parse the JSon received though Covid API -----------------
#include <ArduinoJson.h>
StaticJsonDocument<400> doc;


// ---------   SETUP START -----------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(greenCountLed, OUTPUT);
  pinMode(yellowCountLed, OUTPUT);
  pinMode(redCountLed, OUTPUT);

  pinMode(redHospled, OUTPUT);
  pinMode(greenHospled, OUTPUT);

  // Show welcome message
  lcd.begin(16, 2);
  showWelcome();
}
// ---------   SETUP END -------------------------------------------------------



// ---------   LOOP START ------------------------------------------------------
void loop() {

// ---------   LCD display -----------------------------------------------------
unsigned long currentLCDMillis = millis();
  // Should we switch screen ?
  if(currentLCDMillis - previousLCDMillis > lcdInterval) // save the last time you changed the display
  {
    previousLCDMillis = currentLCDMillis;
    screen++;
    if (screen > screenMax) screen = 0;  // all screens done? => start over
    screenChanged = true;
  }
// Display current screen
  if (screenChanged)   // only update the screen if the screen is changed.
  {
    screenChanged = false; // reset for next iteration
    switch(screen)
    {
    case CASES:
      showCases(long(DailyCasesNb));
      break;
    case HOSPITS:
      showHospits(long(HospitNb));
      break;
    case DEATHS:
      showDeaths(long(DailyDeathNb));
      break;
    case TENSIONS:
      showTensions(TendanceHosp);
      break;
    default:
      showError()
      break;
    }
  }
// ---------   End LCD display  ------------------------------------------------

// ---------   Data catcher ----------------------------------------------------

// get Raw Json from NodeMCU and parse it to update epidemic indicators
  if (Serial.available() > 0){
    delay(10);
    String s = Serial.readStringUntil('\n');
    while(Serial.available() > 0) {Serial.read();}
    DeserializationError error = deserializeJson(doc, s);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;}
    Serial.println(error.c_str());

    // Update epidemiologic indicators
    DailyCasesNb = doc.as<JsonArray>()[0].as<JsonObject>()["conf_j1"];
    DailyDeathNb = doc.as<JsonArray>()[0].as<JsonObject>()["incid_dchosp"];
    HospitNb = doc.as<JsonArray>()[0].as<JsonObject>()["incid_hosp"];
    RadNb = doc.as<JsonArray>()[0].as<JsonObject>()["incid_rad"];

    // Calc and update TandanceHosp
    TendanceHosp = HospitNb / RadNb ;

    // Uncomment next lines for debug
    //    Serial.println("Number of cases is :");
    //    Serial.println(DailyCasesNb);
    //    Serial.println("Number of deaths is :");
    //    Serial.println(DailyDeathNb);
    //    Serial.println("Number of hospitalizations is :");
    //    Serial.println(HospitNb);
    //    Serial.println("Number of recovery is :");
    //    Serial.println(RadNb);
    //    Serial.println("Tension hospit :");
    //    Serial.println(TendanceHosp);

    // Set yellow lED blinking rate (1 for each contamination)
    // calc mean number of ms between each daily contamination
    long MilisecPerConta = MilisecPerDay / DailyCasesNb +1; //+1 to avoid 0 div
    Yled.blink(MilisecPerConta, 100); // flash each conta for 100 ms

    // Same for each death and the red lED
    long MilisecPerDeath = MilisecPerDay / DailyDeathNb+1;
    long RFlashFreq = MilisecPerDeath;
    Rled.blink(MilisecPerDeath, 100);

    // Same for each hospital discharge and the green lED
    long MilisecPerRad = MilisecPerDay / RadNb+1;
    Gled.blink(MilisecPerRad, 100);

    // Set the RGB LED as green if TendanceHosp <1 or red if >=1
    if (TendanceHosp >= 1) {
      // map the 1 to 5 TandanceHosp range to the 1 to 255 PWM range of the LED
      redHospval = floatMap(TendanceHosp, 1, 5, 0, 255);
      analogWrite(redHospled, redHospval);
      digitalWrite(greenHospled, LOW); }
    else {
       greenHospval = 255-(floatMap(TendanceHosp, 0, 1, 0, 255));
       analogWrite(greenHospled, greenHospval);
       digitalWrite(redHospled, LOW); }
   }
// ---------   End of data catcher ---------------------------------------------

// ---------   Blink leds according to parameters ------------------------------
  Yled.loop();
  Rled.loop();
  Gled.loop();

}
// ---------   LOOP END --------------------------------------------------------
