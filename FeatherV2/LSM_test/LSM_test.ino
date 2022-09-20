/*
 * Testing the ESP32 Feather (select this in Arduino IDE)
 * Use of LSM IMU
 * Calibration Not implemented
 */

#include <MadgwickAHRS.h> //https://github.com/arduino-libraries/MadgwickAHRS
#include <Adafruit_LSM6DSOX.h>

//Feather
#define NEOPIXEL_I2C_POWER 2

//LSM
Adafruit_LSM6DSOX sox; //declare sox object
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;

//Magdwick 
Madgwick filter;
#define SAMPLE_FREQ 10
const float GRAVITY = 9.81;
//const float RAD_TO_DEG = 180/3.142; //Commented out i think its already defined 

const uint16_t TIME_DELAY_MILLIS = 1.0/SAMPLE_FREQ * 1000;
float ax, ay, az;
float gx, gy, gz;
float roll, pitch;

void get_readings(){
  //Get readings from LSM, convert them to parameters and update IMU with it
  sox.getEvent(&accel, &gyro, &temp);

  //Accel in m/s^2 > convert to g
  ax = accel.acceleration.x/GRAVITY;
  ay = accel.acceleration.y/GRAVITY;
  az = accel.acceleration.z/GRAVITY;

  //Gyro in rad/s > convert to deg/s
  gx = gyro.gyro.x * RAD_TO_DEG;
  gy = gyro.gyro.y * RAD_TO_DEG;
  gz = gyro.gyro.z * RAD_TO_DEG;
  
  filter.updateIMU(gx, gy, gz, ax, ay, az);  
}


unsigned long int time_now = millis();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(22,20);
  filter.begin(SAMPLE_FREQ);
  
  //Enable I2C
  pinMode(NEOPIXEL_I2C_POWER,OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER,HIGH);

  delay(100); //give time for i2c to init
  
  while(!Serial) delay(10); //Wait until Serial port opens

  if (!sox.begin_I2C()){
    Serial.println("Unable to find LSM");
    while(1);
  }
  

}

void loop() {
  // put your main code here, to run repeatedly:

  if( (millis() - time_now) >= TIME_DELAY_MILLIS){

    get_readings();
    
    roll = filter.getRoll();
    pitch = filter.getPitch();

    Serial.print(roll);
    Serial.print(" ");
    Serial.print(pitch);
    Serial.println("");

    time_now = millis(); //reset flag
    
  }

  




}
