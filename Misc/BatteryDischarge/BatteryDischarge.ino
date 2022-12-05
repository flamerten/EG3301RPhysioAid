#define BATREAD 33
#define ENABLE 22
#define LOWBAT 19

int PD;

void setup(){
    Serial.begin(115200);

    pinMode(BATREAD,INPUT);
    pinMode(LOWBAT,INPUT);

    pinMode(ENABLE,OUTPUT);
    
    digitalWrite(ENABLE,HIGH); //set to LOW to turn off the Booster
    

}

void loop(){
    PD = analogRead(BATREAD);
    if(digitalRead(LOWBAT) == LOW){
        Serial.println("BAT LOW");
        digitalWrite(ENABLE,LOW); //TURN OFF
        while(1);
    }

    Serial.println(PD);

    delay(1000);

}
