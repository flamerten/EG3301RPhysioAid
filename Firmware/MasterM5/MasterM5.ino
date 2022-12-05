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
esp_now_peer_info_t peerInfo;

int PatientAngles[] = {0,0,0};
//bool Turned_On[] = {false,false,false}; //flashing lights to indicate data being recieved
int Time_RCV[] = {-10000,-10000,-10000}; //Use this to check the time last recieved

String RPI_msg;
long Angle_msg; //data type of toInt is long

bool pixel_light_on = false;
const unsigned int LAST_TIME_MAX = 10 * 1000; //10sec: After 10s assume disconnected

Adafruit_NeoPixel pixel(25, 27, NEO_GRB + NEO_KHZ800);
#define BRIGHTNESS 20

//Sending
#define SENDING_FREQ 10
const int TIME_DELAY_MILLIS = 1.0/SENDING_FREQ * 1000;
unsigned long int serial_send_time = millis();

//BUTTON
#define BUTTON 39
bool button_press = false;
void IRAM_ATTR PRESS_ISR(){
  //GPIO interrupt that sends out message to vibrate
  button_press = true;  
}

//Vibrate
uint8_t SlaveAddress[3][6] = { //Feather Slave addresses, to trasnsmit data to them and tell them to vibrate
    {0xE8,0x9F,0x6D,0x28,0x0C,0x98}, //Slave 0
    {0xE8,0x9F,0x6D,0x26,0x09,0x20}, //Slave 1 (testing unit)
    {0xE8,0x9F,0x6D,0x25,0x46,0x70}, //Slave 2 newest
};


void Setup_Interrupt(){ //Button press interrupt
    pinMode(BUTTON,INPUT_PULLUP);
    attachInterrupt(BUTTON, PRESS_ISR, FALLING);
}

void SetUpESPNOW(){
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);
    
    peerInfo.channel = 0;
    peerInfo.encrypt = false; //Do this part once

    for(int i = 0; i < 3; i++){
        memcpy(peerInfo.peer_addr,SlaveAddress[i],6);
        if(esp_now_add_peer(&peerInfo)!= ESP_OK){
            Serial.print("Failed to add peer ");
            Serial.println(i);
        }
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
    
    PatientAngles[message.patient_no] = message.angle; //Update Storage,cast to int
    Time_RCV[message.patient_no] = millis(); //Update timing last recieved
    
    /*
    if(Turned_On[message.patient_no]){
        pixel.setPixelColor(message.patient_no,pixel.Color(0,0,0));
    }
    else{
        pixel.setPixelColor(message.patient_no,pixel.Color(0,100,0));
    }

    Turned_On[message.patient_no] = !Turned_On[message.patient_no];
    */

    pixel.show();

}

void CheckTimings(){
  for( int i = 0; i < 3; i++){
    if(millis() - Time_RCV[i] > LAST_TIME_MAX){
      PatientAngles[i] = -1; //Change to neg 1 to show issues with recieving data
      pixel.setPixelColor(i,pixel.Color(0,0,0));
    }
    else pixel.setPixelColor(i,pixel.Color(0,100,0));
    }

}


void setup(){
    Serial.begin(115200);

    pixel.begin();
    pixel.setBrightness(BRIGHTNESS);
    SetUpESPNOW();

    pinMode(BUTTON,INPUT_PULLUP);
    Setup_Interrupt();    

    Serial.println("Setup Done");
    delay(1000);

    

}

void loop(){
    if(millis() - serial_send_time >= TIME_DELAY_MILLIS){
        Serial.printf(
            "x %i %i %i x", //Use of "x" for the RPI to check the data recieved was cut short or corrupted
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

    if(Serial.available()){//Check if data is available
        RPI_msg = Serial.readString();
        RPI_msg.trim(); //remove any \r \n white space


        Angle_msg = RPI_msg.toInt(); //convert string to int > patient no that should vibrate

        if((Angle_msg >=0) && (Angle_msg <=2)){
            esp_now_send(SlaveAddress[Angle_msg], (uint8_t *) &message, sizeof(message)); //Tell specific slave to vibrate
        }

        //Serial.println("SEND");
        
    }

    CheckTimings();

    
    //For initial testing
    if(button_press){ //Vibrate all 3 slaves > Purely for testing and debugging
        button_press = false;
        Serial.println("Sending Vibrate");
        esp_now_send(SlaveAddress[0], (uint8_t *) &message, sizeof(message));
        esp_now_send(SlaveAddress[1], (uint8_t *) &message, sizeof(message));
        esp_now_send(SlaveAddress[2], (uint8_t *) &message, sizeof(message));
        delay(50);
    }


    
    
}
