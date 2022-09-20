/*
 * Use this to get offset values of ax ay az and gx gy gz
 * Note that acceleration is interms of g, and the gyroscopic values is
 * in terms of rad/s
 * 
 */

#include <Adafruit_LSM6DSOX.h>

//Feather
#define NEOPIXEL_I2C_POWER 2

//LSM
Adafruit_LSM6DSOX sox; //declare sox object
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;

const float GRAVITY = 9.81;

float ax, ay, az;
float gx, gy, gz;

void Calibrate(){
  for(int i = 0; i < 100; i++){
    sox.getEvent(&accel, &gyro, &temp);
    //Accel in m/s^2 > convert to g
    
    ax += accel.acceleration.x/GRAVITY;
    ay += accel.acceleration.y/GRAVITY;
    az += accel.acceleration.z/GRAVITY;
  
    //Gyro in rad/s
    gx += gyro.gyro.x ;
    gy += gyro.gyro.y ;
    gz += gyro.gyro.z ;

    delay(50);
  }

  ax = ax/100.0; ay = ay/100.0; az = az/100.0;
  gx = gx/100.0; gy = gy/100.0; gz = gz/100.0;

  Serial.println("Acceleration offsets in g");
  Serial.print(ax); Serial.print(" ");
  Serial.print(ay); Serial.print(" ");
  Serial.print(az); Serial.print(" ");
  
  Serial.println("");
  
  Serial.println("Gyroscope offsets in rad/s");
  Serial.print(gx); Serial.print(" ");
  Serial.print(gy); Serial.print(" ");
  Serial.print(gz); Serial.print(" ");
  
  Serial.println();
  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(22,20);

  pinMode(NEOPIXEL_I2C_POWER,OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER,HIGH);

  delay(100); //give time for i2c to init

  if (!sox.begin_I2C()){
    Serial.println("Unable to find LSM");
    while(1);
  }

  Serial.println("Offsets in g and in deg/s");

  Calibrate();
}

void loop() {
  // put your main code here, to run repeatedly:

}
