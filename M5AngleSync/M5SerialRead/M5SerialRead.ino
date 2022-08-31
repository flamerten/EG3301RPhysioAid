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

//On board LED matrix
#define LED_COUNT 25
#define LED_PIN   27
#define BRIGHTNESS 50 //set as 0 if you do not want to show the indicator
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);

//Use indicators to change colour upon recieving angle data, diff pos for diff slaves
uint8_t colour_pos1 = 0;
uint8_t colour_pos2 = 0;


//ESP-NOW
//uint8_t slave1[] = {0x4C,0x75,0x25,0xC5,0x02,0x20};   //RED
//uint8_t slave2[] = {0x4C,0x75,0x25,0xC4,0xFA,0xC0};  //YELLOW
uint8_t slave1[6];
uint8_t slave2[6];
bool Mac1Unknown = true;
bool Mac2Unknown = true;

//Use the same float for all 3 devices
//Reduce complexity(?)
typedef struct struct_message{
    int sampling_rate;
    uint8_t slave_mac[6];
    float roll_angle;

} struct_message; //define struct and then initalise it

//Declare message to be sent over, and the slave info
struct_message message;
esp_now_peer_info_t peerInfo;

//IMU
#define SAMPLING_PERIOD  50 //increase period for better visibility
float rolls_rcv[] = {0.0,0.0}; //ANGLE Recieved

void SetUpESPNOW_RCV(){
    //Has to know the mac address of the knees. So once recieve it saves the mac address
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    //Call Func OnDataSent when data is sent > esp_now_send
    
    esp_now_register_recv_cb(OnDataRecv);


}

void SetUpESPNOW_SND(){
    //Assumes that the recv function has been activated
    esp_now_register_send_cb(OnDataSent);
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
void PrintMac(uint8_t * mac){
    for(int i = 0; i < 6; i ++){
        Serial.print(mac[i]);
        Serial.print(":");
    }

    Serial.println();
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&message, incomingData, sizeof(message)); //Save data to struct?
    if(Mac1Unknown){
        memcpy(slave1,message.slave_mac,6);
        Mac1Unknown = false;
        //Serial.print("Mac 1 saved. Mac address is ");
        //PrintMac(slave1);
        //Serial.print(Mac1Unknown);
    }
    else if(Mac2Unknown){
        memcpy(slave2,message.slave_mac,6);
        if(IsMacSame(slave2,slave1)){
          return;
        }
        Mac2Unknown = false; 
        //Serial.print("Mac 2 saved. Mac address is ");
        //PrintMac(slave2);     
        //Serial.print(Mac2Unknown);
    }
    else{
        //Slave 1 is saved to rolls rcv 1
        if(IsMacSame(message.slave_mac,slave1)){
            if(colour_pos1 >= 3) colour_pos1 = 0;
            else colour_pos1++;
            rolls_rcv[0] = message.roll_angle; 

        }
        else if(IsMacSame(message.slave_mac,slave2)) {
            if(colour_pos2 >= 3) colour_pos2 = 0;
            else colour_pos2++; 
            rolls_rcv[1] = message.roll_angle; 
        }
        else Serial.println("Unknown MAC recieved");
    }

    //Serial.print("Message Recieved from Slave");
    //Serial.println(message.slave_no);

    // make use of interupt to change colour pos for both slaves
    
}

bool IsMacSame(uint8_t * mac1, uint8_t * mac2){
    for( int i = 0; i < 6; i++){
        if(mac1[i] != mac2[i]){
            return false;
        }
    }

    return true;
}

void start_sampling(){
    //Send sampling period over to other M5s
    //Rest of data set to be -1, as they are invalid
    message.sampling_rate = SAMPLING_PERIOD;
    
    //memcpy(message.slave_mac,slave2,6); //dummy variable so it is defined
    //message.slave_mac = 1;
    

    //Send same message to both M5s
    message.roll_angle = 0; //Use roll angle to get the ESPNOW lag > 1 = lag
    esp_err_t result1 = esp_now_send(slave1, (uint8_t *) &message, sizeof(message));

    message.roll_angle = 1;
    esp_err_t result2 = esp_now_send(slave2, (uint8_t *) &message, sizeof(message));

    if(result1 == ESP_OK)   Serial.println("Slave 1 sent");
    else                    Serial.println("Slave 1 Sent Fail");

    if(result2 == ESP_OK)   Serial.println("Slave 2 sent");
    else                    Serial.println("Slave 2 Send Fail");


}

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

void setup(){
    Serial.begin(115200);
    init_matrix();
    light_up_n(100,100,100);

    SetUpESPNOW_RCV();
    Serial.println("M5 Serial Receive Set up");
    delay(1000);

    while( (Mac1Unknown == true) || (Mac2Unknown == true) ){
        int r = 100;
        int g = 100;
        int b = 100;

        light_up_n(100,100,100);
        delay(1000);
        Serial.println("Listening for mac address");
        light_up_n(0,0,0);
        delay(1000);

        if(!Mac1Unknown){
          light_up_n(100,100,100,0);
        }
      
    }
    //Only continue when the mac addresses has been recieved

    SetUpESPNOW_SND(); //add peers once they have been saved

    start_sampling();
    Serial.println("Sampling Message Sent");

    light_up_n(0,0,0);
    //Sdelay(1000);


}

int angle_knee;

void loop(){

    lightup(colour_pos1,12);
    lightup(colour_pos2,0);

    Serial.print(rolls_rcv[0]); //-180 to 180
    Serial.print(" ");
    Serial.print(rolls_rcv[1]);
    Serial.print(" ");
    
    angle_knee = int(abs(rolls_rcv[0] - rolls_rcv[1]) );
    if(angle_knee > 180) angle_knee = 360 - angle_knee;
    
    Serial.println(angle_knee);   // use 180 as reference point

    delay(1); //slow together

}
