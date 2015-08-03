
#include <Servo.h>
#include "Smile.h" 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//===============================================
//Display Oled
#define OLED_RESET 6

//===============================================
//Led
#define LED 4
//===============================================
//Seguidor de linha
#define SENSOR_PROXIMIDADE A0 //SENSOR_PROXIMIDADE como A0
#define SENSOR_ESQ A1 //Define SENSOR_ESQ como A1
#define SENSOR_DIR A0 //Define SENSOR_DIR como A0
//===============================================
//Servo 
Servo myservo;
Servo myservo2;
#define pinEsq 6
#define pinDir 5
//===============================================
//Ultrasson 
ULTRA ultrasson(3,2); 
#define distMin 2
#define distMax 400
#define detecMax 40
#define erro 5
int readDist=0, count=0,i=2,j=2;
//===============================================
//PID
PID meuPID(1.0,0.25,0.05);
double setPoint=20;
/////////////////////////////////////////////////
void setup(){ 
  Serial.begin(9600);
//===============================================
//Led
  pinMode(LED,OUTPUT);
//===============================================
//Servo
  pinMode(SENSOR_ESQ, INPUT_PULLUP); 
  pinMode(SENSOR_DIR, INPUT_PULLUP);                                                             
//===============================================
//PID
  meuPID.setSetPoint(setPoint); 
 delay(2000); 
} 
 
void loop(){
  
  //Serial.print(analogRead(SENSOR_ESQ));Serial.print(" - ");//51 >430
 // Serial.println(analogRead(SENSOR_DIR));//197>430
  
   
  
   // Serial.println(ultrasson.distancia());
    while(ultrasson.distancia() < detecMax && ultrasson.distancia()!= 999){
        count++; 
        if(count>60){
           readDist=ultrasson.distancia(); if( readDist!=999) goto liga; else goto desliga;
           liga:
           delay(10);    
           digitalWrite(LED,1);
           while(readDist<detecMax && readDist>setPoint){
              rightFrente(1);leftFrente(1);
              while(digitalRead(SENSOR_ESQ)>430 ){
               left(1);}
               while(digitalRead(SENSOR_DIR)>400){
                 right(1);}
             //front(1);
             readDist=ultrasson.distancia();delay(10);
             if(readDist==999) goto desliga;
           }
           while(readDist<=setPoint+erro){
             //back(1);
             readDist=ultrasson.distancia();delay(10);
             if(readDist==999) goto desliga;
           }   
        }
    }     
    desliga:    
    myservo.detach();  
    myservo2.detach();
    digitalWrite(LED,0);
    count=0;delay(10); 
} 
void rightFrente(int F){
  if(F==1){
    myservo2.attach(pinDir);
    myservo2.write(70); //direita
  }
}
void leftFrente(int B){
  if(B==1){
    myservo.attach(pinEsq);
    myservo.write(100);
    
  }
}
/*
Servo esquerdo de 90 - 170 vai para frente 90 -> lento e 170 -> rapido
Servo direito de 80 - 10 vai para frente 80 -> lento e 0 -> rapido
Servo esquerdo de 0 - 80 vai para traz 0 -> rapido e 80 -> lento
Servo direito de 180 - 90 vai para traz 170 -> rapido e 90 -> lento
*/
void right(int R){
  if(R==1){
    myservo2.detach();
    leftFrente(1);
    delay(400);
  }
}
void left(int L){
  if(L==1){
    myservo.detach();
    rightFrente(1);
    delay(400);
  }
}
void parado(int P){
  if(P==1){
    myservo.detach();
    myservo2.detach();
    delay(150);
  }
}
