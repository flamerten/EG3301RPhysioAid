/*
 * This sketch is the MasterM5 which recieves data over ESP-NOW and outputs it to 
 * the serial terminal. At least 2 wearables will be transmitting data over
 *
 */

#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message{
    float angle;
    int patient_no;
} struct_message; //define struct and then initalise it

struct_message message;
esp_now_peer_info_t peerInfo; // Think i dont need this

int PatientAngles[] = {0,0,0}; //Might need a way to indicate the last time data was recieved
bool Turned_On[] = {false,false,false};
int Time_RCV[] = {0,0,0};

bool pixel_light_on = false;
const unsigned int LAST_TIME_MAX = 10 * 1000; //30sec

Adafruit_NeoPixel pixel(25, 27, NEO_GRB + NEO_KHZ800);
#define BRIGHTNESS 20

//Sending
#define SENDING_FREQ 10
const int TIME_DELAY_MILLIS = 1.0/SENDING_FREQ * 1000;
unsigned long int serial_send_time = millis();

void SetUpESPNOW(){
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
    
    peerInfo.channel = 0;
    peerInfo.encrypt = false;


}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    //Data is only recieved once to init sampling
    
    memcpy(&message, incomingData, sizeof(message)); //Save data to struct
    
    PatientAngles[message.patient_no] = message.angle; //Update Storage,cast to int
    Time_RCV[message.patient_no] = millis(); //Update timing last recieved
    

    if(Turned_On[message.patient_no]){
        pixel.setPixelColor(message.patient_no,pixel.Color(0,0,0));
    }
    else{
        pixel.setPixelColor(message.patient_no,pixel.Color(0,100,0));
    }

    Turned_On[message.patient_no] = !Turned_On[message.patient_no];

    pixel.show();

}

void CheckTimings(){
  for( int i = 0; i < 3; i++){
    if(millis() - Time_RCV[i] > LAST_TIME_MAX){
      PatientAngles[i] = -1; //Change to neg 1 to show issues with recieving data
    }
  }
}


void setup(){
    Serial.begin(115200);

    pixel.begin();
    pixel.setBrightness(BRIGHTNESS);
    SetUpESPNOW();

    delay(1000);

    Serial.println("Setup Done");

}

void loop(){
    if(millis() - serial_send_time >= TIME_DELAY_MILLIS){
        Serial.printf(
            "x %i %i %i x",
            PatientAngles[0],
            PatientAngles[1],
            PatientAngles[2]);

        serial_send_time = millis(); //reset flag;
        Serial.println();

        if(pixel_light_on) pixel.setPixelColor(24,pixel.Color(100,100,100));
        else pixel.setPixelColor(24,pixel.Color(0,0,0));

        pixel_light_on = !pixel_light_on;

        pixel.show();
    }

    CheckTimings();
    
}
