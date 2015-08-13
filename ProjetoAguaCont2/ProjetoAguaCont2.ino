#include "Agua.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS1307.h>
#include <String.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

String diadasemana2, horas2, minutos2, diadomes2, mes2, ano2;
//================================================
//EEPROM
#define addr 0
//================================================
//OLED
#define OLED_RESET 8
Adafruit_SSD1306 display(OLED_RESET);
//================================================
//RTC
#define DS1307_ADDRESS 0x68// Modulo RTC no endereco 0x68
int lastMinutos = 0;
int lastHoras = 0;
byte horaZero = 23;//Horario para a variavel contAgua ZERAR
byte zero = 0x00;
//================================================
//Sd Card
File arquivo;
char* cleanCel = "";
byte flagSD = 0x00;
//================================================
//Contadores
//unsigned long int 0 - 4.294.967.295 4 bytes
unsigned int count = 0x00; // Contador do filtro limite de 65.535 2 bytes
byte flagAgua = 0x00; //Sinalizador de estado logico alto
unsigned int countAgua = 0x00; //Contador de agua limite de 65.535 2 bytes
//================================================
//Ultrasson
#define TRIG 2
#define ECO 3
#define detecMax 20 //Limite de detecção em 20 cm
#define filtro 10 //Define a quantidade minima de leituras para distinguir um objeto
#define timeFiltro 10
ULTRA ultra(ECO, TRIG); //Objeto da classe ultra

void setup() {
  Wire.begin();
  SelecionaDataeHora();//tem que ficar abaixo do Wire.begin é ativo apenas para mudar data e hora
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.clearDisplay();
  display.print("Robot One");
  display.display();
  delay(2000); display.clearDisplay();
  if (!SD.begin(4)) {//Teste para saber se tem SD-CARD
    //Serial.println("Erro ao iniciar cartao SD");
    // Serial.println("Diagnostico: Sem cartao SD");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    display.println("Erro ao iniciar");
    display.setCursor(35, 10);
    display.print("cartao SD");
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 40);
    display.println("Diagnostico: ");
    display.setCursor(25, 50);
    display.print("Sem cartao SD");
    display.display(); delay(2000); display.clearDisplay();
    return;
  }
  else {
    flagSD = 0x01;
    //Serial.println("Cartao SD Iniciado");
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.clearDisplay();
    display.print("Cartao SD Iniciado");
    display.display();
    delay(2000); display.clearDisplay(); display.clearDisplay();
  }

}

void loop() {
  if (flagSD != 0x00) {
    //===================================================================
    // Le os valores (data e hora) do modulo DS1307
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(zero);
    Wire.endTransmission();
    Wire.requestFrom(DS1307_ADDRESS, 7);
    int segundos = ConverteparaDecimal(Wire.read());
    int minutos = ConverteparaDecimal(Wire.read());
    int horas = ConverteparaDecimal(Wire.read() & 0b111111);
    int diadasemana = ConverteparaDecimal(Wire.read());
    int diadomes = ConverteparaDecimal(Wire.read());
    int mes = ConverteparaDecimal(Wire.read());
    int ano = ConverteparaDecimal(Wire.read());
    //====================================================================
    if(horas == 23){ countAgua =0x00; EEPROM.write(addr, addr);}else{ countAgua = EEPROM.read(addr);}//Rotina para saber se caiu energia ou mudou o dia

    switch (diadasemana) {
      case 0: diadasemana2 = "Dom";
        break;
      case 1: diadasemana2 = "Seg";
        break;
      case 2: diadasemana2 = "Ter";
        break;
      case 3: diadasemana2 = "Qua";
        break;
      case 4: diadasemana2 = "Qui";
        break;
      case 5: diadasemana2 = "Sex";
        break;
      case 6: diadasemana2 = "Sab";
        break;
    }
    flagAgua = 0x00;
    while (ultra.distancia() < detecMax && ultra.distancia() != 999) {
      count++;
      flagAgua = 0x01;//verificações
    }
    if (count > filtro && flagAgua == 0x01 && ultra.distancia() == 999) {
      countAgua += 0x01;//conta as garrafas
      WriteSD(horas,  minutos, segundos, diadomes, mes, ano, countAgua);//passa os parametros para escrita no SD


    }
    count = 0x00;
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    display.clearDisplay();
    display.println(countAgua);
    display.display(); display.clearDisplay();
    delay(timeFiltro);
  }
}

