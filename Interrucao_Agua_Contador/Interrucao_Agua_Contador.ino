/* Contador de Garrafões de Água
Autor: Yuri Lima
Empresa: AndXor
Site: http://www.andxor.com.br
---Acesso Remoto---
AndXor_Coun_Agua.dyndns.org
---Acesso Interno---
http://192.168.25.177/interno
---IMPORTANTE---
-> Todas as vezes que mudar de arduino, mude a macro ZERATUDO para true.
-> Se depois de inicar a contagem vc retirar o SD CARD a contagem so vai funcionar corretamente, quando vc inserir novamente o SD.
*/
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include "AndXor_GFX.h"
#include "AndXor_SSD1306.h"
//================================================
//Zerar tudo
#define RESET_COUNT  0//True-1 / False-0 -> Zerar tudo
//================================================
//Debug
#define DEBUG 0 //True-1 / False-0 -> Para debug no Monitor Serial
//================================================
//Data e Hora
#define SET_TIME_DATE 0 //True-1 / False-0 -> Para validar a data e a hora modificada
//================================================
//Strings
static String str_dia_semana;
//================================================
//EEPROM
volatile uint32_t S_ISR_EE; //Guarda o somatorio dos numeros guardados no endereços da EEPRON -> Ela é obrigatoriamente uma variavel global, nao tente mudar!! -> Faz parte da ISR
int16_t end_EE = 0x01;//Guarda os ultimos endereços da EEPRON na posição ZERO -> Ela é obrigatoriamente uma variavel global, nao tente mudar!! -> Faz parte da ISR
bool flag_EE = true;
//================================================
//OLED
#define OLED_RESET 0x08
#define DLY_OLED 0xc8//Tempo de visualização no display 200ms
AndXor_SSD1306 display(OLED_RESET);
//================================================
//RTC
#define DS3231_ADDRESS 0x68// Modulo RTC no endereco 0x68
//================================================
//Sd Card
File ARQ;//Objeto da classe File
volatile bool SD_flag;
//================================================
//Contadores
const uint16_t C_Limit = 0x7D00; //Limita a contagem(Padrão 32.000)
volatile uint32_t C_Water; //Contador que so vai até 255, ele é guardo dentro de cada posição da eepron para ser somado a variavel S_ISR_EE
//-> Ela é obrigatoriamente uma variavel global, nao tente mudar!! -> Faz parte da ISR
//================================================
//Sensor Reflexivo
#define REFLEX_SENSE 0x12 //pino 18 de interruoção do arduino mega

void setup() {
  pinMode(REFLEX_SENSE, INPUT_PULLUP);
  //Interrupção do Sensor Retro-Reflexivo
  attachInterrupt(digitalPinToInterrupt(REFLEX_SENSE), Contador_ISR, FALLING);
#if RESET_COUNT
  Reset_Count(EEPROM.read(0xff));
#endif
#if DEBUG
  Serial.begin(115200);
#endif
  //I2C RTC 3231
  Wire.begin();
#if SET_TIME_DATE
  SelecionaDataeHora();//Tem que ficar abaixo do Wire.begin é ativado apenas para mudar data e hora
#endif
  //==========================================================================================================
  //I2C OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setRotation(0x02);//Rotação do display.
  display.setTextSize(0x02);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.clearDisplay();
  display.print(F("AndXor"));
  display.display();
  delay(DLY_OLED); display.clearDisplay();
  //==========================================================================================================
  //Verifica de tem SD-CARD
  while (!SD.begin(0x35)) {//pino 53 do arduino mega
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    display.println(F("Erro ao iniciar."));
    display.setCursor(35, 10);
    display.println(F("Reinicie!"));
    display.setCursor(35, 20);
    display.print(F("98769.9288"));
    display.display();
    display.setTextSize(0x01);
    display.setTextColor(WHITE);
    display.setCursor(30, 40);
    display.println(F("Diagnostico: "));
    display.setCursor(25, 50);
    display.print(F("Sem cartao SD"));
    display.display();
  }
}
//Fim Setup

