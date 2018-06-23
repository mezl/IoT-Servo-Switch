# IoT-Servo-Switch-
Using Micro Servo and ESP8266 to toggle wall light switch (Taiwan/Japan style switch)

![Alt text](doc/SmartSwitch.png?raw=true "Smart Servo Switch")

**Features**
- Toggle Taiwan/Japan style light switch without modify it.
- Switch can be controlled by Amazon Alexa or webpage

**Software**
In order to use alexa control the switch, you need install 
- ESPAsyncTCP library 
- ESPAsyncUDP library 
- FauxMoESP library  
Please refer to [1] for more detail
**Hardware**
I use seperate coverplate with drilled two hole. 
So the original wall switch doesn't need any modification.

**Test**
Include "UselessMachine.ino" for testing servo toggle switch

**Reference**
[1]:https://learn.adafruit.com/easy-alexa-or-echo-control-of-your-esp8266-huzzah/software-setup
https://bitbucket.org/xoseperez/fauxmoesp
https://github.com/me-no-dev/ESPAsyncTCP

https://techtutorialsx.com/2016/10/15/esp8266-http-server-serving-html-javascript-and-css/
https://github.com/alexcican/lab/tree/gh-pages/switch

Please open an issue in this repository or write to mezlxx@gmail.com if you have any feedback
or problem with this repository. Your input is appreciated.
