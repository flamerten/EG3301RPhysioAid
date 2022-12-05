/*
 * Master, have some solder from vibrator
 * NO VIBRATION
 * Still use roll as it has the largest range of angle but change the data collection method
 * 
 */

#include <MadgwickAHRS.h> //https://github.com/arduino-libraries/MadgwickAHRS
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LC709203F.h> //Bat indicator
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>


//Feather
#define NEOPIXEL_I2C_POWER 2
#define LED_BUILTIN 13
bool led_on = true;

//Deep sleep
#define SLEEP_BUTTON 38 //A0 for actual, 38 board sw for testing
volatile bool go_to_sleep = false; //For the ISR

//ESP-NOW
#define PATIENT_NO 1 //Distiguish between patient nos
uint8_t master[] = {0x4C,0x75,0x25,0xC5,0x02,0x20}; //USB M5 Matrix Bluetack

bool data_ready = false; //Flag when data is recieved
float slave_angle;
float joint_angle = 180;
float roll, pitch;

typedef struct struct_message{
    float angle;
    uint8_t patient_no;
    //The slave sends angle to master, master calculates joint angle and then 
    //Sends that to the master
    //Use mac address to distinguish different wearables // No need this
} struct_message; //define struct and then initalise it

struct_message message;
esp_now_peer_info_t peerInfo;

//LSM IMU
Adafruit_LSM6DSOX sox; //declare sox object
sensors_event_t accel, gyro, temp;
unsigned long int imu_read_time = millis();

//Magdwick 
Madgwick filter;
#define SAMPLE_FREQ 10
const float GRAVITY = 9.81;
//const float RAD_TO_DEG = 180/3.142; //Commented out i think its already defined 

const uint16_t IMU_TIME_DELAY_MILLIS = 1.0/SAMPLE_FREQ * 1000;
float ax, ay, az;
float gx, gy, gz;

//Battery Indicator
Adafruit_NeoPixel onboard_pixel(1, 0, NEO_GRB + NEO_KHZ800); //Use on board neopixel
#define BRIGHTNESS 50

//Batt Measurement
Adafruit_LC709203F lc;
bool USEBAT = true; //ability to ignore bat
unsigned const int BAT_TIME_DELAY_MILLIS = 5 * 60 * 1000; //Query every 5 mins
const float CUT_OFF_VOLTAGE = 3.3; //Source: https://forums.adafruit.com/viewtopic.php?f=19&p=545995 
#define USE_CUTOFF 0 //Dont use the cut off voltage
unsigned long int last_bat_query_time = 0;
int bat_percentage; float bat_voltage;


void(* resetFunc) (void) = 0; //declare reset function @ address 0

void CheckBattery(bool check_now){
    //Check_now used to just do a bat check during startup, from then on its false, until query time
    //First -> Check voltage, if too low shut off battery
    //Second, if query time, change the colour of the pixel

    if(!USEBAT) return;
    
    bat_voltage = lc.cellVoltage();

    if(bat_voltage <= CUT_OFF_VOLTAGE && USE_CUTOFF){
        Serial.print("LOW BAT VOLTAGE: ");
        Serial.println(bat_voltage);
        go_to_sleep = true;
        return;
    }
    

    if( ( (millis() - last_bat_query_time) >= BAT_TIME_DELAY_MILLIS) || check_now){
        
        bat_percentage = round(lc.cellPercent());
        if(bat_percentage == 100 && check_now) onboard_pixel.setPixelColor(0,onboard_pixel.Color(50,50,50));
        //White signify full charge, only signify full charge at the beginning to avoid
        //Confusion
        else if(bat_percentage > 75) onboard_pixel.setPixelColor(0,onboard_pixel.Color(0,100,0)); //Green
        else if(bat_percentage > 40)  onboard_pixel.setPixelColor(0,onboard_pixel.Color(100,100,0)); //Yellow
        else onboard_pixel.setPixelColor(0,onboard_pixel.Color(100,0,0)); //Red

        onboard_pixel.show();

        Serial.print("Bat Level is "); Serial.print(bat_percentage); Serial.println("%");
    }

}

