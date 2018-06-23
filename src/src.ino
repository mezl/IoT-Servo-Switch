/* Smart Servo Light Switch
  by Chin-Kai Chang <mezlxx@gmail.com>

  This use micro servo with ESP8266 to control
  
*/

#include <Servo.h>

//#define FEATHER
#define BAUD 57600

//GPIO0-GPIO15 can be INPUT, OUTPUT, or INPUT_PULLUP. GPIO16 can be INPUT, OUTPUT, or INPUT_PULLDOWN_16. I

#define SERVO_PIN 14
#define LIGHT_SWITCH_PIN 12

#define POS_OFF 160
#define POS_ON 30

#ifdef ESP8266
#define INTERRUPT_PIN 15  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 0 // (Arduino is 13, Teensy is 11, Teensy++ is 6,Pro Micro RXLED 17)
#define FEATHER
#define OLED_WING

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
#include <ESP8266WiFi.h>
#include "wifi_info.h"


#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
String webString="";     // String to display
#include "webhtml.h";
//#include "webjs.h";
#include "webcss.h";


#include <fauxmoESP.h>
fauxmoESP fauxmo;

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
#ifdef OLED_WING
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
#ifdef OLED_WING
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
#ifdef OLED_WING
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

void wifiSetup() {
#ifdef ESP8266
    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);
 
    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    sprintf(buff,"Connecting to %s",WIFI_SSID);
    printLCD2(buff,true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);



 
    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        printLcd(F("."));
        delay(100);
    }
    Serial.println();
 
    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    sprintf(buff,"%s",WiFi.localIP().toString().c_str());
    printLCD2(buff,true);

    fauxmo.addDevice("dorm");
    fauxmo.onMessage(callback);
    
#endif
}


void handle_root() {
  server.send(200, "text/plain", "Try /on or /off");
  delay(100);
}

void webServerSetup(){
  
  server.on("/on", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    turnOnSwitch();
    webString="Turn On Light";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
 
  server.on("/off", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    turnOffSwitch();
    webString="Turn Off Light";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);               // send to someones browser when asked
  });  


  server.on("/", []() {                     //Define the handling function for root path (HTML message)
    server.send(200, "text/html", htmlMessage);
  });
/*
  server.on("/javascript", []() { //Define the handling function for the javascript path
  server.send(200, "text/html", javascriptCode);
  });
*/
  server.on("/style.css", []() { //Define the handling function for the CSS path
    server.send(200, "text/css", cssButton);
  });










  server.begin();
}


void setup() {
  Serial.begin(BAUD);
  Serial.println(F("HELLO Smart Light2..."));
  //myservo.writeMicroseconds(1500); //set initial servo position if desired
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

  wifiSetup();
  webServerSetup();
  
}



void turnOnSwitch(){

    printLcd(F("Turn On Switch"),false);
    pos = POS_ON;
    myservo.write(pos);
    
    
    delay(500);  

    pos = POS_ON + back_track;
    myservo.write(pos);
    
    
    delay(300);

    pos = POS_ON-20;
    myservo.write(pos);

    
    delay(200);
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
    delay(300);

    pos = POS_OFF+20;
    myservo.write(pos);
    delay(200);

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
  
  t_count++;
  bool lightOn = (digitalRead(LIGHT_SWITCH_PIN) == LOW);
  bool result =  (lightOn == switch_on);

  result ? t_success++: t_fail++;

  return result;
  
}


void callback(uint8_t device_id, const char * device_name, bool state) {
  Serial.print("Device "); Serial.print(device_name); 
  Serial.print(" state: ");
  if (state) {
    Serial.println("ON");
    turnOnSwitch();   
  } else {
    Serial.println("OFF");
    turnOffSwitch();   
  }
}


void loop() {


  fauxmo.handle();
  server.handleClient();

  
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    printLcd(F("BTN_A Pressed"),true);
    turnOnSwitch();
  }
  if (digitalRead(BUTTON_B_PIN) == LOW) {
    printLcd(F("BTN_B Pressed"),true);
    toggleMode = !toggleMode;
  }
  if (digitalRead(BUTTON_C_PIN) == LOW) {
    printLcd(F("BTN_C Pressed"),true);
    turnOffSwitch();
  }
  
  if(toggleMode)
  {
    toggleSwitch();
  }

  sprintf(buff,"%s",WiFi.localIP().toString().c_str());
  printLCD2(buff,true);
  //sprintf(buff, "Try:%4d", t_count);
  //printLCD2(buff, true);
  sprintf(buff, "OK:%4d,Fail:%3d", t_success,t_fail);
  printLCD2(buff, false);
  Serial.println(buff);//this cause servo not working....
  delay(1000);

}

