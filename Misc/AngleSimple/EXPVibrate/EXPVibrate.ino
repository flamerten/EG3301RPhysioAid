#include "BluetoothSerial.h"
#include <MadgwickAHRS.h> //https://github.com/arduino-libraries/MadgwickAHRS
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LC709203F.h> //Bat indicator
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

BluetoothSerial SerialBT;

//Feather
#define NEOPIXEL_I2C_POWER 2
#define LED_BUILTIN 13
bool led_on = true;

//Vibration
#define VIBRATE 4 //A5
#define VIBRATE_TIME 1000 //Vibrate for 1000 sec
uint8_t ANGLELOW = 60; //Lower than 60 degrees send out vibration, change through the RPI(?)
bool vibrate_on = false;
unsigned long int time_last_start_vibrate = millis();

//Deep sleep
#define SLEEP_BUTTON 26 //A0 
#define BUTTON_PIN_BITMASK 0x8000000000 // 2^39 in hex
volatile bool go_to_sleep = false; //For the ISR

void IRAM_ATTR SLEEP_ISR(){
  //GPIO interrupt that changes the variable go to sleep
  go_to_sleep = true;  
}

void Setup_DEEPSLEEP(){
  pinMode(SLEEP_BUTTON,INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,LOW); //Wake up when button pulled low
  attachInterrupt(SLEEP_BUTTON, SLEEP_ISR, FALLING); //Interrupt and go to sleep when button pulled low

}
void CheckSleep(){
    if(go_to_sleep){
        digitalWrite(NEOPIXEL_I2C_POWER,LOW); //Turn of i2c
        Serial.println("Going to sleep");
        delay(1000);
        esp_deep_sleep_start();
    }
}

//LSM IMU
Adafruit_LSM6DSOX sox; //declare sox object
sensors_event_t accel, gyro, temp;
unsigned long int imu_read_time = millis();

//Battery Indicator
Adafruit_NeoPixel onboard_pixel(1, 0, NEO_GRB + NEO_KHZ800); //Use on board neopixel
#define BRIGHTNESS 50

//Batt Measurement
Adafruit_LC709203F lc;
unsigned const int BAT_TIME_DELAY_MILLIS = 5 * 60 * 1000; //Query every 5 mins
const float CUT_OFF_VOLTAGE = 3.3; //Source: https://forums.adafruit.com/viewtopic.php?f=19&p=545995 
unsigned long int last_bat_query_time = 0;
int bat_percentage; float bat_voltage;


void setup() {
  Serial.begin(115200);
  SerialBT.begin("EIM_Vibrate"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  Wire.begin(22,20);
  go_to_sleep = false;
  Setup_DEEPSLEEP();

  
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  pinMode(VIBRATE,OUTPUT);
  digitalWrite(VIBRATE,LOW);

  //Enable I2C for the components
  pinMode(NEOPIXEL_I2C_POWER,OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER,HIGH);

  
  delay(100); //give time for i2c to init
  

  if (!sox.begin_I2C()){
    Serial.println("Unable to find LSM");
    delay(1000);   //call reset if unable to find item to reinit
  }
  if (!lc.begin()) {
    Serial.println(F("Couldnt find Adafruit LC709203F?\nMake sure a battery is plugged in!"));
    delay(1000);   //call reset if unable to find item to reinit
  }
  lc.setPackSize(LC709203F_APA_1000MAH);  //Acutal pack size is 1500 but we just go under

  go_to_sleep = false;

  delay(1000);
  Serial.println("START");
}

void loop(){

    if(SerialBT.available()){
        char incomingChar = SerialBT.read();
        if(incomingChar == 'x'){
            vibrate_on = true;
            time_last_start_vibrate = millis();
        }

    }

    if(vibrate_on){
        digitalWrite(VIBRATE,HIGH);
        if(millis() - time_last_start_vibrate >= 1000){
            vibrate_on = false;
        }
    }
    else{
        digitalWrite(VIBRATE,LOW);
    }

    CheckSleep();
}
