/*
 * This file is mostly for testing the serial transmission of data from the M5 to the RPI
 * It can work for ALL mcus, just set USE_LED to be false so as not to init the matrix
 * Rather than random numbers its more of a sin wave which is mapped from 0 to 180 to simulate knee angles
 * 
 * M5 - > esp32 board -> M5 stack , others -> esp32 board -> esp32devkit
 * 
 */

#include <Adafruit_NeoPixel.h> //This is for the onboard LED matrix


#define DELAY 50 //Number of MS between each sending
const bool USE_LED = false; //Change this if you want the LEDS



//On board LED matrix
#define LED_COUNT 25
#define LED_PIN   27
#define BRIGHTNESS 100 //set as 0 if you do not want to show the indicator
bool led_off = true;
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);
float x = 0;

void init_matrix(){
  if(!USE_LED) return; //Just return if user does not want to use the LED matrix
  
  //ensure the matrix behind is turned off, reduce power usage
  //however, this can be useeful as an indicator
  matrix.begin();
  matrix.clear();
  matrix.show();
  matrix.setBrightness(BRIGHTNESS);

}

void light_middle(int r, int g, int b){
  if(!USE_LED) return; //Just return if user does not want to use the LED matrix
  
  //12 is the centre of the matrix
  matrix.setPixelColor(12,matrix.Color(r,g,b));
  matrix.show();
}

void setup() {
  // put your setup code here, to run once:
  init_matrix();
  Serial.begin(115200);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(0);
  Serial.print(" ");
  Serial.print(0);
  Serial.print(" ");
  Serial.println(int(abs(sin(x) * 180)));
  if(led_off) light_middle(0,0,100);
  else light_middle(0,0,0);

  led_off = !led_off;
  delay(50);

  x = x + 0.05;
  
  
  

}
