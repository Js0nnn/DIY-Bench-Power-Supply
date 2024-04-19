#include <ACS712.h>
#include "LiquidCrystal.h"

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define button_B    A0
#define button_D    A2
#define button_U    A1
#define button_E    A3 

#define pwr_led 13
#define cc_led 12
#define psu 8
#define fan 9
#define load 11
#define cc 10

//#define ONBOARD_LED 13

#define DEFAULT_DELAY 300

bool leds;
bool cc_sw=false;
bool load_sw=false;
bool psu_sw=true;
bool fan_sw=false;

float c=0,v=0;
float value=0.00;
float voltage=0.00;
float current=0.00;
float power=0.00;
const float R1=99.9;
const float R2=19.83;
const float R1i=100;
const float R2i=20;

const float K=((R1+R2)/R2)/((R1i+R2i)/R2i);

const float AVCC=4.99;


ACS712 sensor(ACS712_20A, A5);

char buttonPressed = '0';
char button;

byte menuLevel = 0;     // Level 0: no menu display, display anything you like
                        // Level 1: display main menu
                        // Level 2: display sub menu

byte menu = 1;
byte sub = 1;

bool islow=false;

bool LED_STATE = false;

bool currState_B = HIGH;
bool currState_D = HIGH;
bool currState_U = HIGH;
bool currState_E = HIGH;
          
bool prevState_B = HIGH; 
bool prevState_D = HIGH; 
bool prevState_U = HIGH; 
bool prevState_E = HIGH; 

unsigned long lastcalmillis=0;

unsigned long prevTime_B = 0;
unsigned long prevTime_D = 0;
unsigned long prevTime_U = 0;
unsigned long prevTime_E = 0;

unsigned long waitTime_B = 50;
unsigned long waitTime_D = 50;
unsigned long waitTime_U = 50;
unsigned long waitTime_E = 50;


