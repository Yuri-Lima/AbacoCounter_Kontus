
#include "Agua.h"

ULTRA::ULTRA(int _echoPin, int _trigPin) {
  echoPin = _echoPin;
  trigPin = _trigPin;
  pinMode(echoPin, INPUT); pinMode(trigPin, OUTPUT);
}
int ULTRA::distancia() {
  //seta o pino 12 com um pulso baixo "LOW" ou desligado ou ainda 0
  digitalWrite(trigPin, LOW);
  // delay de 2 microssegundos
  delayMicroseconds(2);
  //seta o pino 12 com pulso alto "HIGH" ou ligado ou ainda 1
  digitalWrite(trigPin, HIGH);
  //delay de 10 microssegundos
  delayMicroseconds(10);
  //seta o pino 12 com pulso baixo novamente
  digitalWrite(trigPin, LOW);
  duracao = pulseIn(echoPin, HIGH,1500);
  disT = duracao / 58; 
  if(disT == 0){
    disT = 999;
  }
  return disT;
}
