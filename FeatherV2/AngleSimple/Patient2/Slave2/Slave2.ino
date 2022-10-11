/*
 * This Sketch is the slave, it samples and sends it over to the master
 * Note that the battery indicator and the pogo pin charging is removed
 * 
 */

#include <MadgwickAHRS.h> //https://github.com/arduino-libraries/MadgwickAHRS
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

//CHANGABLE
#define PATIENT_NO 2; //Distiguish between patient nos
uint8_t master[] = {0xE8,0x9F,0x6D,0x28,0x0C,0x98}; //Feather Master Unlableed

//Feather
#define NEOPIXEL_I2C_POWER 2
#define LED_BUILTIN 13
bool led_on = true;

//Deep sleep
#define SLEEP_BUTTON 26 
#define BUTTON_PIN_BITMASK 0x8000000000 // 2^39 in hex
volatile bool go_to_sleep = false; //For the ISR

//ESP-NOW
bool data_ready = false; //Flag when data is recieved
float slave_angle, joint_angle;
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

//Battery
Adafruit_NeoPixel onboard_pixel(1, 0, NEO_GRB + NEO_KHZ800);
int bat_percentage; float bat_voltage; int bat_reading;
#define BRIGHTNESS 20
#define ANALOG_BAT 35 //Analog built in to measure bat
const int BAT_TIME_DELAY_MILLIS = 1000 * 60;
int r_neo, g_neo, b_neo;
unsigned long int last_bat_query_time = 0;

//Magdwick 
Madgwick filter;
#define SAMPLE_FREQ 10
const float GRAVITY = 9.81;
//const float RAD_TO_DEG = 180/3.142; //Commented out i think its already defined 

const uint16_t IMU_TIME_DELAY_MILLIS = 1.0/SAMPLE_FREQ * 1000;
float ax, ay, az;
float gx, gy, gz;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void CheckBattery(bool check_now){
    //Simple Bat Comparisons
    //Use check_value to check_value at the start;
    
    bat_reading = map(analogRead(ANALOG_BAT),0,4096,0,33) *2; //0 - 33 ( 0 - 3.3 since map is int)
    bat_voltage = bat_reading / 10.0;

    if(bat_voltage <= 3.4){
        Serial.print("LOW BAT VOLTAGE: ");
        Serial.println(bat_voltage);
        go_to_sleep = true;
        return;
    }

    if( (millis() - last_bat_query_time >= BAT_TIME_DELAY_MILLIS) || check_now){
        if(bat_voltage >= 3.9)      {r_neo = 0;   g_neo = 100; b_neo = 0;}  //Green
        else if(bat_voltage >= 3.6) {r_neo = 100; g_neo = 100; b_neo = 0;}  //Yellow
        else                        {r_neo = 100; g_neo = 0;   b_neo = 0;}   //Red
    }
    
    onboard_pixel.setPixelColor(0,onboard_pixel.Color(r_neo,g_neo,b_neo)); 
    onboard_pixel.show();

}

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

void SetUpESPNOW(){
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    
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
  filter.updateIMU(gy, gx, gz, ay, ax, -1.0 * az);  //rotate  matrix
}


//Main Program
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(22,20);
  filter.begin(SAMPLE_FREQ);

  pinMode(ANALOG_BAT,INPUT);
  pinMode(LED_BUILTIN,OUTPUT);

  digitalWrite(LED_BUILTIN,HIGH); //Turn on to signal that we have started setup
  
  //Enable I2C for the components
  pinMode(NEOPIXEL_I2C_POWER,OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER,HIGH);
  delay(100); //give time for i2c to init
  

  if (!sox.begin_I2C()){
    Serial.println("Unable to find LSM");
    delay(1000);  resetFunc();  //call reset if unable to find item to reinit
  }
  
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

    
    Serial.print(roll);
    Serial.print(" ");
    Serial.print(pitch);
    Serial.println("");
    

    imu_read_time = millis(); //reset to time last read

    //message.angle = roll; //Lets stick with roll
    message.angle = pitch; //Lets stick with roll
    esp_now_send(master, (uint8_t *) &message, sizeof(message));

    //BLink each time sample is takken
    digitalWrite(LED_BUILTIN,led_on);
    led_on = !led_on;
    
  }

  CheckBattery(false);
  CheckSleep();

}
