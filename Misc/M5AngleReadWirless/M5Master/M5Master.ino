#include "M5Atom.h" //helper functions for the MPU
#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <esp_now.h>
#include <WiFi.h>

//Use the same float for all 3 devices, Reduce complexity(?)
typedef struct struct_message{
    float pitch_angle;
    float roll_angle;

} struct_message; //define struct and then initalise it

struct_message message;


float angle_recieved[] = {0.0,0.0}; //ANGLE Recieved

void SetUpESPNOW(){
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    //Call Func OnDataSent when data is sent > esp_now_send

    esp_now_register_recv_cb(OnDataRecv);
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&message, incomingData, sizeof(message)); //Save data to struct?
    angle_recieved[0] = message.roll_angle;
    angle_recieved[1] = message.pitch_angle;

    
}

void setup(){

    Serial.begin(115200);
    SetUpESPNOW();
    Serial.println("M5 Serial Receive Set up");
    delay(1000);

}

void loop(){
    Serial.print("Pitch: ");
    Serial.print(angle_recieved[0]);
    Serial.print("  Roll: ");
    Serial.println(angle_recieved[1]);
}
