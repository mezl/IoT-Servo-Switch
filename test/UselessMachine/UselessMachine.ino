/* UselessMachine.ino : This will turn off switch when user turn it on
 * Chin-Kai Chang <mezlxx@gmail.com> 
 *
 * This code is used for testing smart servo switch
 * When wired the test switch, it can be the "Useless" machine.
 *
 *  Servo Pin 14
 *  Switch Pin 12
 *
 * 2018/06/24
*/

#include <Servo.h>

//#define FEATHER
#define BAUD 57600



#define SERVO_PIN 14
#define LIGHT_SWITCH_PIN 12

#define POS_OFF 160
#define POS_ON 30

#ifdef ESP8266
#define INTERRUPT_PIN 15  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 0 // (Arduino is 13, Teensy is 11, Teensy++ is 6,Pro Micro RXLED 17)
#define FEATHER
#else
#define INTERRUPT_PIN 0  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 17 // (Arduino is 13, Teensy is 11, Teensy++ is 6,Pro Micro RXLED 17)
#endif

#ifdef FEATHER
#include <Adafruit_FeatherOLED.h>
Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 16
#define BUTTON_C_PIN 2
#endif

#ifdef FEATHER

#endif


Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
char buff[20];
int back_track = 50;
boolean switch_on = false;
boolean toggleMode = false;
int t_success = 0;
int t_fail = 0;
int t_count = 0;


void initOLED()
{
#ifdef FEATHER
  // Initialize oled
  oled.init();
  oled.setBatteryVisible(false);
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Initializing I2C...");
  oled.display();
#endif
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  pinMode(BUTTON_C_PIN, INPUT);

}
void printLcd(const __FlashStringHelper * str, bool clear = false)
{
#ifdef FEATHER
  // Initialize oled
  if (clear) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
  }
  oled.println(str);
  oled.display();
  ESP.wdtFeed();//reset watch dog
#endif

}
void printLCD2(char* str, bool clear = false)
{
#ifdef FEATHER
  // Initialize oled
  if (clear) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
  }
  oled.println(str);
  oled.display();
  ESP.wdtFeed();//reset watch dog
#endif

}
void setup() {
  Serial.begin(BAUD);
  Serial.println(F("HELLO Smart Light..."));
  myservo.attach(SERVO_PIN, 500, 2500);  //the pin for the servo control, and range if desired
  Serial.println(F("Attached Servo"));

  initOLED();
  printLcd(F("Initializing LCD..."));

  pinMode(LIGHT_SWITCH_PIN, INPUT_PULLUP);

  pos = POS_OFF;
  myservo.write(pos);
  delay(1500);
  pos = POS_ON;
  myservo.write(pos);
  delay(1500);

  
}

void turnOnSwitch(){

    printLcd(F("Turn On Switch"),false);
    pos = POS_ON;
    myservo.write(pos);
    
    
    delay(500);  

    pos = POS_ON + back_track;
    myservo.write(pos);
    
    
    delay(500);

    pos = POS_ON-20;
    myservo.write(pos);

    
    delay(1500);
    switch_on = true;

    checkSwitchState();

}

void turnOffSwitch(){
    printLcd(F("Turn Off Switch"),false);
    pos = POS_OFF;
    myservo.write(pos);
    delay(500);  

    pos = POS_OFF- back_track;
    myservo.write(pos);
    delay(500);

    pos = POS_OFF+20;
    myservo.write(pos);
    delay(1500);

    switch_on = false;

    checkSwitchState();
}

void toggleSwitch(){
  if(switch_on){
    
    turnOffSwitch();
   }else{
    turnOnSwitch();     
   }  
}

bool checkSwitchState(){
  return (digitalRead(LIGHT_SWITCH_PIN) == LOW);
}
void loop() {
  
if(checkSwitchState()){
    printLcd(F("Light Switch is on"),true);
    turnOffSwitch();
}else{
    printLcd(F("Light Switch is off"),true);
}
  delay(1000);

}