void SelecionaDataeHora() { //Seta a data e a hora do DS1307
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 11; //Valores de 0 a 59
  byte horas = 0; //Valores de 0 a 23
  byte diadasemana = 4; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 13; //Valores de 1 a 31
  byte mes = 8; //Valores de 1 a 12
  byte ano = 15; //Valores de 0 a 99
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //Stop no CI para que o mesmo possa receber os dados

  //As linhas abaixo escrevem no CI os valores de
  //data e hora que foram colocados nas variaveis acima
  Wire.write(ConverteParaBCD(segundos));
  Wire.write(ConverteParaBCD(minutos));
  Wire.write(ConverteParaBCD(horas));
  Wire.write(ConverteParaBCD(diadasemana));
  Wire.write(ConverteParaBCD(diadomes));
  Wire.write(ConverteParaBCD(mes));
  Wire.write(ConverteParaBCD(ano));
  Wire.write(zero);
  Wire.endTransmission();
  Serial.print("entrou");
}

byte ConverteParaBCD(byte val) {
  //Converte o número de decimal para BCD
  return ( (val / 10 * 16) + (val % 10) );
}

byte ConverteparaDecimal(byte val) {
  //Converte de BCD para decimal
  return ( (val / 16 * 10) + (val % 16) );
}
void WriteSD(int horas, int minutos, int segundos, int diadomes, int mes, int ano, int countAgua) {
  EEPROM.write(0, countAgua);
  arquivo = SD.open("Cadastro.csv", FILE_WRITE);//escreve no SD
  arquivo.seek(0x00);
  arquivo.print("Quantidade: ");
  //int linha=arquivo.seek(0x00);Serial.println(linha);
  arquivo.println(countAgua);
  arquivo.print("Horario: ");
  arquivo.print(horas); arquivo.print(":"); arquivo.println(minutos);
  arquivo.print("Data: "); arquivo.print(diadasemana2); arquivo.print("  --  ");
  arquivo.print(diadomes); arquivo.print("/"); arquivo.print(mes); arquivo.print("/"); arquivo.print(ano);
  arquivo.close();
}


/*
// Acrescenta o 0 (zero) se a hora for menor do que 10
  if (horas < 10)display.print("0");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 0);
  display.print(horas);display.print(":");display.display(); if(lastHoras!=horas)display.clearDisplay();//limpa a tela
  lastHoras=horas;
  Serial.print(horas);
  Serial.print(":");

  // Acrescenta o 0 (zero) se minutos for menor do que 10
  if (minutos < 10)Serial.print("0");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(38, 0);
  display.print(minutos);display.display();if(lastMinutos!=minutos)display.clearDisplay();//limpa a tela
  lastMinutos=minutos;
  Serial.print(minutos);
  //Serial.print(":");
  /*if (segundos < 10)Serial.print("0");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(56, 0);
  display.print(segundos);display.display(); delay(50);display.clearDisplay();
  Serial.print(segundos);
  Serial.print("    ");

  // Mostra o dia da semana
  switch (diadasemana)
  {
    case 0: Serial.print("Dom");
      break;
    case 1: Serial.print("Seg");
      break;
    case 2: Serial.print("Ter");
      break;
    case 3: Serial.print("Qua");
      break;
    case 4: Serial.print("Qui");
      break;
    case 5: Serial.print("Sex");
      break;
    case 6: Serial.print("Sab");
  }
   Serial.print("  ");
  // Acrescenta o 0 (zero) se dia do mes for menor do que 10
  if (diadomes < 10) Serial.print("0");
  Serial.print(diadomes);
  Serial.print("/");
  // Acrescenta o 0 (zero) se mes for menor do que 10
  if (mes < 10)  Serial.print("0");
  Serial.print(mes);
  Serial.print("/");
  Serial.println(ano);*/