void loop() {
  int16_t segundos, minutos, horas, diadasemana, diadomes, mes, ano;
  if (SD_flag) {
    WriteSDEE(horas,  minutos, segundos, diadomes, mes, ano, C_Water);//Passa os parametros para escrita no SD
    SD_flag = false;
  } 
  // Le os valores (data e hora) do modulo DS130
  WIRE(&segundos, &minutos, &horas, &diadasemana, &diadomes, &mes, &ano);
  //==========================================================================================================
  switch (diadasemana) {
    case 1: str_dia_semana = F("Segunda");
      break;
    case 2: str_dia_semana = F("Terca");
      break;
    case 3: str_dia_semana = F("Quarta");
      break;
    case 4: str_dia_semana = F("Quinta");
      break;
    case 5: str_dia_semana = F("Sexta");
      break;
    case 6: str_dia_semana = F("Sabado");
      break;
    case 7: str_dia_semana = F("Domingo");
      break;
  }
  //==========================================================================================================
  //Faz essa rotina ativar apenas uma vez a cada rest ou queda de luz
  uint16_t pos_EE[0xff];
  //Rotina para saber se caiu energia ou mudou o dia
  if (EEPROM.read(0xff) != diadasemana) { //endereço 255
    Reset_Count(diadasemana);
  }
  if (flag_EE) {
    for (int16_t i = 0x01; i < 0xff; i++) {//i==1 pois a posição zero esta reservada
      pos_EE[i] = EEPROM.read(i);//Realizar a leitura dos endereços da EEPRON e guarda no vetor
      //Soma os vetores acima de zero em soma_Pos_EE
      if (pos_EE[i] > 0x00) {
        S_ISR_EE += pos_EE[i];
      }
      //Verifica lembra o numero que está guardado na ultima posição
      if (pos_EE[i] > 0 && pos_EE[i] < 0xff) {
        C_Water = pos_EE[i];
        end_EE = i;
      }
      //Caso o numero da ultima posição seja 255 significa que ele foi para o proximo endereço da eeprom
      //Entao se posição atual é 0 e a leitura do endereço anterior é 255 guardamos a posição atual e o valor
      //atual do endereço em C_Water para posterior soma com soma_Pos_EE
      if (pos_EE[i] == 0 && EEPROM.read(i - 0x01) == 0xff) {
        C_Water = 0x00;
        end_EE = i;
      }
    } flag_EE = false;
  }
  if (S_ISR_EE == C_Limit) {
    erro();//Para todo o processo e vai para erro() de contagem Maxima;
  }
  print_Oled(S_ISR_EE, minutos,  horas,  diadasemana,  diadomes,  mes,  ano);//Imprimi Display
  delay(10);  
#if DEBUG
   //Posição na EEPROM - Contador interno até 255 - Somador(Acumulador) de cada valor escrito posição da EEPROM. 
   Serial.print(end_EE); Serial.print(" - "); Serial.print(C_Water); Serial.print(" - "); Serial.println(S_ISR_EE);
  //HORAS Serial.print(horas); Serial.print(":"); Serial.println(minutos);
  //DATA Serial.println(diadasemana); Serial.print(" - "); Serial.print(diadomes); Serial.print("/"); Serial.print(mes); Serial.print("/"); Serial.println(ano);
  //Limite de contagem Serial.println(C_Limit);
#endif
}
//Fim do Loop
//==========================================================================================================
//Controle de EEPRON + SD escrita
void WriteSDEE(int16_t horas, int16_t minutos, int16_t segundos, int16_t diadomes, int16_t mes, int16_t ano, int16_t C_Water) {
  ARQ = SD.open(F("LogData.txt"), FILE_WRITE);//abri arq no SD
  ARQ.print(F("Quantidade: ")); ARQ.println(S_ISR_EE);
  ARQ.print(F("Horario: ")); ARQ.print(horas); ARQ.print(F(":")); ARQ.println(minutos);
  ARQ.print(F("Data: ")); ARQ.print(diadomes); ARQ.print(F("/")); ARQ.print(mes); ARQ.print(F("/")); ARQ.println(ano); ARQ.println(F("------------------------------------------------------"));
  ARQ.close();
  //Rotina que salva no primeiro endereço da EEPRON==1 quando C_Water é menor que 255
  if (C_Water > 0x00 && C_Water <= 0xff)EEPROM.write(end_EE, C_Water); //Na posição zero ja tem que esta gravado a posição inicial de leitura
  if (C_Water == 0xff) {
    end_EE++;
    EEPROM.write(0x00, end_EE);//coloca a posição 0 o endereço da EEPRON atual
    C_Water = 0x00;
    S_ISR_EE = 0x00;
    EEPROM.write(end_EE, C_Water);//Pra validar uma das condição if da EEPRON salva ZERO no proximo endereço
    flag_EE = true;
  }
}
//==========================================================================================================
void SelecionaDataeHora() { //Seta a data e a hora do DS1307
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 15; //Valores de 0 a 59
  byte horas = 12; //Valores de 0 a 23
  byte diadasemana = 7; //Valores de 1 a 7 - 7=Domingo, 0 = Segunda, etc.
  byte diadomes = 18; //Valores de 1 a 31
  byte mes = 10; //Valores de 1 a 12
  byte ano = 15; //Valores de 0 a 99
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)0);
  //As linhas abaixo escrevem no CI os valores de
  //data e hora que foram colocados nas variaveis acima
  Wire.write(ConverteParaBCD(segundos));
  Wire.write(ConverteParaBCD(minutos));
  Wire.write(ConverteParaBCD(horas));
  Wire.write(ConverteParaBCD(diadasemana));
  Wire.write(ConverteParaBCD(diadomes));
  Wire.write(ConverteParaBCD(mes));
  Wire.write(ConverteParaBCD(ano));
  Wire.write((byte)0);
  Wire.endTransmission();
}
//==========================================================================================================
//Conversão
//==========================================================================================================
byte ConverteParaBCD(byte val) {
  //Converte o número de decimal para BCD
  return ( (val / 0x0A * 0x10) + (val % 0x0A) );
}
//==========================================================================================================
//Conversão
//==========================================================================================================
byte ConverteparaDecimal(byte val) {
  //Converte de BCD para decimal
  return ( (val / 0x10 * 0x0A) + (val % 0x10) );
}
//==========================================================================================================
void erro() {
  //Imprime no display a contagem
  if (C_Limit <= 0x3E7) {//999
    display.setTextSize(0x04);
    display.setTextColor(WHITE);
    display.setCursor(40, 0);
    display.clearDisplay();
    display.print(C_Limit);
  }
  //==========================================================================================================
  else if (C_Limit > 0x3E7) { //Deslocamento dos numeros no display 999
    //==========================================================================================================
    display.setTextSize(0x04);//4
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.clearDisplay();
    display.print(C_Limit);
  }
  //==========================================================================================================
  else if (C_Limit > 0x270F) { //Deslocamento dos numeros no display 9999
    //==========================================================================================================
    display.setTextSize(0x03);
    display.setTextColor(WHITE);
    display.setCursor(30, 0);
    display.clearDisplay();
    display.print(C_Limit);
  }
  display.setTextSize(0x02);
  display.setTextColor(WHITE);
  display.setCursor(25, 30);
  display.print(F("Contagem"));
  display.setTextSize(0x02);
  display.setTextColor(WHITE);
  display.setCursor(30, 50);
  display.print(F("Maxima"));
  display.display();
  delay(600000);//10 minutos no aguardo para depois zerar tudo
  Reset_Count(EEPROM.read(0xff));
}

