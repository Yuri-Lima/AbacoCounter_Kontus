//Smile.ino

class PID{
public:
  
  double error;
  double sample;
  double lastSample;
  double kP, kI, kD;      
  double P, I, D;
  double pid;
  
  double setPoint;
  long lastProcess;
  
  PID(double _kP, double _kI, double _kD){
    kP = _kP;
    kI = _kI;
    kD = _kD;
  }
  
  void addNewSample(double _sample){
    sample = _sample;
  }
  
  void setSetPoint(double _setPoint){
    setPoint = _setPoint;
  }
  
  double process(){
    // Implementação P I D
    error = setPoint<0? sample+setPoint : setPoint- sample;
    float deltaTime = (millis() - lastProcess) / 1000.0;
    lastProcess = millis();
    
    //P
    P = error * kP;
    
    //I
    I = I + (error * kI) * deltaTime;
    
    //D
    D = (lastSample - sample) * kD / deltaTime;
    lastSample = sample;
    
    // Soma tudo
    pid = P + I + D;
    
    return pid;
  }
};
//==========================================================
//Buzzer
#define BUZ 5 string
//==========================================================
//RGB
#define RED 8
#define BLUE 7
#define GREEN 6
//==========================================================
//Millis
#define seg   60//Tempo de atuação do Smile
//==========================================================
//Ultrasson
#define echoPin 10 //Pino 13 recebe o pulso do echo
#define trigPin 9 //Pino 12 envia o pulso para gerar o echo
#define SetPoint 20 //Seta em 10 Cm
//==========================================================
//PID construtor 
PID meuPid1(20, 0.02, 0.05);
PID meuPid2(3.2, 0.02, 0.05);
//========================================================== 
//Servo
#include <Servo.h>
Servo myservo1;
Servo myservo2; 
#define direita 1468//90
#define esquerda 1436//86
int controlePwm1 = direita;//90;//direita parado e quando diminui  velocidade
int controlePwm2 = esquerda;//86;//esquerda parado e quando aumetar ele vai pra frente e aumentar a velocidade 86+45

void setup() {
  Serial.begin(9600);
  //myservo1.attach(12);//direito
 // myservo2.attach(11);//esquerdo
  pinMode(echoPin, INPUT); // define o pino 13 como entrada (recebe)
  pinMode(trigPin, OUTPUT); // define o pino 12 como saida (envia)
  pinMode(RED, 1);pinMode(GREEN, 1);pinMode(BLUE, 1);
  meuPid1.setSetPoint(SetPoint);
  meuPid2.setSetPoint(SetPoint);                                                                                                                                                                                  
}
void loop() {
  int diT,flag;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
    flag=1;
    while(distancia()<40){
      if(flag=1){
        myservo1.attach(12);//direito  
        myservo2.attach(11);//esquerdo
        Serial.print(meuPid1.process());Serial.print(" - ");Serial.println(diT);
      }flag=0;
      diT=distancia(); 
      // Manda as amostra de leitura da distancia para o objeto PID!
      //Serial.println(diT);
      meuPid1.addNewSample(diT);
      //meuPid2.addNewSample(diT);
      // Converte para controle
      int teste=meuPid1.process();
      controlePwm1 = (teste + controlePwm1);//Direita
      controlePwm2 = (-(teste - controlePwm2)+10);//Esquerda
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
     
        digitalWrite(BLUE,0);
         myservo1.writeMicroseconds(controlePwm1);// direita quanto menor maior velocidade
         myservo2.writeMicroseconds(controlePwm2);// esquerda
        controlePwm1=direita;
        controlePwm2=esquerda;
        (controlePwm1<90) || (controlePwm2>86)?digitalWrite(GREEN,1):digitalWrite(RED,1);
     
    }
      digitalWrite(RED,0);
      digitalWrite(BLUE,0);
      digitalWrite(GREEN,0);
      myservo1.detach();//direito 
      myservo2.detach();//esquerdo

      
}

int distancia(){
    long duracao, distancia;
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
    duracao=pulseIn(echoPin, HIGH);
    distancia = duracao / 29.4 / 2;
    return distancia;

    
  
}
void quadrado(){

  myservo1.attach(12);//direito 
  myservo2.attach(11);//esquerdo

  myservo1.write(45);// direita
  myservo2.write(131);//esqerda


}