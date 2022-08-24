/*
Serial Read Sketch
This M5 sends a message to the other M5s and tells them to start sampling
Then this M5 recieves the 2 angles and calculates the joint angle

SLAVE NO
red > 32
yellow > 192

*/

#include "M5Atom.h" //helper functions for the MPU
#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <esp_now.h>
#include <WiFi.h>


#define SAMPLING_PERIOD  1000 //increase period for better visibility

//On board LED matrix
#define LED_COUNT 25
#define LED_PIN   27
#define BRIGHTNESS 50 //set as 0 if you do not want to show the indicator
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);

void init_matrix(){
  //ensure the matrix behind is turned off, reduce power usage
  //however, this can be useeful as an indicator
  matrix.begin();
  matrix.clear();
  matrix.show();
  matrix.setBrightness(BRIGHTNESS);

}

void light_up_n(int r, int g, int b, int index = 12){
  //12 is the centre of the matrix > 12 if not specified
  matrix.setPixelColor(index,matrix.Color(r,g,b));
  matrix.show();
}

//Use indicators to change colour upon recieving angle data
//red blue green off > cycle
//diff pos for diff slaves
uint8_t colour_pos1 = 0;
uint8_t colour_pos2 = 0;

void lightup(int pos, int index){
    switch(pos){
        case(0):
            light_up_n(100,0,0, index);
            break;
        case(1):
            light_up_n(0,100,0,index);
            break;
        case(2):
            light_up_n(0,0,100,index);
            break;
        case(3):
            light_up_n(0,0,0,index);
            break; 
    }
}

//ESP-NOW
uint8_t slave1[] = {0x4C,0x75,0x25,0xC5,0x02,0x20};   //RED
uint8_t slave2[] = {0x4C,0x75,0x25,0xC4,0xFA,0xC0};  //YELLOW

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
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    //Call Func OnDataSent when data is sent > esp_now_send
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    //Add slave 1
    memcpy(peerInfo.peer_addr, slave1, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add slave1");
        return;
    }

    //Add Slave 2
    memcpy(peerInfo.peer_addr, slave2, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer2");
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
    memcpy(&message, incomingData, sizeof(message)); //Save data to struct?
    if(message.slave_no == 32){
        if(colour_pos1 >= 3) colour_pos1 = 0;
        else colour_pos1++; 
    }
    else if(message.slave_no == 192) {
        if(colour_pos2 >= 3) colour_pos2 = 0;
        else colour_pos2++; 
    }

    Serial.print("Message Recieved from Slave");
    Serial.println(message.slave_no);

    // make use of interupt to change colour pos for both slaves
    
}


void start_sampling(){
    //Send sampling period over to other M5s
    //Rest of data set to be -1, as they are invalid
    message.sampling_rate = SAMPLING_PERIOD;
    message.slave_no = -1;
    message.roll_angle = -1;

    //Send same message to both M5s
    esp_err_t result1 = esp_now_send(slave1, (uint8_t *) &message, sizeof(message));
    esp_err_t result2 = esp_now_send(slave2, (uint8_t *) &message, sizeof(message));

    if(result1 == ESP_OK)   Serial.println("Slave 1 sent");
    else                    Serial.println("Slave 1 Send Fail");

    if(result2 == ESP_OK)   Serial.println("Slave 2 sent");
    else                    Serial.println("Slave 2 Send Fail");


}

void setup(){
    Serial.begin(115200);
    init_matrix();
    light_up_n(100,100,100);

    SetUpESPNOW();
    Serial.println("M5 Serial Receive Set up");
    delay(1000);

    start_sampling();
    Serial.println("Sampling Message Sent");

    light_up_n(0,0,0);
    //Sdelay(1000);


}


void loop(){

    lightup(colour_pos1,12); //slave 32 red
    lightup(colour_pos2,0); //slave 192 yellow

}
