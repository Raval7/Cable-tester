// Cable tester
// Version: 1.4
// By Rafał Mlicki

// Liberies:
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <NeoPixelBus.h>
#include <Arduino.h>
#include <jm_PCF8574.h>

// Variables:
LiquidCrystal_I2C lcd(0x27,20,4);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> ledDiode(1, 13);

jm_PCF8574 pcfIn1;
jm_PCF8574 pcfIn2;
jm_PCF8574 pcfIn3;
jm_PCF8574 pcfOut1;
jm_PCF8574 pcfOut2;
jm_PCF8574 pcfOut3;

boolean isCable = false;
boolean isError = false;
boolean connectionMap[24][24];
boolean intermittentOn = false;
boolean intermittentMap[24];

int refreshTime=250;
int mode=0;
unsigned long actualTime = 0;
unsigned long savedTime = 0;
unsigned long timeDifference = 0;
int interCount = 0;
int interLastPin = 0;

// Main program setup
void setup()
{
  // Component initialization
  lcd.init();
  lcd.backlight();
  ledDiode.Begin();
  Wire.begin();
  Serial.begin(9600);
  
  start_page();
  savedTime = millis();

  pinMode(7,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  
  // PCF modules initialization
  pcfIn1.begin(0x20);
  pcfIn2.begin(0x21);
  pcfIn3.begin(0x22);

  pcfOut1.begin(0x38);
  pcfOut2.begin(0x39);
  pcfOut3.begin(0x3A);

  // Input PCF modules configuration
  pcfIn1.pinMode(0, INPUT);
  pcfIn1.pinMode(1, INPUT);
  pcfIn1.pinMode(2, INPUT);
  pcfIn1.pinMode(3, INPUT);
  pcfIn1.pinMode(4, INPUT);
  pcfIn1.pinMode(5, INPUT);
  pcfIn1.pinMode(6, INPUT);
  pcfIn1.pinMode(7, INPUT);

  pcfIn2.pinMode(0, INPUT);
  pcfIn2.pinMode(1, INPUT);
  pcfIn2.pinMode(2, INPUT);
  pcfIn2.pinMode(3, INPUT);
  pcfIn2.pinMode(4, INPUT);
  pcfIn2.pinMode(5, INPUT);
  pcfIn2.pinMode(6, INPUT);
  pcfIn2.pinMode(7, INPUT);

  pcfIn3.pinMode(0, INPUT);
  pcfIn3.pinMode(1, INPUT);
  pcfIn3.pinMode(2, INPUT);
  pcfIn3.pinMode(3, INPUT);
  pcfIn3.pinMode(4, INPUT);
  pcfIn3.pinMode(5, INPUT);
  pcfIn3.pinMode(6, INPUT);
  pcfIn3.pinMode(7, INPUT);

  // Output PCF modules configuration
  pcfOut1.pinMode(0, OUTPUT);
  pcfOut1.pinMode(1, OUTPUT);
  pcfOut1.pinMode(2, OUTPUT);
  pcfOut1.pinMode(3, OUTPUT);
  pcfOut1.pinMode(4, OUTPUT);
  pcfOut1.pinMode(5, OUTPUT);
  pcfOut1.pinMode(6, OUTPUT);
  pcfOut1.pinMode(7, OUTPUT);

  pcfOut2.pinMode(0, OUTPUT);
  pcfOut2.pinMode(1, OUTPUT);
  pcfOut2.pinMode(2, OUTPUT);
  pcfOut2.pinMode(3, OUTPUT);
  pcfOut2.pinMode(4, OUTPUT);
  pcfOut2.pinMode(5, OUTPUT);
  pcfOut2.pinMode(6, OUTPUT);
  pcfOut2.pinMode(7, OUTPUT);

  pcfOut3.pinMode(0, OUTPUT);
  pcfOut3.pinMode(1, OUTPUT);
  pcfOut3.pinMode(2, OUTPUT);
  pcfOut3.pinMode(3, OUTPUT);
  pcfOut3.pinMode(4, OUTPUT);
  pcfOut3.pinMode(5, OUTPUT);
  pcfOut3.pinMode(6, OUTPUT);
  pcfOut3.pinMode(7, OUTPUT);

  
}

// Main program loop
void loop()
{
  actualTime = millis();
  
  if (actualTime > savedTime+refreshTime) {
    MainScreen();
    savedTime=actualTime;
  }
  modeChange();

}

// Running welcome page and testing diode before tester starts
void start_page()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("     ZiKE Labs");
  lcd.setCursor(0,2);
  lcd.print(" Tester okablowania");
  lcd.setCursor(0,3);
  lcd.print("  By Rafal Mlicki");

  StripGreen();
  delay(1000);
  StripRed();
  delay(1000);
  StripBlue();
  delay(1000);
  StripBlackOut();
  
  delay(2000);
}

