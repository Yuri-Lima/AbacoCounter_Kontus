#include <Servo.h>
#include "Smile.h"
//==========================================================
//Buzzer
#define BUZ 5 string
//==========================================================
//RGB
LED rgb(8, 7, 6); //Seta os pinos do Led RGB
//==========================================================
//PID construtor
PID meuPid1(10, 0.02, 0.05);
#define SetPoint 20 //Seta em 20 Cm
double PID1 = 0;
//==========================================================
//Ultrasson
ULTRA Ultrasson(13, 12); //Passa o Echo e o Trig
#define distLimitMax 400 //Define que a medidas maxima do sensor é de 4 metros
#define distLimitMin 2 //Define que a medidas minimas do sensor é de 2 centimetro
#define alcanceMin 40 //Define o alcance de ativação do Smile
#define filtroInter 5 //Esse primeiro é para filtrar ruidos dos sinais abaixo de 40 quando ele estiver parado sem nenhum obstaculo a ser detectado
#define filtroInter2 5 //Esse segunda filtra possiveis ruidos acima ou abaixo do intervalo de parada 1 e parada 2
#define parada1 25 //Primeiro intervalo de parada Maxima
#define parada2 20 //Segundo intervalo de parada Minima
double lastDist = 0, disT = 0, count = 0; //lastDist guarda a ultima leitura da distancia, count é a variavel utilizadas para contagem e obtencao da certeza de um objeto ou obstaculo.
//==========================================================
//Servo
Servo myservo1;
Servo myservo2;
double posCentral = 1468;//direita parado e quando diminui  velocidade
double posCentral2 = 1436;//esquerda parado e quando aumetar ele vai pra frente e aumentar a velocidade
//==========================================================
//Flags
boolean flag, flag2;

void setup() {
  Serial.begin(9600);
  meuPid1.setSetPoint(SetPoint);//Passa o offset/ponto de referencia
}
void loop() {
  flag = 1;
  Serial.println(Ultrasson.distancia());
  if ((Ultrasson.distancia() < distLimitMax) && (Ultrasson.distancia() > distLimitMin)) {
    Serial.println(Ultrasson.distancia());
    while (Ultrasson.distancia()< alcanceMin) {
      Serial.println(Ultrasson.distancia());
      count++;//filtra possiveis interferencias de sinal do ultrasson quando o valor de alcance for menor que o previsto
      if (count > filtroInter) { //Se foi realizado as leituras foram feitas acima da quantidade count é pq realmente tem um objeto para realizar uma ação
        if (flag != 0) {
          myservo1.attach(12);//direito
          myservo2.attach(11);//esquerdo
        } flag = 0;//Muda o estado para os pinos do servo serem ativados apenas uma vez
        // Intervalo para desativar os pinos do servo
        if (disT <= parada1 && disT >= parada2) {
          count = 0;
          while ((Ultrasson.distancia() <= parada1) && (Ultrasson.distancia() >= parada2)) {
            count++; lastDist = Ultrasson.distancia(); //pra assegura que a leitura do sensor esta dentro do range de maximo 4 metros e filtrar possiveis ruidos
            Serial.println(Ultrasson.distancia());
            if (count > filtroInter2) {
              myservo1.detach();//direito
              myservo2.detach();//esquerdo
              flag = 1;//Sinaliza novamente a ativação dos pinos do servo
            } delay(50);
            disT=Ultrasson.distancia();
            ((Ultrasson.distancia() < distLimitMax) && (Ultrasson.distancia() > distLimitMin)) ? disT = disT : disT = lastDist;//assegura uma boa leitura dos intervalos de parada
          }
        }
        disT=Ultrasson.distancia();
        //Manda as amostra de leitura da distancia para o objeto PID!
        meuPid1.addNewSample(disT);
        // Converte para controle
        PID1 = meuPid1.process();
        myservo1.writeMicroseconds(int(PID1 + posCentral));// Direita quanto menor maior velocidade
        myservo2.writeMicroseconds(int(abs(PID1 - posCentral2)));// Esquerda
        Serial.print(PID1); Serial.print(" - "); Serial.print(int(PID1 + posCentral)); Serial.print(" - "); Serial.print(int(abs(PID1 - posCentral2))); Serial.print(" - "); Serial.println(Ultrasson.distancia());
        delay(100);
      }
    }
  }
  //rgb.statusRed(0);
  //rgb.statusBlue(0);
  //rgb.statusGreen(0);
  myservo1.detach();//direito
  myservo2.detach();//esquerdo
  count = 0;
  delay(50);
}


