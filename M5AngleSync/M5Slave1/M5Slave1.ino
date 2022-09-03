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
#define BRIGHTNESS 50 //set as 0 if you do not want to show the indicator
bool led_off = true;
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);

//CPU CLOCK
#define CLOCKSPEED 80
//function takes the following frequencies as valid values:
//  240(Def), 160, 80    <<< For all XTAL types
//  40, 20, 10      <<< For 40MHz XTAL

//Deep sleep button
#define BUTTON 39
#define BUTTON_PIN_BITMASK 0x8000000000 // 2^39 in hex
volatile bool go_to_sleep = false; //For the ISR

//BatteryAnalog
#define BATTERY 33
int BatLastChecked;
int BatNumber;
const int BatCheckInterval = 5*60*1000; //5mins

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

void light_up(int r, int g, int b, int index){
  //12 is the centre of the matrix
  matrix.setPixelColor(index,matrix.Color(r,g,b));
  
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
    CheckSleep();
    M5.IMU.getAhrsData(&pitch,&roll,&yaw);
    
    angle_storage[curr_index] = roll;

    if(arr_is_valid(difference, angle_storage) && (millis() - time_now > CALIBRATE_LAG) ){
      //check that the angle has stabalised
      //let the callibration time be that number to prevent early activation

      for(int i = 0; i  < compare_no; i++){
        cal_roll += angle_storage[i];
      }

      cal_roll = cal_roll / compare_no;

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

void IRAM_ATTR SLEEP_ISR(){
  go_to_sleep = true;  
}

void Setup_DEEPSLEEP(){
  pinMode(BUTTON,INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39,LOW); //Wake up when button pulled low
  attachInterrupt(BUTTON, SLEEP_ISR, FALLING); //Interrupt and go to sleep when button pulled low

}

int BatAmt(){
  int number = analogRead(BATTERY);
  int pd_out_map = map(number,0,4096,0,330); //Number reads from 0 to 4096 (12bit)
  float voltage = pd_out_map / 100.0 * 2.0;

  if(voltage > 3.9) return 3;
  else if(voltage > 3.0) return 2; //by right 3.2V for low battery but i suspect is not very accurate so i minus 0.2 form the actual value
  else return 1;

}

void DisplayBat(int number){
  matrix.clear();
  ShowBattery();

  switch(number){
    case 3: //green 3 bar
      light_up(0,100,0,11); light_up(0,100,0,12); light_up(0,100,0,13);
      break;
    case 2:
      light_up(0,100,0,11); light_up(0,100,0,12); light_up(0,0,0,13);
      break;
    case 1:
      light_up(100,0,0,11); light_up(0,0,0,12); light_up(0,0,0,13);
      break;
    case 0: //Clear the Middle, leave battery box
      light_up(0,0,0,11); light_up(0,0,0,12); light_up(0,0,0,13);
      break;
    case -1: //White during connection
      for(int i = 5; i < 20; i++){
        light_up(50,50,50,i);
      }
      break;
  }

  matrix.show();
}

void ClearMatrix(){
  matrix.clear();
  matrix.show();  
}

void ShowBattery(){
  for(int i = 5; i < 20; i++){
    light_up(50,50,50,i);
  }
}

void CheckSleep(){
  if(go_to_sleep){
    ClearMatrix(); //Turn off indicator and go to sleep
    Serial.println("Going to sleep");
    delay(1000);
    esp_deep_sleep_start();
  }  
}

void setup(){
  M5.begin();
  M5.IMU.Init();
  setCpuFrequencyMhz(CLOCKSPEED);
  
  Serial.begin(115200);
  delay(100);
  
  go_to_sleep = false;
  pinMode(BATTERY,INPUT);

  init_matrix();
  BatNumber = BatAmt();
  BatLastChecked = millis();

  Setup_DEEPSLEEP();
  
  SetUpESPNOW();
  Serial.println("ESP-NOW setup");

  //Set Parameters not too sure why i can't do it before setup
  //uint8_t mac[6];
  WiFi.macAddress(message.slave_mac); // Copy the current mac address to mac 
  Serial.println(WiFi.macAddress()); 
  //memcpy(message.slave_mac,mac,6); //6 Values in the mac address so byte sioze of 6 
  go_to_sleep = false;
  Serial.print("But Value: ");
  Serial.println(digitalRead(BUTTON));
  Serial.print("go_to_sleep: ");
  Serial.println(go_to_sleep);  

  DisplayBat(BatNumber);
  
  calibrate_IMU_MAX();

  bool off = false;
  while(!data_rcv){
    CheckSleep();
    //Only start when message is recieved, send one pulse every second
    esp_now_send(master, (uint8_t *) &message, sizeof(message));
    if(off){
      DisplayBat(-1);
      off = !off;
    }
    else{
      ClearMatrix();
      off = !off;
    }
    delay(1000); 
    //Serial.println("Pulse sent out");
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
    if(led_off) DisplayBat(BatNumber);
    else DisplayBat(0);

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

  if(millis() - BatLastChecked >= BatCheckInterval){
    BatNumber = BatAmt();
    BatLastChecked = millis(); //Reset interval
  }


  CheckSleep();
}
