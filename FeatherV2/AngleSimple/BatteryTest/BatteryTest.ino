
#include <Adafruit_LC709203F.h>
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_I2C_POWER 2

Adafruit_LC709203F lc;
#define BRIGHTNESS 50
#define ANALOG_BAT 35 //Analog built in to measure bat
int bat_reading;
int bat_percentage;
float bat_voltage;

Adafruit_NeoPixel onboard_pixel(1, 0, NEO_GRB + NEO_KHZ800);

void CheckBattery(bool check_now){
    //Check for low bat and for the query time u set neopixel to be certain colour
    //Check now used to just do a bat check during startup, from then on its false;
    
    bat_reading = map(analogRead(ANALOG_BAT),0,4096,0,33) * 2;
    //0 - 33 ( 0 - 3.3 since map is int)
    //*2
    
    bat_voltage = bat_reading / 10.0;

    Serial.printf("The Battery Voltage is %f",bat_voltage);
    Serial.println();

        
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


void setup(){
    Serial.begin(115200);
    Wire.begin(22,20);

    pinMode(ANALOG_BAT,INPUT);
    pinMode(LED_BUILTIN,OUTPUT);

    pinMode(NEOPIXEL_I2C_POWER,OUTPUT);
    digitalWrite(NEOPIXEL_I2C_POWER,HIGH);
    delay(100); //give time for i2c to init

    if (!lc.begin()) {
        Serial.println(F("Couldnt find Adafruit LC709203F?\nMake sure a battery is plugged in!"));
        delay(1000);  
    }
    lc.setPackSize(LC709203F_APA_1000MAH);  //Acutal pack size is 1500 but we just go under

    onboard_pixel.begin();
    onboard_pixel.setBrightness(BRIGHTNESS);

    CheckBattery(true); 

}

void loop(){

}
