
#define TIME_DELAY 100 //CHANGE THIS
//IN MILLISECONDS(THE PERIOD) DEPENDING ON THE GAP BETWEEN DATA SENT
//BUT THE TIME WILL INCREMENT AT FIXED TIMES - 50MS 

float angle1;
float angle2;
float angle3;

float time_t;

unsigned long time_last_send = 0;
unsigned long time_last_increment_t = 0;
unsigned long time_now;


void setup() {
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  time_now = millis();

  if( (time_now - time_last_send) >= TIME_DELAY){
    angle1 = int(abs(sin(time_t) * 180));
    angle2 = int(abs(sin(time_t + 0.1) + 180));
    angle3 = int(abs(sin(time_t + 0.2) + 180));
  
    Serial.print("x ");
    Serial.print(angle1); Serial.print(" ");
    Serial.print(angle2); Serial.print(" ");
    Serial.print(angle3); Serial.print(" ");
    Serial.println("x"); //end

    time_last_send = time_now; //reset time
  }

  if( (time_now - time_last_increment_t) >= 50){
    time_t = time_t + 0.05;

    time_last_send = time_now; //reset time
  }

  
}
