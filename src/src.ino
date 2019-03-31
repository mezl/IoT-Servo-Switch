/* Smart Servo Light Switch
  by Chin-Kai Chang <mezlxx@gmail.com>

  This use micro servo with ESP8266 to control
  
*/

#include <Servo.h>

#define BAUD 115200 

//GPIO0-GPIO15 can be INPUT, OUTPUT, or INPUT_PULLUP. GPIO16 can be INPUT, OUTPUT, or INPUT_PULLDOWN_16. I


#define NODEMCU
#define POS_OFF 160
#define POS_ON 30

#define TESTING_MODE false 
#define LIGHT_SWITCH_PIN 0//only for testing


#ifdef ESP8266
    #define INTERRUPT_PIN 15  // use pin 2 on Arduino Uno & most boards
    #define LED_PIN 0 // (Arduino is 13, Teensy is 11, Teensy++ is 6,Pro Micro RXLED 17)
    //#define FEATHER
    //#define OLED_WING
#endif



#ifdef OLED_WING 
    #include <Adafruit_FeatherOLED.h>
    Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();
    //#define BUTTON_A_PIN 0
    //#define BUTTON_B_PIN 16
    //#define BUTTON_C_PIN 2
#endif

#ifdef NODEMCU
  #include <Adafruit_FeatherOLED.h>
  Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();

  //D0 GPIO-16
  //D1 GPIO-5
  

  
  #define BUTTON_A_PIN 0 
  #define BUTTON_B_PIN 4
  #define BUTTON_C_PIN 5
  //#define SDA_PIN D2
  //#define SCL_PIN D1
  #define SERVO_PIN 10
  #define OLED_RESET LED_BUILTIN
  #define OLED_WING
#endif

#ifdef ESP8266 
    #include <ESP8266WiFi.h>
    #include "wifi_info.h" //SSID & Wifi Password
    
    // Running Web Server
    #include <ESP8266WebServer.h>
    ESP8266WebServer server(80);
    String webString="";     // String to display
    #include "webhtml.h";
    //#include "webjs.h";
    #include "webcss.h";
    
    // Support Amazon Alexa using Sinric
    #include <Arduino.h>
    #include <ESP8266WiFiMulti.h>
    #include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
    #include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
    #include <StreamString.h>

    WebSocketsClient webSocket;
#endif



Servo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position
char buff[100];
int back_track = 50;
boolean switch_on = false;
boolean toggleMode = false;
int t_success = 0;
int t_fail = 0;
int t_count = 0;


//==============================================
//=============== Display Setup ================
//==============================================
void initOLED() {
#ifdef OLED_WING
  // Initialize oled
  oled.init();
  oled.setBatteryVisible(false);
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Initializing I2C...");
  oled.display();
  //USE Button with Feather HAUZZAH
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  pinMode(BUTTON_C_PIN, INPUT);

#endif
}
//==============================================
// For constand sting, using F("Put string here")
void printLcd(const __FlashStringHelper * str, 
                            bool clear = false,
                            bool newline = true) {
#ifdef OLED_WING
  // Initialize oled
  if (clear) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
  }
  newline ? oled.println(str) : oled.print(str);
  oled.display();
  ESP.wdtFeed();//reset watch dog
#endif

}
//==============================================
// For sprinf(buff,"%d ",some_int)
void printLCD2(char* str, bool clear = false) {
#ifdef OLED_WING
  // Initialize oled
  if (clear) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
  }
  oled.println(str);
  oled.display();
  //ESP.wdtFeed();//reset watch dog
#endif

}
//==============================================
void handleButton(){
#ifdef OLED_WING

  if (digitalRead(BUTTON_A_PIN) == LOW) {
    printLcd(F("BTN_A Pressed"),true);
    //turnOnSwitch();
    toggleSwitch();
  }
  if (digitalRead(BUTTON_B_PIN) == LOW) {
    printLcd(F("BTN_B Pressed"),true);
    toggleMode = !toggleMode;
  }
  if (digitalRead(BUTTON_C_PIN) == LOW) {
    printLcd(F("BTN_C Pressed"),true);
    turnOffSwitch();
  }
#endif
}

//==============================================
//=============== Wifi Setup ===================
//==============================================
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
        printLcd(F("."),false,false);
        delay(100);
    }
    Serial.println();
 
    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", 
            WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    sprintf(buff,"%s",WiFi.localIP().toString().c_str());
    printLCD2(buff,true);


#endif
}
//==============================================
//=============== Sinric Setup =================
//==============================================
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

void setPowerStateOnServer(String deviceId, String value);
void setTargetTemperatureOnServer(String deviceId, String value, String scale);
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      //printLcd(F("[WSc]disconnected"));
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      //printLcd(F("[WSc]Sinric connected.."));
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
        // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} 

        // For Light device type
        // Look at the light example in github
          
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if (action == "SetTargetTemperature") {
            String deviceId = json ["deviceId"];     
            String action = json ["action"];
            String value = json ["value"];
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
            printLcd(F("[WSc]Received Test"));
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary: %u\n", length);
      break;
  }
}
void sinricSetup()
{

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   

}
void sinricHandle()
{
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting
      // IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   

}

