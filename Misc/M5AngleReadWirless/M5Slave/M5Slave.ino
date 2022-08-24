#include "M5Atom.h" //helper functions for the MPU
#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <esp_now.h>
#include <WiFi.h>

uint8_t master[] = {0x4C,0x75,0x25,0xC4,0xFA,0xF4}; //GREY

//Use the same float for all 3 devices, Reduce complexity(?)
typedef struct struct_message{
    float pitch_angle;
    float roll_angle;

} struct_message; //define struct and then initalise it

struct_message message;
esp_now_peer_info_t peerInfo;

float pitch, roll, yaw;
unsigned long time_now = millis();
int sampling_period = 100; //Changed on data rcv



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

    Serial.println("ESPNOW setup");

}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

    //Mostly debugging messages > don't do anything

    //Serial.print("\r\nLast Packet Send Status:\t");
    //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    
    return;
}

void setup(){
    M5.begin();
    Serial.begin(115200);
    M5.IMU.Init();  
    SetUpESPNOW();  


}

void loop(){
    if(millis() - time_now >= sampling_period){
        

        //IMU calc and send
        M5.IMU.getAhrsData(&pitch,&roll,&yaw);

        message.roll_angle = roll;
        message.pitch_angle = pitch;

        esp_now_send(master, (uint8_t *) &message, sizeof(message));
        
        Serial.print(roll);
        Serial.print(" ");
        Serial.println(pitch);
        //Reset variable

        time_now = millis(); // reset time for next sample


    }

}