void WIRE(int16_t* segundos, int16_t* minutos, int16_t* horas, int16_t* diadasemana, int16_t* diadomes, int16_t* mes, int16_t* ano) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 0x07);//7
  *segundos = ConverteparaDecimal(Wire.read());
  *minutos = ConverteparaDecimal(Wire.read());
  *horas = ConverteparaDecimal(Wire.read() & 0b111111);
  *diadasemana = ConverteparaDecimal(Wire.read());
  *diadomes = ConverteparaDecimal(Wire.read());
  *mes = ConverteparaDecimal(Wire.read());
  *ano = ConverteparaDecimal(Wire.read());
}
void print_Oled(int16_t soma_Pos_EE, int16_t minutos, int16_t horas, int16_t diadasemana, int16_t diadomes, int16_t mes, int16_t ano) {
  //Imprime no display a contagem
  if (soma_Pos_EE <= 0x3E7) {
    display.setTextSize(0x04);
    display.setTextColor(WHITE);
    display.setCursor(40, 0);
    display.clearDisplay();
    display.print(soma_Pos_EE);
  }
  //==========================================================================================================
  else if (soma_Pos_EE > 0x3E7 && soma_Pos_EE <= 0x270F) { //Deslocamento dos numeros no display
    //==========================================================================================================
    display.setTextSize(0x04);
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.clearDisplay();
    display.print(soma_Pos_EE);
  }
  //==========================================================================================================
  else if (soma_Pos_EE > 0x270F && soma_Pos_EE <= 0x7D00) { //Deslocamento dos numeros no display 32.000
    //==========================================================================================================
    display.setTextSize(0x03);
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.clearDisplay();
    display.print(soma_Pos_EE);
  }
  //==========================================================================================================
  //Data
  display.setTextSize(0x01);
  display.setTextColor(WHITE);
  display.setCursor(10, 40);
  display.print(str_dia_semana); display.print(F(" - "));
  if (diadomes < 0x0A) {//10
    display.print(F("0"));
    display.print(diadomes);
  } else display.print(diadomes);
  display.print(F("/"));
  if (mes < 0x0A) {
    display.print(F("0"));
    display.print(mes);
  } else display.print(mes);
  display.print(F("/")); display.print(ano);
  //==========================================================================================================
  //Horario
  display.setCursor(45, 50);
  if (horas < 0x0A) {
    display.print(F("0"));
    display.print(horas);
  } else display.print(horas);
  display.print(F(":"));
  if (minutos < 0x0A) {
    display.print(F("0"));
    display.print(minutos);
  } else display.print(minutos);
  /* display.print(":");
   if (segundos < 10) {
     display.print("0");
     display.print(segundos);
   } else display.print(segundos);*/
  display.display(); display.clearDisplay();
}
void Reset_Count(int16_t diadasemana) {
  EEPROM.write(0xff, diadasemana);//Salva o diadasemana na EEPROM
  C_Water = 0x00;
  S_ISR_EE = 0x00;
  EEPROM.write(0x00, 0x01);//Inicializador que vai lembrar a ultima posição da contagem. So ativar uma vez e depois comentar
  for (int16_t i = 0x00; i < 0xFF; i++) EEPROM.write(i, 0x00); //zera todos os endereços da EEPRON
}
void Contador_ISR() {
  C_Water++, S_ISR_EE++, SD_flag = true;
  delayMicroseconds(50);
}



