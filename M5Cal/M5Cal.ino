//Board is M5 Atom Stack
//IMU code referenced from https://docs.m5stack.com/en/api/atom/mpu
//Orientation Filter based on https://courses.cs.washington.edu/courses/cse466/14au/labs/l4/madgwick_internal_report.pdf
//Code based on M5 original code

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


float ax_c, ay_c, az_c;
float gx_c, gy_c, gz_c;

void calibrate_IMU(){
  light_middle(100,0,0);
  const uint8_t times = 50;

  for(int i = 0; i < times; i ++){
     M5.IMU.getGyroData(&gx,&gy,&gz);
     M5.IMU.getAccelData(&ax,&ay,&az);
     
     ax_c += ax; ay_c += ay; az_c += az;
     gx_c += gx; gy_c += gy; gz_c += gz;
     delay(10);

  }

  ax_c /= 50; ay_c /= 50; az_c /= 50; //ac should be = g > 1
  gx_c /= 50; gy_c /= 50; gz_c /= 50;

  az_c = az_c - 1;

  light_middle(0,100,0);
  Serial.print(ax_c);
  Serial.print(" ");
  Serial.print(ay_c);
  Serial.print(" ");
  Serial.println(az_c);

  Serial.print(gx_c);
  Serial.print(" ");
  Serial.print(gy_c);
  Serial.print(" ");
  Serial.println(gz_c);


 
  
  return;
}

void rectify_errors(){
  ax = ax - ax_c;
  ay = ay - ay_c;
  az = az - az_c;

  gx = gx - gx_c;
  gy = gy - gy_c;
  gz = gz - gz_c;
 
}

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

    calibrate_IMU();

}

void loop(){
    M5.IMU.getGyroData(&gx,&gy,&gz);
    M5.IMU.getAccelData(&ax,&ay,&az);

    rectify_errors();

    MahonyAHRSupdateIMU(
        gx * DEG_TO_RAD, gy * DEG_TO_RAD, gz * DEG_TO_RAD,
        ax, ay, az,
        &pitch, &roll, &yaw);

    //Serial.printf("pitch:%.2f, roll:%.2f", pitch,roll);
    //Serial.println();

    Serial.print(pitch);
    Serial.print(" ");
    Serial.println(roll);

    /*
    Serial.printf("ax:%.2f, ay:%.2f, az:%.2f", ax,ay,az);
    Serial.println();
    Serial.printf("gx:%.2f, gy:%.2f, gz:%.2f", gx,gy,gz);
    Serial.println();
    Serial.println();
    */
    

    
    delay(READ_DELAY);


}
