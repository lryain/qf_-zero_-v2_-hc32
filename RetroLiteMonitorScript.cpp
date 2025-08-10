#define SDA_PIN 6 
#define SDA_PORT PORTA
#define SCL_PIN 4 
#define SCL_PORT PORTA
#include <SoftI2CMaster.h>

byte power_btn = 8; //Power button connected to this pin. Low Active
byte sys_on = 1; //Regulator power. Active High
byte sht_dwn = 2; //Connected to GPIO25. Signal to start Pi Shutdown. Active High

byte powerBtnState;
byte lowVoltInState = LOW;
byte lastLowVoltInState = LOW;
byte systemState = 0; //Low Power Off
bool shutdownInit = false;

unsigned long powerBtnTimer;
unsigned long shutDown;
long powerOnDelay = 1000;
long powerOffDelay = 3000;
long shutDownDelay = 10000;
bool btnTimerStarted = false;
bool shutDownTimerStarted = false;

unsigned long lastLowVoltDebounce = 0;
unsigned long debounceDelay = 50;

void setup() {
  //Serial.begin(9600);
  i2c_init();
  pinMode(power_btn, INPUT_PULLUP);
  pinMode(sys_on, OUTPUT);
  pinMode(sht_dwn, OUTPUT);
}

void loop() {
  powerButtonDebounce();
  lowVoltShutdownDebounce();
  
  if(!shutdownInit){
    if(powerBtnState){
      powerTimerCheck();
    } else {
      btnTimerStarted = false;
    }
  } else {
    shutdownTimer();
  }
  if(lowVoltInState){
    shutdownTimer();
  }
}

void powerTimerCheck(){
  if(!btnTimerStarted){
    btnTimerStarted = true;
    powerBtnTimer = millis();
  } else {
    if(systemState == 0){
      if(powerBtnTimer + powerOnDelay < millis()){
        systemState = 1;
        digitalWrite(sys_on, HIGH);
        btnTimerStarted = false;
      }
    } else {
      if(powerBtnTimer + powerOffDelay < millis()){
        systemState = 0;
        digitalWrite(sht_dwn, HIGH);
        btnTimerStarted = false;
        shutdownInit = true;
      }
    }
  }
}

void shutdownTimer(){
  if(!shutDownTimerStarted){
    shutDown = millis();
    shutDownTimerStarted = true;
    digitalWrite(sht_dwn, HIGH);//Tell Pi to Shut down
    shutdownInit = true;
  } else {
    if(shutDown + shutDownDelay < millis()){
      digitalWrite(sys_on, LOW);
      digitalWrite(sht_dwn, LOW);
      systemState = 0;
      shutDownTimerStarted = false;
      shutdownInit = false;
    }
  }
}

void powerButtonDebounce(){
  int input = !digitalRead(power_btn);
  if (input != powerBtnState){
    powerBtnState = input;
  }
}