// Set diode to green color
void StripGreen() {
  ledDiode.SetPixelColor(0, RgbColor(0, 255, 0));
  ledDiode.Show();
}

// Set diode to red color
void StripRed() {
  ledDiode.SetPixelColor(0, RgbColor(255, 0, 0));
  ledDiode.Show();
}

// Set diode to blue color
void StripBlue() {
  ledDiode.SetPixelColor(0, RgbColor(0, 0, 255));
  ledDiode.Show();
}

// Set diode off
void StripBlackOut() {
  ledDiode.SetPixelColor(0, RgbColor(0, 0, 0));
  ledDiode.Show();
}

// Display main screen and run selected mode 
void MainScreen() {
  switch (mode) {
    case 0:
      refreshTime=250;
      cableTest(3,"Tryb: XLR           ");
      break;

    case 1:
      refreshTime=250;
      cableTest(6,"Tryb: P2            ");
      break;

    case 2:
      refreshTime=250;
      cableTest(12,"Tryb: P4            ");
      break;

    case 3:
      refreshTime=250;
      cableTest(24,"Tryb: P8            ");
      break;

    case 4:
      refreshTime=250;
      cableTest(4,"Tryb: NL4           ");
      break;

    case 5:
      refreshTime=250;
      cableTest(8,"Tryb: Eth           ");
      break;

    case 6:
      refreshTime=250;
      cableTest(9,"Tryb: Eth+gnd       ");
      break;

    case 7:
      refreshTime=50;
      intermission();
      break;
      
    case 8:
      refreshTime=250;
      pcfTest();
      break;
  }
}

// Checking connections, writing to connection map
void connectionCheck(int n) {
  pinSetHigh(n);
  isError = false;
  for (int i = 0; i < n; i++) {
    pcfDigitalWriteLow(i);
    for (int j = 0; j < n; j++) {
      if (pcfDigitalRead(j) == true) {
        connectionMap[i][j] = true;
        if (i != j) {
          isError = true;
        }
      } else {
        connectionMap[i][j] = false;
        if (i == j) {
          isError = true;
        }
      }
    }
    pcfDigitalWriteHigh(i);
  }
}

// Detecting pluged in cable
boolean ifCable(int pin){
  pinSetHigh(pin);
  for (int i = 0; i < pin; i++) {
    pcfDigitalWriteLow(i);
    if (pcfDigitalRead(i) == true) {
      pcfDigitalWriteHigh(i);
      return true;
    }
    pcfDigitalWriteHigh(i);
  }
  return false; 
}

// Analyze results of connection map and print errors
void printErrors(int channelQuantity) {
  int connectionCount = 0;
  int connections[channelQuantity];
  for (int row = 0; row < channelQuantity; row++){
    for (int column = 0; column < channelQuantity; column++) {
      if(connectionMap[row][column]){
        connectionCount++;
        connections[connectionCount-1] = column;
      }
    }
    if(connectionCount == 0){
      lcd.print(row+1);
      lcd.print("o ");
    } else if (connectionCount == 1){
        if(row != connections[0]){
          lcd.print(row+1);
          lcd.print(F("c"));
          lcd.print(connections[0]+1);
          lcd.print(F(" "));
        }
    } else if(connectionCount == 2){
        if (row == connections[0]){
          lcd.print(row+1);
          lcd.print(F("s"));
          lcd.print(connections[0]+1);
          lcd.print(F(" "));
        } else if (row == connections[1]){
          lcd.print(row+1);
          lcd.print(F("s"));
          lcd.print(connections[1]+1);
          lcd.print(F(" "));
        } else {
          lcd.print(row+1);
          lcd.print(F("c"));
          lcd.print(connections[0]+1);
          lcd.print(F(" "));
          lcd.print(row+1);
          lcd.print(F("c"));
          lcd.print(connections[1]+1);
          lcd.print(F(" "));
        }
    } else {
      for (int connection = 0; connection < connectionCount; connection++){
        if (row != connections[connection]){
          lcd.print(row+1);
          lcd.print("c");
          lcd.print(connections[connection]+1);
          lcd.print(" ");
        }
      }
    }
    connectionCount = 0;    
  }
}

