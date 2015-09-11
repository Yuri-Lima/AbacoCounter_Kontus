  
//Programa : Modulo WTV020-SD e Arduino Uno
//Autor : Arduino e Cia
 
#include <Wtv020sd16p.h>
#include <Servo.h> 
Servo myservo; 
byte pos= 0, pos2=0;

#define tempMusic 120000
int resetPin = 4;  //Pino Reset
int clockPin = 21;  //Pino clock
int dataPin = 20;   //Pino data (DI)
int busyPin = 7;   //Pino busy
 
//Variavel que armazena os caracteres recebidos
char buf;

Wtv020sd16p wtv020sd16p(resetPin,clockPin,dataPin,busyPin);
 
void setup(){
  myservo.attach(9);
  Serial.begin(9600);
  wtv020sd16p.reset();
  menu_inicial();
}
 
void loop(){
  for(pos = 0, pos2=0; pos <= 180, pos2 <= 180; pos += 1, pos2 += 1) { myservo.write(pos); myservo.write(pos2); delay(15);} 
  for(pos = 180, pos2 = 180; pos>=0, pos2>=0; pos-=1, pos2-=1) { myservo.write(pos); myservo.write(pos2); delay(15);}
  
  while(Serial.available() > 0){
    buf = Serial.read();
     if (buf == '0'){
      Serial.println("Reproduzindo 0001.ad4");
      wtv020sd16p.playVoice(0);
      //Reproduz o arquivo 1
      wtv020sd16p.asyncPlayVoice(0);
      delay(tempMusic);
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();  
    }
    if (buf == '1'){
      Serial.println("Reproduzindo 0002.ad4");
      wtv020sd16p.playVoice(1);
      //Reproduz o arquivo 1
      wtv020sd16p.asyncPlayVoice(1);
      delay(tempMusic);
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();  
    }
     
    //Caso seja recebido o numero 2, reproduz o arquivo 0002.ad4
    if (buf == '2')
    {
      Serial.println("Reproduzindo 0002.ad4");
      wtv020sd16p.playVoice(2);
      //Reproduz o arquivo 2
      wtv020sd16p.asyncPlayVoice(2);
      delay(tempMusic);
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();
    }
    if (buf == '3')
    {
      Serial.println("Reproduzindo 0002.ad4");
      wtv020sd16p.playVoice(3);
      //Reproduz o arquivo 2
      wtv020sd16p.asyncPlayVoice(3);
      delay(tempMusic);
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();
    }
    if (buf == '4')
    {
      Serial.println("Reproduzindo 0002.ad4");
      wtv020sd16p.playVoice(4);
      //Reproduz o arquivo 2
      wtv020sd16p.asyncPlayVoice(4);
      delay(tempMusic);
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();
    }
    if (buf == '5')
    {
      Serial.println("Reproduzindo 0002.ad4");
      wtv020sd16p.playVoice(5);
      //Reproduz o arquivo 2
      wtv020sd16p.asyncPlayVoice(5);
      delay(tempMusic);
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();
    }
     if (buf == 'r')
    {
      Serial.println("Reset");
      wtv020sd16p.stopVoice();
      delay(2000);
      menu_inicial();  
    }
  }
}
 
//Mostra menu de opcoes
void menu_inicial()
{
  Serial.println("\nDigite : ");
  Serial.println("1 - Reproduz o arquivo 0001.ad4");
  Serial.println("2 - Reproduz o arquivo 0002.ad4");
  Serial.println();
}
