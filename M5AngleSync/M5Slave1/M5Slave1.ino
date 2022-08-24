/*
Slave Sketch
This M5 recieves the esp-now message from the master and then  begins sampling
Sampling rate defined from the messgae recieved
*/

#include "M5Atom.h" //helper functions for the MPU
#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <esp_now.h>
#include <WiFi.h>

//On board LED matrix
#define LED_COUNT 25
#define LED_PIN   27
#define BRIGHTNESS 100 //set as 0 if you do not want to show the indicator
bool led_off = true;
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);


//ESP-NOW Communication
#define DEFAULT_DELAY_LAG 50 //communication delay
#define CALIBRATE_LAG 5000 //Let the IMU do gradient descent for N ms at least
uint8_t master[] = {0x4C,0x75,0x25,0xC4,0xFA,0xF4}; //GREY
bool data_rcv = false;
int sampling_period = 10000000; //Changed on data rcv > master sends the sampling period he wants
int delaylag;

//Use the same float for all 3 devices, Reduce complexity(?)
typedef struct struct_message{
    int sampling_rate;
    uint8_t slave_mac[6];
    float roll_angle;

} struct_message; //define struct and then initalise it

struct_message message;
esp_now_peer_info_t peerInfo;

//IMU
#define CALIBRATION_LAG 100
float pitch, roll, yaw;
float cal_roll;
unsigned long time_now = millis();
const uint8_t compare_no = 20; //compare n different readings during calibration
const uint8_t difference = 1; //1 degrees diff between readings = stabalise


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
    data_rcv = true; // Flag to start sampling
    sampling_period = message.sampling_rate;

}

void init_matrix(){
  //ensure the matrix behind is turned off, reduce power usage
  //however, this can be useeful as an indicator
  matrix.begin();
  matrix.clear();
  matrix.show();
  matrix.setBrightness(BRIGHTNESS);

}

void light_middle(int r, int g, int b){
  //12 is the centre of the matrix
  matrix.setPixelColor(12,matrix.Color(r,g,b));
  matrix.show();
}

bool arr_is_valid(float diff, float *arr ){
  //check if the diff between max and min is whithin the diff, for callibration 
  
  float max_value = -360.0;
  float min_value = 360.0;
  for(int i = 0; i < compare_no; i++){
    if(arr[i] > max_value) max_value = arr[i];
    if(arr[i] < min_value) min_value = arr[i];
  }

  return abs(max_value - min_value) <= diff;
}

void calibrate_IMU_MAX(){
  //It appears that this is gradient descent. 
  //As such i will wait for the readings to be stabalised by comparing the gradients
  
  Serial.println("Calibrating");
  
  float angle_storage[compare_no];

  int curr_index = 0;
  int count = 0;
  int time_now = millis();

  cal_roll = 0;

  while(1){
    M5.IMU.getAhrsData(&pitch,&roll,&yaw);
    
    angle_storage[curr_index] = roll;

    if(arr_is_valid(difference, angle_storage) && (millis() - time_now > CALIBRATE_LAG) ){
      //check that the angle has stabalised
      //let the callibration time be that number to prevent early activation

      for(int i = 0; i  < compare_no; i++){
        cal_roll += angle_storage[i];
      }

      cal_roll = cal_roll / compare_no;

      light_middle(0,100,0); //light up green when finished
      Serial.println("Calibration Done");
      Serial.printf("Offset roll:%.2f", cal_roll);
      Serial.println();

      return;
    }
    else{
      if(curr_index >= compare_no - 1) curr_index = 0;
      else curr_index++;
      
      count++;
      if(count%20 == 0) Serial.println(count);
    }

    delay(CALIBRATION_LAG);
  }
  
}



void setup(){
    M5.begin();
    Serial.begin(115200);
    M5.IMU.Init();
    init_matrix();
    light_middle(100,0,0);

    delay(3000);

    SetUpESPNOW();
    Serial.println("ESP-NOW setup");

    //Set Parameters not too sure why i can't do it before setup
    //uint8_t mac[6];
    WiFi.macAddress(message.slave_mac); // Copy the current mac address to mac 
    Serial.println(WiFi.macAddress()); 
    //memcpy(message.slave_mac,mac,6); //6 Values in the mac address so byte sioze of 6 

    light_middle(0,0,100); //Set blue when callibration starts
    calibrate_IMU_MAX();
    light_middle(0,0,0); //Turn off when done
    
    bool off = true;
    while(!data_rcv){
      esp_now_send(master, (uint8_t *) &message, sizeof(message));
      if(off){
        light_middle(0,0,100);
        off = !off;
      }
      else{
        light_middle(0,0,0);
        off = !off;
      }
      delay(1000); //Only start when message is recieved, send one pulse every second
      Serial.println("Pulse sent out");
    }

    Serial.println("Data Recieved");

    //Set message variables to slave_no AFTER message recieved

    

    if(message.roll_angle == 1) delaylag = 0;
    else delaylag = DEFAULT_DELAY_LAG;

    message.roll_angle = -1; //reset roll angle
    WiFi.macAddress(message.slave_mac); //reset mac slave number instead of the one recieved
    
    delay(delaylag); //at the start of the setup, give some lag time
    time_now = millis();

}

void loop(){
  //Serial.println(millis() - time_now);
    if(millis() - time_now >= sampling_period){
        
        //LED indicator
        if(led_off) light_middle(0,100,0);
        else light_middle(0,0,0);

        //IMU calc and send
        M5.IMU.getAhrsData(&pitch,&roll,&yaw);
        Serial.print("Pitch: ");
        Serial.println(roll);
        Serial.print("Callibrated Ptich: ");
        Serial.println(roll - cal_roll);
        message.roll_angle = roll - cal_roll;
        esp_now_send(master, (uint8_t *) &message, sizeof(message));
        
        //Reset variable
        time_now = millis();
        led_off = !led_off; 

    }

}