void setup() 
{
  lcd.begin(20,4);
  pinMode(button_B, INPUT_PULLUP);
  pinMode(button_D, INPUT_PULLUP);
  pinMode(button_U, INPUT_PULLUP);
  pinMode(button_E, INPUT_PULLUP);
  pinMode(psu, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(load, OUTPUT);
  pinMode(cc, OUTPUT);
  pinMode(cc_led, OUTPUT);
  pinMode(pwr_led, OUTPUT);

  digitalWrite(psu, LOW);
  digitalWrite(fan, HIGH);
  digitalWrite(load, HIGH);
  digitalWrite(cc, HIGH);

  lcd.setCursor(4,0);
  lcd.print("ARDUINO BENCH");
  lcd.setCursor(4,1);
  lcd.print("POWER SUPPLY");
  lcd.setCursor(4,2);
  lcd.print("VERSION: 2.0");

  sensor.calibrate();

  delay(2000);
  lcd.clear();

  showHomeScreen();
  
}


void loop() {

  checkButton();

}

void checkButton() 
{
//  if(digitalRead(button_B)==LOW)
//  {
//    while(digitalRead(button_B)==LOW){}
//    buttonPressed='B';
//  }
//
//  if(digitalRead(button_D)==LOW)
//  {
//    while(digitalRead(button_D)==LOW){}
//    buttonPressed='D';
//  }
//
//  if(digitalRead(button_U)==LOW)
//  {
//    while(digitalRead(button_U)==LOW){}
//    buttonPressed='U';
//  }
//
//  if(digitalRead(button_E)==LOW)
//  {
//    while(digitalRead(button_E)==LOW){}
//    buttonPressed='E';
//  }
  // Button Debouncing
  bool currRead_B = digitalRead(button_B);
  bool currRead_D = digitalRead(button_D);
  bool currRead_U = digitalRead(button_U);
  bool currRead_E = digitalRead(button_E);

  if (currRead_B != prevState_B) {
    prevTime_B = millis();
  }
  if (currRead_D != prevState_D) {
    prevTime_D = millis();
  }
  if (currRead_U != prevState_U) {
    prevTime_U = millis();
  }
  if (currRead_E != prevState_E) {
    prevTime_E = millis();
  }

  if ((millis() - prevTime_B) > waitTime_B) {
    if (currRead_B != currState_B) {
      currState_B = currRead_B;
      if (currState_B == LOW) {
        buttonPressed = 'B';
      } 
    }
  } else buttonPressed = '0';
  if ((millis() - prevTime_D) > waitTime_D) {
    if (currRead_D != currState_D) {
      currState_D = currRead_D;
      if (currState_D == LOW) {
        buttonPressed = 'D';
      } 
    }
  } else buttonPressed = '0';
  if ((millis() - prevTime_U) > waitTime_U) {
    if (currRead_U != currState_U) {
      currState_U = currRead_U;
      if (currState_U == LOW) {
        buttonPressed = 'U';
      } else {
        //buttonPressed = '0';
      }
    }
  } else buttonPressed = '0';
  if ((millis() - prevTime_E) > waitTime_E) {
    if (currRead_E != currState_E) {
      currState_E = currRead_E;
      if (currState_E == LOW) {
        buttonPressed = 'E';
      } 
    }
  } else buttonPressed = '0';

  prevState_B = currRead_B;
  prevState_D = currRead_D;
  prevState_U = currRead_U;
  prevState_E = currRead_E;

   processButton(buttonPressed);
}

void processButton(char buttonPressed) 
{

  switch(menuLevel) {
    case 0: // Level 0
      switch ( buttonPressed ) {
        case 'E': // Enter
          menuLevel = 1;
          menu = 1;
          islow=true;
          updateMenu();
          delay(DEFAULT_DELAY);
          break;
        case 'U':
          loadOnOff();
          showHomeScreen();
          // Up
          delay(DEFAULT_DELAY);
          break;
        case 'D': 
          constantCurrent();// Down
          showHomeScreen();
          delay(DEFAULT_DELAY);
          break;
        case 'B': // Back
          //powersupply();
          break;
        default:
          break;
      }
      break;
    case 1: // Level 1, main menu
      switch ( buttonPressed ) {
        case 'E': // Enter
          updateSub();
          menuLevel = 2;  // go to sub menu
          updateSub();
          delay(DEFAULT_DELAY);
          break;
        case 'U': // Up
          menu++;
          updateMenu();
          delay(DEFAULT_DELAY);
          break;
        case 'D': // Down
          menu--;
          updateMenu();
          delay(DEFAULT_DELAY);
          break;
        case 'B': // Back
          menuLevel = 0;  // hide menu, go back to level 0
          lcd.clear();
          showHomeScreen();
          delay(DEFAULT_DELAY);
          break;
        default:
          break;
      } 
      break;
    case 2: // Level 2, sub menu
      switch ( buttonPressed ) {
        case 'E': 
          menuLevel = 1;
          updateMenu();
          delay(DEFAULT_DELAY);
          break;
        case 'U': // U
          if (menu == 1) {
            if (psu_sw==true) {  // 1 hour max
              psu_sw = false;
              digitalWrite(psu,HIGH);
            } else {
              psu_sw = true;
              digitalWrite(psu,LOW);
            }
          } else if (menu == 2) {       
            if (leds == true) {  // 1 hour max
              leds = false;
            } else {
              leds = true;
            }
          } else if (menu == 3) {
            if (fan_sw == true) {  // 1 hour max  
              fan_sw = false;
              digitalWrite(fan,HIGH);
            } else {
              fan_sw = true;
              digitalWrite(fan,LOW);
            }
          }
          updateSub();
          delay(DEFAULT_DELAY);
          break;
        case 'D': // D
          if (menu == 1) {
            if (psu_sw == true) {
              psu_sw = true;
            } else {
              psu_sw = false;
            }
          } else if (menu == 2) {
            if (leds == true) {
              leds = true;
            } else {
              leds = false;
            }
          } else if (menu == 3) {
            if (fan_sw == true) {
              fan_sw = true;
            } else {
              fan_sw = false;
            }
          }
          updateSub();
          delay(DEFAULT_DELAY);
          break;
        case 'B': // L
          menuLevel = 1;  // go back to main menu
          updateMenu();
          delay(DEFAULT_DELAY);
          break;
        default:  
          break;
      } 
      break;
    case 3: // Level 3, sub menu of sub menu
    
      break;
    default:
      break;
  }
  
}

void updateMenu() 
{
  
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("SETTINGS");
      lcd.setCursor(0,1);
      lcd.print(">Power Supply: ");

      if(psu_sw == true)
      lcd.print("ON");
      else
      lcd.print("OFF");

      lcd.setCursor(0, 2);
      lcd.print(" Max Current");
      lcd.setCursor(0,3);
      lcd.print(" Cooling Fan");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("SETTINGS");
      lcd.setCursor(0,1);
      lcd.print(" Power Supply");
      lcd.setCursor(0, 2);
      lcd.print(">Max Current: ");
      
      if(leds == true)
      lcd.print("ON");
      else
      lcd.print("OFF");

      lcd.setCursor(0,3);
      lcd.print(" Cooling Fan");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("SETTINGS");
      lcd.setCursor(0,1);
      lcd.print(" Power Supply");
      lcd.setCursor(0, 2);
      lcd.print(" Max Current");
      lcd.setCursor(0,3);
      lcd.print(">Cooling Fan: ");

      if(fan_sw == true)
      lcd.print("ON");
      else
      lcd.print("OFF");

      break;
    case 4:
      menu = 3;
      break;
  }
  
}

