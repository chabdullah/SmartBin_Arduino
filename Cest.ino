#include "SR04.h"
#include <Servo.h>
#define TRIG_PIN 0 //D3
#define ECHO_PIN 4 //D2

//Motore
Servo myservo_1;
int pos_servo1;


int inductivePin = 12; //pin sensore induttivo D6
int capacitivePin = 14; //pin sensore capacitivo D5
int fsrAnalogPin = 16; //pin bilancia
int fsrReading; //segnale bilancia
int inductiveState = LOW; //stato sensore induttivo
int capacitiveState = LOW; //stato sensore capacitivo

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long oldDistance = 2000;
long newDistance;
int distanceSensibility = 10;

int triggerDelay = 3000;
int material_type;

void setup() {
  
  Serial.begin(9600);
  delay(1000);
  pinMode(inductivePin,INPUT);
  pinMode(capacitivePin,INPUT);
  myservo_1.attach(5);
}

void loop() {
  
    newDistance=sr04.Distance();
//    Serial.println(distance);
//    oldDistance = newDistance;
//    Serial.print(oldDistance);
//    Serial.print("\t");
//    Serial.println(newDistance);

    if(newDistance <= distanceSensibility && oldDistance > distanceSensibility){
      Serial.println("Piatto: PIENO");
      Serial.println("Calcolo...");
      delay(triggerDelay);
      int valInductive = digitalRead(inductivePin);
      int valCapacitive = digitalRead(capacitivePin);
         
      inductiveState = valInductive;
      capacitiveState = valCapacitive;
      oldDistance = newDistance;
      
      Serial.print("Materiale = ");
      if( inductiveState == 0 ){
        material_type = 1;
        Serial.print( "Metallo\t" );
        for (pos_servo1 = 0; pos_servo1 <= 180; pos_servo1 += 1) {
          myservo_1.write(pos_servo1);              
          delay(15);
        }
      }else if(capacitiveState == 1){
        material_type = 2;
        Serial.print( "Plastica/Vetro\t");
      }else{
        material_type = 3;
        Serial.print("Indifferenziata\t");
      }
      Serial.println();
      Serial.println("Misura peso...");
      delay(2000);
      int sum = 0;
      for(int i=0; i<25; i++)
        sum += analogRead(fsrAnalogPin);
      fsrReading = sum/25;
      Serial.print("Peso: ");
      Serial.print(fsrReading);
      Serial.println(" g");
      
    }else if(oldDistance <= distanceSensibility && newDistance > distanceSensibility){
      Serial.println("Caduto");
      oldDistance = newDistance;
      Serial.println("Piatto: VUOTO");
      Serial.println("Attendere...");
      if(material_type == 1){
        for (pos_servo1 = 180; pos_servo1 >= 0 ; pos_servo1 -= 1) {
          myservo_1.write(pos_servo1);              
          delay(15);
        }  
      }
      delay(triggerDelay);
      Serial.println("########Pronto########");
    }
}
