/*
Slave Sketch
This M5 recieves the esp-now message from the master and then  begins sampling
Sampling rate defined from the messgae recieved
Use MacAddress[5] as SlaveNo > During testing shown to be diff
*/

#include "M5Atom.h" //helper functions for the MPU
#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <esp_now.h>
#include <WiFi.h>

//On board LED matrix
#define LED_COUNT 25
#define LED_PIN   27
#define BRIGHTNESS 100 //set as 0 if you do not want to show the indicator
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);

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

//ESP-NOW Communication
uint8_t master[] = {0x4C,0x75,0x25,0xC4,0xFA,0xF4}; //GREY
bool data_rcv = false;
int sampling_period = 1000000; //Changed on data rcv
int delaylag; //Add a slight delay for one of the slaves depending on its mac add

#define DEFAULT_DELAY_LAG 50


//Use the same float for all 3 devices
//Reduce complexity(?)
typedef struct struct_message{
    int sampling_rate;
    int slave_no;
    float roll_angle;

} struct_message; //define struct and then initalise it
//Declare message to be sent over, and the slave info
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

uint8_t calc_slave_no(){
    //Slave no is the last number of mac address that is found to be constant
    //The better method will generate too big a number

    uint8_t mac[6]; //actually 8 bit but for later it is more conv
    WiFi.macAddress(mac); 

    return mac[5];

}

void setup(){
    M5.begin();
    Serial.begin(115200);
    init_matrix();
    light_middle(100,0,0);

    delay(100);

    SetUpESPNOW();
    Serial.println("ESP-NOW setup");

    //Set Parameters not too sure why i can't do it before setup
    uint8_t mac[6];
    WiFi.macAddress(mac); //fill mac with the mac address
    

    while(!data_rcv) delay(1); //Only start when message is recieved

    Serial.println("Data Recieved");

    //Set message variables to slave_no AFTER message recieved
    message.sampling_rate = -1;
    message.roll_angle = -1;
    message.slave_no =  calc_slave_no(); //fill in with last value
    if(message.slave_no == 32) delaylag = 0;
    else delaylag = DEFAULT_DELAY_LAG;

    delay(delaylag); //at the start of the setup, give some lag time

}

void loop(){
    light_middle(0,100,0);
    esp_now_send(master, (uint8_t *) &message, sizeof(message));
    delay(sampling_period);
    light_middle(0,0,0);
    esp_now_send(master, (uint8_t *) &message, sizeof(message));
    delay(sampling_period);
}
