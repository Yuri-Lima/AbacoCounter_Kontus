/*
//Stepper myStepper(stepsPerRevolution,10,11,8,9,7,5,4,6);//ir para frente
//Stepper myStepper(stepsPerRevolution,8,9,10,11,4,6,7,5);//ir para tras
Stepper myStepper(stepsPerRevolution,10,11,8,9,4,6,7,5);//ir para Direta
//Stepper myStepper(stepsPerRevolution,8,9,10,11,7,5,4,6);//ir para esquerda
*/
#include <Stepper.h>
#include "Smile.h"
//==========================================================
//Buzzer
#define BUZ 5 string
//==========================================================
//RGB
LED rgb(8, 7, 6); //Seta os pinos do Led RGB
//==========================================================
//PID construtor
PID meuPid1(50, 0.02, 0.05);
#define SetPoint 20 //Seta em 20 Cm
double PID1 = 0;
//==========================================================
//Ultrasson
ULTRA Ultrasson(13, 12); //Passa o Echo e o Trig
#define distLimitMax 400 //Define que a medidas maxima do sensor é de 4 metros
#define distLimitMin 2 //Define que a medidas minimas do sensor é de 2 centimetro
#define alcanceMin 40 //Define o alcance de ativação do Smile
#define erro 5 //Primeiro intervalo de parada Maxima
double lastDist = 0, disT = 0, count = 0; //lastDist guarda a ultima leitura da distancia, count é a variavel utilizadas para contagem e obtencao da certeza de um objeto ou obstaculo.
//==========================================================
//Passos
 int stepsPerRevolution =200;
 double parado = 0,velocidade=0;
Stepper myStepper(stepsPerRevolution,10,11,8,9,7,5,4,6);//ir para frente ou para tras
//Stepper myStepper(stepsPerRevolution,10,11,8,9,4,6,7,5);//ir para Direta ou esquerda


void setup() {
  Serial.begin(9600);
  meuPid1.setSetPoint(SetPoint);//Passa o offset/ponto de referencia
  myStepper.setSpeed(velocidade);
}
void loop() {
  Serial.println(Ultrasson.distancia());
  if ((Ultrasson.distancia() < distLimitMax) && (Ultrasson.distancia() > distLimitMin)) {
    //Serial.println(Ultrasson.distancia());
    while (Ultrasson.distancia()< alcanceMin) {
        //Manda as amostra de leitura da distancia para o objeto PID!
        
         while(Ultrasson.distancia()>SetPoint + erro && Ultrasson.distancia()< alcanceMin){
          disT=Ultrasson.distancia();
          meuPid1.addNewSample(disT);
          PID1 = meuPid1.process();
          velocidade= PID1+parado;
          velocidade=abs(velocidade);
          //velocidade=map(velocidade,0,1000,10,100);
          velocidade<=150?myStepper.setSpeed(velocidade):myStepper.setSpeed(150);
          myStepper.step(200);
          Serial.println(disT);
        }
        while(Ultrasson.distancia()<SetPoint - erro  && Ultrasson.distancia() < alcanceMin){
          disT=Ultrasson.distancia();
          meuPid1.addNewSample(disT);
          PID1 = meuPid1.process();
          velocidade= PID1+parado;
          velocidade=abs(velocidade);
          //velocidade=map(velocidade,0,500,10,100);
          velocidade<=150?myStepper.setSpeed(velocidade):myStepper.setSpeed(150);
          myStepper.step(-200);
          Serial.println(disT);
        }
        Serial.println(Ultrasson.distancia());
       //while(Ultrasson.distancia()>=SetPoint && (Ultrasson.distancia() <= SetPoint+erro)){
         //myStepper.setSpeed(0);
         //myStepper.step(0);
       // }
         myStepper.setSpeed(0);
        myStepper.step(0);
    }
    myStepper.setSpeed(0);
    myStepper.step(0);
  }
    myStepper.setSpeed(0);
    myStepper.step(0);
    
  
}


