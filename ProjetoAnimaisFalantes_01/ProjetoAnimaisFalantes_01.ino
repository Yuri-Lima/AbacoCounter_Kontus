
//Programa : Modulo WTV020-SD e Arduino Uno
//Autor : Arduino e Cia

#include <Wtv020sd16p.h>
#include <Servo.h>
Servo myservo,myservo2;
int pos = 0,pos2=0;

#define echoPin 2
#define trigPin 3
double duracao = 0;
double disT = 0;
boolean flag = 0;
#define tempMusic 3000
# define resetPin 4  //Pino Reset
#define clockPin A5  //Pino clock
#define dataPin A4   //Pino data (DI)
int busyPin = 5;   //Pino busy
long previousMillis = 0;        // Variável de controle do tempo
long redLedInterval = 10000;     // Tempo em ms do intervalo a ser executado
long previousMillis2 = 0;        // Variável de controle do tempo
long redLedInterval2 = 12000;     // Tempo em ms do intervalo a ser executado

//Variavel que armazena os caracteres recebidos
char buf;

Wtv020sd16p wtv020sd16p(resetPin, clockPin, dataPin, busyPin);

void setup() {
  myservo.attach(6);
  myservo2.attach(7);
  Serial.begin(9600);
  wtv020sd16p.reset();
  myservo.write(0);
  menu_inicial();
}

void loop() {
  //==========================================================================
  //Asa
  unsigned long currentMillis = millis();    //Tempo atual em ms
  if (currentMillis - previousMillis > redLedInterval) {
    previousMillis = currentMillis;    
    for (pos = 0; pos <= 50; pos += 1) { 
      myservo.write(pos);              
      delay(5);                       
    }
    for (pos = 50; pos >= 0; pos -= 1) {
      myservo.write(pos);              
      delay(5);                       
    }
  }
  //==========================================================================
   //==========================================================================
  //Boca
   unsigned long currentMillis2 = millis();    //Tempo atual em ms
  if (currentMillis2 - previousMillis2 > redLedInterval2) {
    previousMillis2 = currentMillis2;    
    for (pos2 = 0; pos2 <= 70; pos2 += 1){ // in steps of 1 degree
      myservo2.write(pos2);              
      delay(5);                       
    }
    for (pos2 = 70; pos2 >= 0; pos2 -= 1) {
      myservo2.write(pos2);              
      delay(5);                       
    }
  }
  if (distancia() < 20) {
    Serial.println("Reproduzindo 0001.ad4");
    wtv020sd16p.playVoice(1);
    //Reproduz o arquivo 1
    wtv020sd16p.asyncPlayVoice(0);
    delay(tempMusic);
    wtv020sd16p.stopVoice();
    delay(20);
    menu_inicial();
  }
}


//Mostra menu de opcoes
void menu_inicial()
{
  Serial.println("\nDigite : ");
  Serial.println("1 - Reproduz o arquivo 0001.ad4");
  Serial.println();
}
int distancia() {
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
  duracao = pulseIn(echoPin, HIGH, 4000);
  disT = duracao / 58;
  if (disT == 0) {
    disT = 999;//3E7->999
  }
  return disT;
}
