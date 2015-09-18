#include <Wtv020sd16p.h>
#include <Servo.h>
Servo myservo, myservo2;
#define boca 6
#define asa 7
int pos = 0, pos2 = 0;
#define echoPin 10
#define trigPin 8
double duracao = 0;
double disT = 0;
#define play 2
#define reset2 3
#define resetPin 4  //Pino Reset
#define clockPin A5  //Pino clock
#define dataPin A4   //Pino data (DI)
int busyPin = 5;   //Pino busy
long previousMillis = 0;        // Variável de controle do tempo
long redLedInterval = 10000;     // Tempo em ms do intervalo a ser executado
long previousMillis2 = 0;        // Variável de controle do tempo
long redLedInterval2 = 20000;     // Tempo em ms do intervalo a ser executado
unsigned long currentMillis;
unsigned long currentMillis2;

//Variavel que armazena os caracteres recebidos
char buf;

Wtv020sd16p wtv020sd16p(resetPin, clockPin, dataPin, busyPin);

void setup() {
  pinMode(9, INPUT);
  pinMode(play, OUTPUT);
  pinMode(reset2, OUTPUT);
  myservo.attach(boca);
  myservo.write(50);
  myservo2.attach(asa);
  myservo2.write(50);
  //Inicializa a serial
  Serial.begin(9600);
  //Inicializa o modulo WTV020
  wtv020sd16p.reset();

}

void loop() {
  //Serial.println(distancia());
  Serial.println(myservo.read());
  if(myservo.read()!=50)myservo.write(50);
  //if (distancia() < 20) {
  //==========================================================================
  //Boca
  currentMillis = millis();    //Tempo atual em ms
  if (currentMillis - previousMillis > redLedInterval) {
    previousMillis = currentMillis;
    Serial.print("Boca");Serial.println(" - ");
    Serial.println("Play");
    digitalWrite(play, 1); //Play
    delay(10);
    digitalWrite(play, 0); //Base
    for (pos = 50; pos >= 0; pos -= 1) {
      myservo.write(pos);
      delay(50);
    }
    for (pos = 0; pos <= 50; pos += 1) {
      myservo.write(pos);
      delay(50);
    }
    Serial.println("Pause");
    digitalWrite(play, 1); //Pausa
    delay(10);
    digitalWrite(play, 0); //Base
  }
  //==========================================================================
  //==========================================================================
  //Asa
  currentMillis2 = millis();    //Tempo atual em ms
  if (currentMillis2 - previousMillis2 > redLedInterval2) {
    Serial.println("Asa");
    previousMillis2 = currentMillis2;
    for (pos2 = 50; pos2 >= 0; pos2 -= 1) {
      myservo2.write(pos2);
      delay(50);
    }
    for (pos2 = 0; pos2 <= 50; pos2 += 1) {
      myservo2.write(pos2);
      delay(50);
    }
  }
  //}
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
  duracao = pulseIn(echoPin, HIGH, 2000);
  disT = duracao / 58;
  if (disT == 0) {
    disT = 999;//3E7->999
  }
  return disT;
}

