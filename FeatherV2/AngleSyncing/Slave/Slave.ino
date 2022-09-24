/*
 * This Sketch is the slave, it sends data to the master and the master
 * sends data to USB M5
 * 
 */


#include <MadgwickAHRS.h> //https://github.com/arduino-libraries/MadgwickAHRS
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LC709203F.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

//Feather
#define NEOPIXEL_I2C_POWER 2
#define LED_BUILTIN 13
bool led_on = true;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//Deep sleep
#define SLEEP_BUTTON 38 
#define BUTTON_PIN_BITMASK 0x8000000000 // 2^39 in hex
volatile bool go_to_sleep = false; //For the ISR

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

//ESP-NOW
uint8_t master[] = {0xE8,0x9F,0x6D,0x1F,0xFC,0x00}; 


typedef struct struct_message{
    float angle;
    uint8_t mac_address[6];
    //The slave sends angle to master, master calculates joint angle and then 
    //Sends that to the master
    //Use mac address to distinguish different wearables
} struct_message; //define struct and then initalise it
struct_message message;
esp_now_peer_info_t peerInfo;

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



//LSM IMU
Adafruit_LSM6DSOX sox; //declare sox object
sensors_event_t accel, gyro, temp;
unsigned long int imu_read_time = millis();

//Battery Indicator
Adafruit_LC709203F lc;
#define BRIGHTNESS 50
#define ANALOG_BAT 35 //Analog built in to measure bat

unsigned const int BAT_TIME_DELAY_MILLIS = 5 * 60 * 1000; //Query every 5 mins
Adafruit_NeoPixel onboard_pixel(1, 0, NEO_GRB + NEO_KHZ800);
unsigned long int last_bat_query_time = 0;

int bat_percentage; float bat_voltage; int bat_reading;
void CheckBattery(bool check_now){
    //Check for low bat and for the query time u set neopixel to be certain colour
    //Check now used to just do a bat check during startup, from then on its false;
    
    bat_reading = map(analogRead(ANALOG_BAT),0,4096,0,33) *2; //0 - 33 ( 0 - 3.3 since map is int)
    bat_voltage = bat_reading / 10.0;

    if(bat_voltage <= 3.5){
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

//Magdwick 
Madgwick filter;
#define SAMPLE_FREQ 10
const float GRAVITY = 9.81;
//const float RAD_TO_DEG = 180/3.142; //Commented out i think its already defined 

const uint16_t IMU_TIME_DELAY_MILLIS = 1.0/SAMPLE_FREQ * 1000;
float ax, ay, az;
float gx, gy, gz;
float roll, pitch;

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
  
  filter.updateIMU(gx, gy, gz, ax, ay, az);  
}


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
  if (!lc.begin()) {
    Serial.println(F("Couldnt find Adafruit LC709203F?\nMake sure a battery is plugged in!"));
    delay(1000);  resetFunc();  //call reset if unable to find item to reinit
  }
  lc.setPackSize(LC709203F_APA_1000MAH);  //Acutal pack size is 1500 but we just go under
  
  onboard_pixel.begin();
  onboard_pixel.setBrightness(BRIGHTNESS);

  CheckBattery(true); //Just check for the first time and ignore the timing

  go_to_sleep = false;
  Setup_DEEPSLEEP();
  
  SetUpESPNOW();
  Serial.println("ESP-NOW setup");
  WiFi.macAddress(message.mac_address);

  delay(1000); //End setup

}

void loop() {
  // put your main code here, to run repeatedly:

  if( (millis() - imu_read_time) >= IMU_TIME_DELAY_MILLIS){

    get_readings();
    
    roll = filter.getRoll();
    pitch = filter.getPitch();

    
    Serial.println(roll);

    /*
    Serial.print(" ");
    Serial.print(pitch);
    Serial.println("");
    */

    imu_read_time = millis(); //reset to time last read
    
    message.angle = roll; //Lets stick with roll
    esp_now_send(master, (uint8_t *) &message, sizeof(message));


    //BLink each time sample is takken
    digitalWrite(LED_BUILTIN,led_on);
    led_on = !led_on;
    
  }

  CheckBattery(false);
  CheckSleep();


}
