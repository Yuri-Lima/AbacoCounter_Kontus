#include <Servo.h>
#include "Smile.h"
//==========================================================
//Buzzer
#define BUZ 5 string
//==========================================================
//RGB
LED rgb(8,7,6);//Seta os pinos do Led RGB
//==========================================================
//PID construtor 
PID meuPid1(20, 0.02, 0.05);
PID meuPid2(3.2, 0.02, 0.05);
#define SetPoint 20 //Seta em 10 Cm
//========================================================== 
//Ultrasson
ULTRA Ultrasson(10,9);//Passa o Echo e o Trig
//==========================================================
//Servo
Servo myservo1;
Servo myservo2; 
#define direita 1468//velocidade em microssegundos por passo //90 
#define esquerda 1436//velocidade em microssegundos por passo //86
short int controlePwm1 = direita;//90;//direita parado e quando diminui  velocidade
short int controlePwm2 = esquerda;//86;//esquerda parado e quando aumetar ele vai pra frente e aumentar a velocidade 86+45

void setup() {
  Serial.begin(9600);
  meuPid1.setSetPoint(SetPoint);//Passa o offset/ponto de referencia
  meuPid2.setSetPoint(SetPoint);                                                                                                                                                                                  
}
void loop() {
  int flag;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
    flag=1;
    while(Ultrasson.distancia()<40){
      if(flag==1){
        myservo1.attach(12);//direito  
        myservo2.attach(11);//esquerdo
        Serial.print(meuPid1.process());Serial.print(" - ");Serial.println(Ultrasson.distancia());
      }flag=0;
      //diT=Ultrasson.distancia(); 
      // Manda as amostra de leitura da distancia para o objeto PID!
      //Serial.println(diT);
      meuPid1.addNewSample(int(Ultrasson.distancia()));//converto o retorno para inteiro e passa o paramentro
      //meuPid2.addNewSample(diT);
      // Converte para controle
      int PID=meuPid1.process();
      controlePwm1 = (PID + controlePwm1);//Direita
      controlePwm2 = (abs(PID - controlePwm2)+10);//Esquerda
      // Intervalo para desativar os pinos do servo
      //if((controlePwm1<96 && controlePwm1>84) || (controlePwm2<92 && controlePwm2>80)){
     /* while(distancia()<25 && distancia()>15){
        digitalWrite(BLUE,1);  
        myservo1.detach();//direito 
        myservo2.detach();//esquerdo
        controlePwm1=direita;
        controlePwm2=esquerda;
      }*/
        myservo1.attach(12);//direito  
        myservo2.attach(11);//esquerdo
     
        rgb.statusBlue(0);
        myservo1.writeMicroseconds(controlePwm1);// direita quanto menor maior velocidade
        myservo2.writeMicroseconds(controlePwm2);// esquerda
        (controlePwm1<direita) || (controlePwm2>esquerda)?rgb.statusGreen(1):rgb.statusRed(1);
        controlePwm1=direita;
        controlePwm2=esquerda;
    }
      rgb.statusRed(0);
      rgb.statusBlue(0);
      rgb.statusGreen(0);
      myservo1.detach();//direito 
      myservo2.detach();//esquerdo    
}


