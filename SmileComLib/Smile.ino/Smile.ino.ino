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
//PID meuPid2(3.2, 0.02, 0.05);
#define SetPoint 20 //Seta em 10 Cm
//========================================================== 
//Ultrasson
ULTRA Ultrasson(10,9);//Passa o Echo e o Trig
//Ultrasonic Ultrasonic(9,10);
//==========================================================
//Servo
Servo myservo1;
Servo myservo2; 
double controlePwm1 = 1468;//90;//direita parado e quando diminui  velocidade
double controlePwm2 = 1436;//86;//esquerda parado e quando aumetar ele vai pra frente e aumentar a velocidade 86+45
double flag,flag2,diT=0,diT2=0,PID=0;
double controlePwm11=0;
double controlePwm22=0;
void setup() {
  Serial.begin(9600);
  meuPid1.setSetPoint(SetPoint);//Passa o offset/ponto de referencia
//  meuPid2.setSetPoint(SetPoint);                                                                                                                                                                                  
}
void loop() {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
    flag=1;
    Serial.println(Ultrasson.distancia());
      while(Ultrasson.distancia()<40){
      if(flag==1){
        myservo1.attach(12);//direito  
        myservo2.attach(11);//esquerdo
      }flag=0;
      diT=Ultrasson.distancia();
      delayMicroseconds(20);
      diT2=Ultrasson.distancia();
      //if(diT<10||diT2<10){diT=10;diT2=10;}
      diT!=0 || diT<400?meuPid1.addNewSample(diT):meuPid1.addNewSample(diT2); 
      // Manda as amostra de leitura da distancia para o objeto PID!
      //converto o retorno para inteiro e passa o paramentro
      //meuPid2.addNewSample(diT);
      // Converte para controle
      PID=meuPid1.process();
      controlePwm11 = (PID + controlePwm1);//Direita
      controlePwm22 = (abs(PID - controlePwm2));//Esquerda
      myservo1.writeMicroseconds(int(controlePwm11));// direita quanto menor maior velocidade
      myservo2.writeMicroseconds(int(controlePwm22)+10);// esquerda
       Serial.print(PID);Serial.print(" - ");Serial.print(int(controlePwm11));Serial.print(" - ");Serial.print(int(controlePwm22));Serial.print(" - ");Serial.print(diT);Serial.print(" - ");Serial.println(diT2);
      // Intervalo para desativar os pinos do servo     
              
        if(diT<=25 && diT>=20){
          flag2=1;
          while(Ultrasson.distancia()<=25 && Ultrasson.distancia()>=20){
            //digitalWrite(BLUE,1);
            Serial.println(Ultrasson.distancia());
            if(flag2==1){
              myservo1.detach();//direito 
              myservo2.detach();//esquerdo
              controlePwm11=0;
              controlePwm22=0;
              flag=1;
            }flag2=0;
            //controlePwm1=direita;
            //controlePwm2=esquerda;
          }
       }
       controlePwm11=0;
       controlePwm22=0;
    }
      //rgb.statusRed(0);
      //rgb.statusBlue(0);
      //rgb.statusGreen(0);
     myservo1.detach();//direito 
     myservo2.detach();//esquerdo    
}


