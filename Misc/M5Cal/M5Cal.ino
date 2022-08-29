//Board is M5 Atom Stack
//IMU code referenced from https://docs.m5stack.com/en/api/atom/mpu
//Orientation Filter based on https://courses.cs.washington.edu/courses/cse466/14au/labs/l4/madgwick_internal_report.pdf
//Code based on M5 original code
//Use this for calibration testing

#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <Wire.h> //I2c for the MPU
#include "M5Atom.h" //helper functions for the MPU

float pitch, roll, yaw; //Stores attitude related variables.
float ax,ay,az,gx,gy,gz; //acceleration and gyro values
#define READ_DELAY  50 //magdwick filter is effective at low sampling rates


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

void setup(){
    //Serial Enable baud rate is 115200
    M5.begin(true, true, false); //bool SerialEnable, bool I2CEnable, bool DisplayEnable
    init_matrix();
    M5.IMU.Init();

    delay(1000);

}

void loop(){

}
