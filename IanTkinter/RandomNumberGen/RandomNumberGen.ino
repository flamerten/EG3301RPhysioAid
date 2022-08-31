#include <Adafruit_NeoPixel.h> //This is for the onboard LEDs

//On board LED matrix
#define LED_COUNT 25
#define LED_PIN   27
#define BRIGHTNESS 100 //set as 0 if you do not want to show the indicator
bool led_off = true;
Adafruit_NeoPixel matrix(LED_COUNT,LED_PIN , NEO_GRB + NEO_KHZ800);
int x = 0.1;

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

void setup() {
  // put your setup code here, to run once:
  init_matrix();
  Serial.begin(115200);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(random(0,180));
  Serial.print(" ");
  Serial.print(random(0,180));
  Serial.print(" ");
  Serial.println(int(abs(sin(x)) * 180));
  if(led_off) light_middle(0,0,100);
  else light_middle(0,0,0);

  led_off = !led_off;
  delay(100);

  x++;
  
  
  

}