void updateSub() 
{
  switch (menu) {
    case 0:

      break;
    case 1:
      lcd.clear();
      lcd.print(" Power Supply:");
      lcd.setCursor(0, 1);
      lcd.print("  State: ");
      lcd.print(psu_sw);
      break;
    case 2:
      lcd.clear();
      lcd.print(" On-board LEDs:");
      lcd.setCursor(0, 1);
      lcd.print("  Val= ");
      lcd.print(leds);
      break;
    case 3:
      lcd.clear();
      lcd.print(" Cooling Fan:");
      lcd.setCursor(0, 1);
      lcd.print("  State: ");
      lcd.print(fan_sw);
      break;
    case 4:
      menu = 3;
      break;
  }
}



void showHomeScreen() 
{
  buttonPressed='0';

 
  while(buttonPressed=='0'){

     if(psu_sw==false)
  {
    lcd.setCursor(16,0);
    lcd.print("   ");
  }
  else
  {
    lcd.setCursor(16,0);
    lcd.print("PSU");
  }

  if(fan_sw==true)
  {
    lcd.setCursor(12,0);
    lcd.print("FAN");
  }
  else
  {
    lcd.setCursor(12,0);
    lcd.print("   ");
  }

  if(cc_sw==true)
  {
    lcd.setCursor(1,0);
    lcd.print("CC");
    lcd.setCursor(4,0);
    lcd.print("CV");
  }
  else
  {
    lcd.setCursor(1,0);
    lcd.print("  ");
    lcd.setCursor(4,0);
    lcd.print("  ");
  }
  
  if(load_sw==true)
  {
    lcd.setCursor(4,0);
    lcd.print("CV");
  }
  else
  {
    lcd.setCursor(4,0);
    lcd.print("  ");
  }

  if(leds==false)
  {
    digitalWrite(pwr_led,LOW);
    digitalWrite(cc_led,LOW);

  }
  else
  {
    continue;
  }



  
   if(millis()-lastcalmillis > 600)
   {
       current = ( sensor.getCurrentDC())-0.22;
       value=(float)analogRead(A4);
       voltage=(value*(AVCC/1023)*((R1+R2)/R2)*K); 

       lastcalmillis=millis();
   }
//       current=c/300;
//       voltage=v/300;

  if(voltage<0)voltage=0;
  if(current<0.1)current=0;
  power=voltage*current;

  

  lcd.setCursor(2, 1);
  lcd.print("Voltage: ");
  lcd.print(voltage, 2);
  lcd.print("V   ");

  lcd.setCursor(2, 2);
  lcd.print("Current: ");
  lcd.print(current, 2);
  lcd.print("A   ");

  lcd.setCursor(2, 3);
  lcd.print("Power: ");
  lcd.print(power, 2);
  lcd.print("W   ");


  if(current>3.00)
  {
    digitalWrite(fan,LOW);
    fan_sw=true;
  }


  if(current>9.00)
  {
    digitalWrite(load,HIGH);
    digitalWrite(cc,HIGH);
    cc_sw=false;
    load_sw=false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("****OVER LOADED!****");
    lcd.setCursor(1,3);
    lcd.print("REDUCE CURRENT!!");
    
    for(int i=0;i<5;i++)
    {
      digitalWrite(cc_led,HIGH);
      delay(250);
      digitalWrite(cc_led,LOW);
      delay(250);
    }
    lcd.clear();
  }
  
  checkButton();
  if(buttonPressed!='0')break;
  }

  
  
  }



void constantCurrent()
{
  if(cc_sw==false)
  {
    digitalWrite(cc,LOW);
    digitalWrite(cc_led, HIGH);
    digitalWrite(load,LOW);
    digitalWrite(pwr_led,HIGH);
    lcd.setCursor(1,0);
    lcd.print("CC");
    lcd.setCursor(4,0);
    lcd.print("CV");
    load_sw=true;
    cc_sw=true;
  }
  else
  {
    digitalWrite(cc,HIGH);
    digitalWrite(cc_led, LOW);
    digitalWrite(load,HIGH);
    digitalWrite(pwr_led,LOW);
    lcd.setCursor(1,0);
    lcd.print("  ");
    lcd.setCursor(4,0);
    lcd.print("  ");
    cc_sw=false;
    load_sw=false;
  }
  
}


void loadOnOff()
{
  if(load_sw==false)
  {
    digitalWrite(load,LOW);
    digitalWrite(pwr_led, HIGH);
    digitalWrite(cc,HIGH);
    digitalWrite(cc_led,LOW);
    lcd.setCursor(1,0);
    lcd.print("  ");
    lcd.setCursor(4,0);
    lcd.print("CV");
    load_sw=true;
    cc_sw=false;
  }
  else
  {
    digitalWrite(load,HIGH);
    digitalWrite(pwr_led, LOW);
    lcd.setCursor(4,0);
    lcd.print("  ");
    load_sw=false;
  }
  
}
