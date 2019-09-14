#include "SR04.h"
#include <Servo.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#define TRIG_PIN 0 //D3
#define ECHO_PIN 4 //D2


/* Put your SSID & Password */
const char* ssid = "Questa";  // Enter SSID here
const char* password = "lasolita";  //Enter Password here


//Motore
Servo myservo_1, myservo_2;
int pos_servo1, pos_servo2;


//Sensori materiali
int inductivePin = 12; //pin sensore induttivo D6
int capacitivePin = 14; //pin sensore capacitivo D5
int inductiveState = LOW; //stato sensore induttivo
int capacitiveState = LOW; //stato sensore capacitivo


//Sensore ultrasuoni
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long oldDistance = 2000;
long newDistance;
int distanceSensibility = 10;


//Bilancia
int fsrAnalogPin = A0;
int weight = 0;


//Utility varie
int triggerDelay = 3000;
int material_type;


void setup() {

  //Inizializzazione dei sensori
  Serial.begin(9600);
  delay(1000);
  pinMode(inductivePin,INPUT);
  pinMode(capacitivePin,INPUT);
  myservo_1.attach(5);
  myservo_2.attach(16);
  
  //Collegamento al router
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected !");
  Serial.println("########Pronto########");
}

void loop() {
  
    newDistance=sr04.Distance();

    if(newDistance <= distanceSensibility && oldDistance > distanceSensibility){
      Serial.println("Piatto: PIENO");
      Serial.println("Calcolo...");
      delay(triggerDelay);
      int valInductive = digitalRead(inductivePin);
      int valCapacitive = digitalRead(capacitivePin);
         
      inductiveState = valInductive;
      capacitiveState = valCapacitive;
      oldDistance = newDistance;
      
      Serial.print("Elaboro tipo materiale");
      delay(2000);
      if( inductiveState == 0 ){
        material_type = 1;
      }else if(capacitiveState == 1){
        material_type = 2;
      }else{
        material_type = 4;
      }
      Serial.println("Fatto.");
      delay(1000);
      ////////////////////////////////////////////////
      Serial.println("Misura peso...");
      delay(2000);
      int sum = 0;
      for(int i=0; i<25; i++)
        sum += analogRead(fsrAnalogPin);
      weight = sum/25;
      ////////////////////////////////////////////////
      
      if( material_type == 1 ){
        Serial.print( "Metallo\t" );
        for (pos_servo1 = 0; pos_servo1 <= 180; pos_servo1 += 1) {
          myservo_1.write(pos_servo1);              
          delay(15);
        }
      }else if(material_type == 2){
        if(weight > 230){
            Serial.print("Vetro");
            material_type = 3;  
        }else{
            Serial.print("Plastica");
            material_type = 2;
            for (pos_servo2 = 0; pos_servo2 <= 180; pos_servo2 += 1) {
              myservo_2.write(pos_servo2);              
              delay(15);
            }
        }
      }else{
        Serial.print("Indifferenziata\t");
      }
      Serial.println();
      Serial.print("Peso: ");
      Serial.print(weight);
      Serial.println(" g");
      

      if(WiFi.status()== WL_CONNECTED){
        Serial.println("Invio dati al server...");
        HTTPClient http;
        String url = "http://192.168.43.69:8000/history/"+String(material_type);
        url += "/"+String(weight);
        url += "/update";
        Serial.println(url);
        http.begin(url);
        http.GET();
        Serial.println("Dati inviati");  
      }
      
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
      }else if(material_type == 2){
        for (pos_servo2 = 180; pos_servo2 >= 0 ; pos_servo2 -= 1) {
          myservo_2.write(pos_servo2);              
          delay(15);
        }  
      }
      delay(triggerDelay);
      Serial.println("########Pronto########");
    }
}