// PCF module output digital write low
void pcfDigitalWriteLow(int i) {
  if (i >= 0 && i <= 7) {
    pcfOut1.digitalWrite(i, LOW);
  } else if (i >= 8 && i <= 15) {
    pcfOut2.digitalWrite(i - 8, LOW);
  } else if (i >= 16 && i <= 23) {
    pcfOut3.digitalWrite(i - 16, LOW);
  }
}

// PCF module output digital write high
void pcfDigitalWriteHigh(int i) {
  if (i >= 0 && i <= 7) {
    pcfOut1.digitalWrite(i, HIGH);
  } else if (i >= 8 && i <= 15) {
    pcfOut2.digitalWrite(i - 8, HIGH);
  } else if (i >= 16 && i <= 23) {
    pcfOut3.digitalWrite(i - 16, HIGH);
  }
}

// PCF module input read
boolean pcfDigitalRead(int i) {
  if (i >= 0 && i <= 7) {
    if (pcfIn1.digitalRead(i) == LOW) {
      return true;
    }
  } else if (i >= 8 && i <= 15) {
    if (pcfIn2.digitalRead(i - 8) == LOW) {
      return true;
    }
  } else if (i >= 16 && i <= 23) {
    if (pcfIn3.digitalRead(i - 16) == LOW) {
      return true;
    }
  }
  return false;
}

// Testing PCF modules, all outputs active, all inputs read and print to lcd
void pcfTest(){
  lcd.setCursor(0,0);
  lcd.print("Tryb: PCF test      ");
  lcd.setCursor(0,2);
  lcd.print("Polaczone piny:     ");
  lcd.setCursor(0,3);
  lcd.print("                    ");
  lcd.setCursor(0,3);
  for (int i = 0; i < 24; i++) {
    pcfDigitalWriteLow(i);
  }
  int n=0;
  for (int i = 0; i < 24; i++) {
    if (pcfDigitalRead(i)) {
      n++;
      if (n==7){
        break;
      }
      lcd.print(i+1);
      lcd.print(" ");
    }
  }
}

// Testing "n" pin cable with "text" describe
void cableTest(int n, char text[]){
  lcd.setCursor(0,0);
  lcd.print(text);
  if (ifCable(n)) {
    connectionCheck(n);
    if(isError){
      lcd.setCursor(0,2);
      lcd.print("      ERROR!        ");
      StripRed();
      lcd.setCursor(0,3);
      lcd.print("                    ");
      lcd.setCursor(0,3);
      printErrors(n);
    } else {
      lcd.setCursor(0,2);
      lcd.print("      OK!           ");
      lcd.setCursor(0,3);
      lcd.print("                    ");
      StripGreen();
      Serial.println("OK!");
    }
  } else {
    lcd.setCursor(0,2);
    lcd.print("    no cable!       ");
    lcd.setCursor(0,3);
    lcd.print("                    ");
    StripBlackOut();
    Serial.println("nocable");
  }
}

// Seting n successive pin to high
void pinSetHigh(int n){
  for (int i = 0; i < n; i++) {
    pcfDigitalWriteHigh(i);
  }
}

// Seting n successive pin to low
void pinSetLow(int n){
  for (int i = 0; i < n; i++) {
    pcfDigitalWriteLow(i);
  }
}

// Testing cable, checking for short connection lost
void intermission(){
  lcd.setCursor(0,0);
  lcd.print("Detekcja przerwan   ");
  if (intermittentOn) {
    StripBlue();
  } else {
    StripRed();
  }

  if (buttonLed()){
    interCount=0;
    interLastPin=0;
    intermittentOn=true;

    pinSetLow(24);
    for (int i = 0; i < 24; i++){
      intermittentMap[i]=pcfDigitalRead(i);
    }
  }

  for (int i = 0; i < 24; i++) {
    if (intermittentMap[i]!=pcfDigitalRead(i)){
      interCount++;
      interLastPin=i+1;
      intermittentOn=false;
    }
  }
  lcd.setCursor(0,2);
  lcd.print("Ilosc przerwan: ");
  lcd.print(interCount);
  lcd.print("   ");
  lcd.setCursor(0,3);
  lcd.print("Przerwany pin: ");
  lcd.print(interLastPin);
  lcd.print("   ");

}

// Button 1 detect 
boolean buttonLed(){
  if (digitalRead(7)==LOW){
    return true;
  }
  else {
    return false;
  }
}

// Button 2 detect
boolean buttonEncoder(){
  if (digitalRead(8)==LOW){
    return true;
  }
  else {
    return false;
  }
}

// Changing mode on button press
void modeChange() {
  if (buttonEncoder()==true){
    mode++;
    delay(200);
  } 
  if (mode==9) {
    mode=0;
  }
}