void IRAM_ATTR SLEEP_ISR(){
  //GPIO interrupt that changes the variable go to sleep
  go_to_sleep = true;  
}

void Setup_DEEPSLEEP(){
  pinMode(SLEEP_BUTTON,INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_38,LOW); //Wake up when button pulled low
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

void SetUpESPNOW(){
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    memcpy(peerInfo.peer_addr, master, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add master");
        return;
    }    

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

    //Mostly debugging messages > don't do anything

    //Serial.print("\r\nLast Packet Send Status:\t");
    //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    
    return;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    //Data is only recieved once to init sampling
    
    memcpy(&message, incomingData, sizeof(message)); //Save data to struct

    slave_angle = message.angle;
    data_ready = true;
}

void CheckToSend(){
    if(data_ready){
      //Use roll
      joint_angle = abs(180 - int(abs(slave_angle - roll)));
      message.angle = joint_angle;
      esp_now_send(master, (uint8_t *) &message, sizeof(message));
      data_ready = false;
      Serial.print(slave_angle);
      Serial.print(" ");
      Serial.print(roll);
      Serial.print(" ");
      Serial.println(joint_angle);
    }
}

void get_readings(){
  //Get readings from LSM, convert them to parameters and update IMU with it
  sox.getEvent(&accel, &gyro, &temp);

  //Accel in m/s^2 > convert to g
  ax = accel.acceleration.x/GRAVITY;
  ay = accel.acceleration.y/GRAVITY;
  az = accel.acceleration.z/GRAVITY;

  //Gyro in rad/s > convert to deg/s
  gx = gyro.gyro.x * RAD_TO_DEG;
  gy = gyro.gyro.y * RAD_TO_DEG;
  gz = gyro.gyro.z * RAD_TO_DEG;
  
  //filter.updateIMU(gx, gy, gz, ax, ay, az);  
  filter.updateIMU(gy, gx, gz, ay, ax, -1.0 * az);
  //Swap the x and y readings, such that the roll i get is pitch, but the range is -180 to 180  
  //Rotate Matrix
}


//Main Program
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(22,20);
  filter.begin(SAMPLE_FREQ);


  pinMode(LED_BUILTIN,OUTPUT);

  digitalWrite(LED_BUILTIN,HIGH); //Turn on to signal that we have started setup
  
  //Enable I2C for the components
  pinMode(NEOPIXEL_I2C_POWER,OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER,HIGH);
  delay(100); //give time for i2c to init
  

  if (!sox.begin_I2C()){
    Serial.println("Unable to find LSM");
    delay(1000);    //call reset if unable to find item to reinit
  }
  if (!lc.begin()) {
    Serial.println(F("Couldnt find Adafruit LC709203F?\nMake sure a battery is plugged in!"));
    USEBAT = false;
    delay(1000);    //call reset if unable to find item to reinit
  }
  lc.setPackSize(LC709203F_APA_1000MAH);  //Acutal pack size is 1500 but we just go under
  
  onboard_pixel.begin();
  onboard_pixel.setBrightness(BRIGHTNESS);

  CheckBattery(true); //Just check for the first time and ignore the timing

  go_to_sleep = false;
  Setup_DEEPSLEEP();
  
  SetUpESPNOW();
  Serial.println("ESP-NOW setup");

  message.patient_no = PATIENT_NO;

  delay(1000); //End setup

  Serial.println("Setup done");

}

void loop() {
  // put your main code here, to run repeatedly:

  if( (millis() - imu_read_time) >= IMU_TIME_DELAY_MILLIS){

    get_readings();
    
    roll = filter.getRoll();
    pitch = filter.getPitch();

    /*
    Serial.print(roll);
    Serial.print(" ");
    Serial.print(pitch);
    Serial.println("");
    */
    

    imu_read_time = millis(); //reset to time last read

    //BLink each time sample is takken
    digitalWrite(LED_BUILTIN,led_on);
    led_on = !led_on;
    
  }

  CheckBattery(false);
  CheckSleep();
  CheckToSend(); //if data ready send

}
