//SensorTemperaturaProjeto.ino

//Autor: Yuri Lima Empresa: AutoCore Robotica
//Blog: http://www.autocorerobotica.blog.br/
//Loja http://www.autocorerobotica.com/
//-------------------------------------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <Servo.h>
Servo myservo;
//=========================================
#define butao 4
//=========================================
//DHT11
#define DHTPIN A1 // pino de dados 
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
//=========================================
//Encoder
#define encoder0PinA  2
#define encoder0PinB  3
//=========================================
//Oled
#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);
//=========================================
short int encoder0Pos=0, cont=0;
short int cursorA=80, cursorB=0x00, negativoPos=44;//Afasta o cursor qnd negativo
short int H, T, B;
short int Set1=0, Set2=0;
short int val;

void setup() {
  myservo.attach(9); 
  //Inicializações====================================================================================
  Serial.begin (9600);
  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // usando I2C addr 0x3C (para o display de 128x64)
  delay(100);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20,0);
  display.println("Autocore");
  display.setCursor(20,20);
  display.println("Robotica");
  display.display();
  delay(1000);
  display.clearDisplay();
  //Estados da pinagem====================================================================================
  pinMode(encoder0PinA, INPUT);
  digitalWrite(encoder0PinA, HIGH);       
  pinMode(encoder0PinB, INPUT);
  digitalWrite(encoder0PinB, HIGH);       
  //Interrupções====================================================================================
  attachInterrupt(0, doEncoder, CHANGE);  // encoder pino 2 usando a interrupção 0
}
 
void loop(){    
    while(cont<2){
      leitButao(&B);//Recebe a leitura do butao 0 ou 1
      display.setTextSize(1);display.setTextColor(WHITE);display.setCursor(0,40);display.println("Qual a");
      display.setCursor(40,40);display.println("Temp");
      display.setCursor(70,40);display.println("Desejada?");display.display();
      //Mantem um intervalo de max 100º e min -100º
      if(encoder0Pos<100 && encoder0Pos>-100){
        myservo.write(val+20); 
        display.setTextSize(5);
        display.setTextColor(WHITE);
        display.setCursor(negativoPos,0);
        display.println(encoder0Pos);
        display.setTextSize(1);
        display.setCursor(cursorA,cursorB);
        display.println("o");
        display.display();
        delay(10);
      }
      else{
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(44,0);
        display.println("Valor");
        display.setCursor(20,40);
        display.println("invalido!");
        display.display();
        delay(10);
      }
      if(B==1)break;
    }
        B=0;
        display.clearDisplay();
        if(cont>1){escreveDisplay();}
        if(cont<=1)cont++;  
}

void escreveDisplay(){
  short int H, T;
//========================================
  //Temperatura Atual
    TemperaturaUmidade(&H,&T);//Recebe os valores da umidade H e da temperatura T
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("Temp.Atual");
    //-------------------------------------
    display.setTextSize(2);
    display.setCursor(44,40);
    display.println((int)T); //Imprimi no display a temperatura
    //-------------------------------------
    display.setTextSize(1);
    display.setCursor(75,40);
    display.println("o");// simbolo do Grau
    //-------------------------------------
    display.display();
    //-------------------------------------
    alarme(&T,&Set1,&Set2);//Envia a temperatura atual e a temperatura setada
    //-------------------------------------
    delay(2000);
    display.clearDisplay();
//=================================================
  //Temperatura Programada
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print("Temp.");
    //-------------------------------------
    display.setTextSize(2);
    display.setCursor(60,0);
    display.println("Prog.");
    //-------------------------------------
    display.setTextSize(2);
    display.setCursor(24,40);
    display.println(Set1); //Mostra a temp programada
    //-------------------------------------
    display.setTextSize(1);
    display.setCursor(55,40);
    display.println("o");// simbolo do Grau
    //-------------------------------------
    display.setTextSize(2);
    display.setCursor(74,40);
    display.println(Set2); //Mostra a temp programada
    //-------------------------------------
    display.setTextSize(1);
    display.setCursor(105,40);
    display.println("o");// simbolo do Grau
    //-------------------------------------
    display.display();
    delay(2000);
    display.clearDisplay();

 }

void doEncoder() {
 //if(cont<=1){
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
      display.clearDisplay();
      //val=encoder0Pos++;
      //myservo.write(val+20); 
      dispGrau(&encoder0Pos);
    } 
    else {
      display.clearDisplay();
      //val=encoder0Pos--;
      //myservo.write(val-20); 
      dispGrau(&encoder0Pos);
   }
// }
}
//Função so para acerta a posição da bolinha do grau
void dispGrau(short int* encoder){
  if(*encoder>9){
    negativoPos=44; 
    cursorA=120;//Ajuste do ponto Grau
    cursorB=0;
    delay(30);
  }
  else if(*encoder==0){
    negativoPos=44;
    cursorA=80;
    cursorB=0;
    delay(30);
  }
  else if (*encoder<0 && *encoder>-9){
    negativoPos=15;
    cursorA=84;
    cursorB=0;
    delay(30);
  }
  else if (*encoder<=-10){
    negativoPos=15;
    cursorA=119;
    cursorB=0;
    delay(30);
  }
  if(cont==0){Set1=*encoder;}
  else if(cont==1){Set2=*encoder;}

}
void TemperaturaUmidade(short int *a, short int *b){
   int h = dht.readHumidity(); //Chama a função dentro da biblioteca para realizar a leitura da Umidade e guarda na variavel h
   int t = dht.readTemperature(); //Chama a função dentro da biblioteca para realizar a leitura da Umidade e guarda na variavel t
    if (isnan(t) || isnan(h)) //Verifica se a
  {
    Serial.println("Falha na leitura do sensor DHT-11");
  } 
  else{
    *a=h;
    *b=t; 
  }
}
 void leitButao(short int* but){
    short int leituraButao=digitalRead(butao);
    delay(50);
    if(leituraButao==1){
      *but=leituraButao; 
    }
 }
void alarme(short int* atual, short int* prog1, short int * prog2){
    if(*atual>*prog2 || *atual <*prog1){
      display.setTextSize(2);
      display.setCursor(20,20);
      display.println("ALERTA");
      display.display();
      delay(1000);
      display.clearDisplay();
    }

}


void doEncoder_Expanded(){
  if(cont<=1){
  if (digitalRead(encoder0PinA) == HIGH) {   // found a low-to-high on channel A
    if (digitalRead(encoder0PinB) == LOW) {  // check channel B to see which way
       display.clearDisplay();                                      // encoder is turning
      encoder0Pos = encoder0Pos - 1;         // CCW
      dispGrau(&encoder0Pos);
    }
    else {
      display.clearDisplay();
      encoder0Pos = encoder0Pos + 1;         // CW
      dispGrau(&encoder0Pos);
    }
  }
  else                                        // found a high-to-low on channel A
  {
    if (digitalRead(encoder0PinB) == LOW) {   // check channel B to see which way
        display.clearDisplay();                                      // encoder is turning  
      encoder0Pos = encoder0Pos + 1;          // CW
      dispGrau(&encoder0Pos);
    }
    else {
      display.clearDisplay();
      encoder0Pos = encoder0Pos - 1;          // CCW
      dispGrau(&encoder0Pos);
    }
 
  }
}
}