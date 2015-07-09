//Robot One
//www.robotone.com.br
//GarraRoboticaMG1.ino

#include <Servo.h>
#include "DHT.h"
#define echoPin 6
#define trigPin 7
#define media 3//Filtro

Servo myservo; 

void setup(){
  myservo.attach(12);
  pinMode(echoPin, INPUT); // define o pino 13 como entrada (recebe)
  pinMode(trigPin, OUTPUT); // define o pino 12 como saida (envia)
  Serial.begin(9600);
}

void loop(){ 
	while(mediaMovel()>=10){
	    myservo.write(116);
	}
	while(mediaMovel()<10 && mediaMovel()>7){
	myservo.write(37); 	
	delay(1000);
	}
 	while(mediaMovel()<5){
	    myservo.write(116);
	}                   
} 
int  mediaMovel(){
	int vetorDist[media];
	int soma=0;
	for(int i=0; i<media; i++){
		vetorDist[i]=distancia();
	}
	for(int j=0; j<media; j++){
	    soma+=vetorDist[j];
	}
	return soma/media;
}

int distancia(){
 	long distancia;
 	long duracao;
	//seta o pino 12 com um pulso baixo "LOW" ou desligado ou ainda 0
	digitalWrite(trigPin, 0);
	// delay de 2 microssegundos
	delayMicroseconds(2);
	//seta o pino 12 com pulso alto "HIGH" ou ligado ou ainda 1
	digitalWrite(trigPin, 1);
	//delay de 10 microssegundos
	delayMicroseconds(10);
	//seta o pino 12 com pulso baixo novamente
	digitalWrite(trigPin, 0);
	duracao=pulseIn(echoPin, 1);
	distancia = duracao / 29.4 / 2;
	//distancia = ((duracao * C)/1000000)/2;
	return (int)distancia;
}