//==============================================
//=============== Web Server Setup =============
//==============================================
void webServerSetup(){
  
  server.on("/on", [](){   
    turnOnSwitch();
    webString="Turn On Light";   
    server.send(200, "text/plain", webString);
  });
 
  server.on("/off", [](){  
    turnOffSwitch();
    webString="Turn Off Light";   
    server.send(200, "text/plain", webString);
  });  

  //Define the handling function for root path (HTML message)
  server.on("/", []() {     
    server.send(200, "text/html", htmlMessage);
  });
    //Define the handling function for the CSS path
  server.on("/style.css", []() { 
    server.send(200, "text/css", cssButton);
  });

/*
  //Define the handling function for the javascript path
  server.on("/javascript", []() { 
  server.send(200, "text/html", javascriptCode);
  });
*/

  server.begin();
}

//==============================================
//=============== Servo Setup ==================
//==============================================
void turnOnSwitch(){
    printLcd(F("Turn On Switch"),false);
    // Move to farest pos
    pos = POS_ON;
    myservo.write(pos);
    delay(500);  
    // Back track to ensure switch toggled
    pos = POS_ON + back_track;
    myservo.write(pos);
    delay(300);
    // Move away from switch for manual operation
    pos = POS_ON-20;
    myservo.write(pos);
    delay(200);

    switch_on = true;
    t_count++;
    if(TESTING_MODE)checkSwitchState();
}
//==============================================
void turnOffSwitch(){
    printLcd(F("Turn Off Switch"),false);
    // Move to farest pos
    pos = POS_OFF;
    myservo.write(pos);
    delay(500);  
    // Back track to ensure switch toggled
    pos = POS_OFF- back_track;
    myservo.write(pos);
    delay(300);
    // Move away from switch for manual operation
    pos = POS_OFF+20;
    myservo.write(pos);
    delay(200);

    switch_on = false;
    t_count++;
    if(TESTING_MODE)checkSwitchState();
}
//==============================================
void toggleSwitch(){
  if(switch_on){
    turnOffSwitch();
   }else{
    turnOnSwitch();     
   }  
}
//==============================================
bool checkSwitchState(){
  
  bool lightOn = (digitalRead(LIGHT_SWITCH_PIN) == LOW);
  bool result =  (lightOn == switch_on);

  result ? t_success++: t_fail++;

  return result;
  
}
//==============================================
void turnOn(String deviceId) {
  if (deviceId == DORMLIGHT_ID) // Device ID of first device
  {  
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    turnOnSwitch();//actuall servo code
  } 
  else if (deviceId == "5axxxxxxxxxxxxxxxxxxx") // Device ID of second device
  { 
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    turnOffSwitch();
  }
  else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

//==============================================
void turnOff(String deviceId) {
   if (deviceId == DORMLIGHT_ID) // Device ID of first device
   {  
     Serial.print("Turn off Device ID: ");
     Serial.println(deviceId);
     turnOffSwitch();//actuall servo code
   }
   else if (deviceId == "5axxxxxxxxxxxxxxxxxxx") // Device ID of second device
   { 
     Serial.print("Turn off Device ID: ");
     Serial.println(deviceId);
  }
  else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);    
  }
}

//==============================================
//=============== Arduino Setup ================
//==============================================
void setup() {
  Serial.begin(BAUD);
  Serial.println(F("HELLO Smart Light..."));
  myservo.attach(SERVO_PIN, 500, 2500);
   
  Serial.println(F("Attached Servo"));

  initOLED();
  printLcd(F("Initializing LCD..."));

  // For testing switch on/off in test mode
  pinMode(LIGHT_SWITCH_PIN, INPUT_PULLUP);

  // Do one toggle test
  printLcd(F("Test Servo..."));
  pos = POS_OFF;
  myservo.write(pos);
  delay(1500);
  pos = POS_ON;
  myservo.write(pos);
  delay(1500);
  printLcd(F("Done Servo"));

  // Bring up Wifi and Web server
  wifiSetup();
  sinricSetup();
  webServerSetup();
  
}
//==============================================
//=============== Arduino Main =================
//==============================================
void loop() {


  sinricHandle();
  server.handleClient();
  handleButton();
  
  if(toggleMode)
  {
    toggleSwitch();
  }
  // Display IP Address on Screen
  sprintf(buff,"%s",WiFi.localIP().toString().c_str());
  printLCD2(buff,true);

  // Display Toggle Result 
    sprintf(buff, "Toggled: %d", t_count);
    printLCD2(buff, false);


  delay(1000);
}

// If you are going to use a push button to on/off the switch manually, use this function to update the status on the server
// so it will reflect on Alexa app.
// eg: setPowerStateOnServer("deviceid", "ON")
void setPowerStateOnServer(String deviceId, String value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
}

//eg: setPowerStateOnServer("deviceid", "CELSIUS", "25.0")
void setTargetTemperatureOnServer(String deviceId, String value, String scale) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["action"] = "SetTargetTemperature";
  root["deviceId"] = deviceId;
  
  JsonObject& valueObj = root.createNestedObject("value");
  JsonObject& targetSetpoint = valueObj.createNestedObject("targetSetpoint");
  targetSetpoint["value"] = value;
  targetSetpoint["scale"] = scale;
   
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
}

