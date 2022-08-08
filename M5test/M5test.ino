//Board is M5 Atom Stack
//Referenced from https://docs.m5stack.com/en/api/atom/mpu
//Orientation Filter based on https://courses.cs.washington.edu/courses/cse466/14au/labs/l4/madgwick_internal_report.pdf


#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs
#include <Wire.h> //I2c for the MPU
#include "M5Atom.h" //helper functions for the MPU

float pitch, roll, yaw; //Stores attitude related variables.

#define READ_DELAY  100 //magdwick filter is effective at low sampling rates
float cal_pitch, cal_roll; //calibration during init;

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
void calibrate_IMU(){
  Serial.println("Calibrating");
  light_middle(100,0,0);//light up red during calibration
  cal_pitch = 0;
  cal_roll = 0;

  float max_pitch, max_roll;
  float min_pitch, min_roll;

  max_pitch = max_roll = -200;
  min_pitch = min_roll = 200;

  int num_iter = 50;

  for(int i = 0; i < num_iter; i++){
    M5.IMU.getAhrsData(&pitch,&roll,&yaw);
    
    if(pitch > max_pitch) max_pitch = pitch;
    if(pitch < min_pitch) min_pitch = pitch;
    if(roll > max_roll) max_roll = roll;
    if(roll < min_roll) min_roll = roll;
    
    cal_pitch += pitch;
    cal_roll += roll;
    delay(READ_DELAY);
  }

  cal_pitch = cal_pitch/num_iter;
  cal_roll = cal_roll/num_iter;


  light_middle(0,100,0); //light up green when finished
  Serial.printf("cal_pitch:%.2f, cal_roll:%.2f", cal_pitch,cal_roll);
  Serial.println();
  Serial.printf(
    "PITCH Variation:%.2f , Max_pitch:%.2f, Min_Pitch: %2f",
    max_pitch - min_pitch,
    max_pitch,
    min_pitch
    );
    
  Serial.println();
  Serial.printf(
    "ROLL Variation:%.2f , Max_roll:%.2f, Min_roll: %2f",
    max_roll - min_roll,
    max_roll,
    min_roll
    );

   Serial.println();
   
}


void calibrate_IMU_MAX(){
  Serial.println("Calibrating");
  light_middle(100,0,0);//light up red during calibration
  cal_pitch = 0;
  cal_roll = 0;


  light_middle(0,100,0); //light up green when finished
  
}

void setup() {
  //Serial Enable baud rate is 115200
  M5.begin(true, true, false); //bool SerialEnable, bool I2CEnable, bool DisplayEnable
  init_matrix();
  M5.IMU.Init();

  //calibrate_IMU();

}

void loop() {
  
  M5.IMU.getAhrsData(&pitch,&roll,&yaw);
  Serial.printf("ORGINAL pitch:%.2f, roll:%.2f", pitch,roll);
  Serial.println();
  pitch = pitch - cal_pitch;
  roll = roll - cal_roll;
  Serial.printf("CALIBRATED pitch:%.2f, roll:%.2f", pitch,roll);
  Serial.println();

  //For The serial plotter printout
  /*Serial.print(pitch);
  Serial.print(" ");
  Serial.print(roll);
  Serial.println();*/
  
  delay(READ_DELAY);

}
