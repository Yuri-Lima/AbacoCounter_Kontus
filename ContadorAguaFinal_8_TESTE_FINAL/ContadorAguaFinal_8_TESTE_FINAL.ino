/* Contador de Garrafões de Água
Autor: Yuri Lima
Empresa: Robot One
Site: http://www.Robotone.com.br
---Acesso Remoto---
aguarobotone.dyndns.org
---Acesso Interno---
http://192.168.25.177/interno
---IMPORTANTE---
-> Todas as vezes que mudar o arduino tem que zerar a variavel flag2 passando ela para true
-> Se depois de inicar a contagem vc retirar o SD CARD a contagem so vai funcionar corretamente, quando vc inserir novamente o SD.
*/
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS1307.h>
//#include <Ethernet.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//================================================
//Ethernet
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
//IPAddress ip(192, 168, 25, 177);
//EthernetServer server(80);//(port 80 is default for HTTP):
//boolean arrayEstado[1];//Se true entrar para acionar as cargas
//byte estadoip = 0x00;//Variavel de teste. Se conexão interna ou externa
//================================================
//Strings
String diadasemana2;
//================================================
//EEPROM
#define addr 0x00
unsigned int somaPosEEpron = 0x00;//Guarda o somatorio dos numeros guardados no endereços da EEPRON
unsigned int endEEpron = 0x01;//Guarda os ultimos endereços da EEPRON na posição ZERO
unsigned int posEpron[256];
boolean flagEEpron = true, flagEndPron = true;
//================================================
//OLED
#define OLED_RESET 8
#define delayRobotOne 1000//Tempo de visualização no display
#define delayCartaoIniciado 1000//Tempo de visualização no display
Adafruit_SSD1306 display(OLED_RESET);
//================================================
//RTC
#define DS1307_ADDRESS 0x68// Modulo RTC no endereco 0x68
int lastHoras = 0;//Guarda a ultima leitura da hora
//byte horaZero = 23;//Horario para a variavel contAgua ZERAR
byte zero = 0x00;//Serve para recpção de dados no CI do RTC
int segundos, minutos, horas, diadasemana, diadomes, mes, ano;
//================================================
//Sd Card
File arquivo;//Objeto da classe File
boolean flagSD = true;//Flag de validação do SD-CARD
boolean flagWriteSDEE = true;//False para nao gravar nada
//================================================
//Contadores
#define limiteCont 0xF230//Limita a contagem(Padrão 62000)
//int count = 0x00; // Contador do filtro para sensibilidade do ultrasson na detecção de objetos
boolean flagAgua = false; //Sinalizador de estado logico alto
int countAgua = 0x00; //Contador de agua limite de 65.535 2 bytes
//================================================
//Sensor Reflexivo
#define sensorReflexivo  36
int detecObj = 0; //Limite de detecção
//#define filtro 2 //Define a quantidade minima de leituras para distinguir um objeto
#define timeFiltro 5 //Intervalos entre leituras que vai influenciar no filtro

void setup() {
  ZERA(false);//Iniciadores, true para zerar tudo
  pinMode(sensorReflexivo, INPUT_PULLUP);
  //Serial.begin(9600);
  //I2C RTC
  Wire.begin();
  //SelecionaDataeHora();//Tem que ficar abaixo do Wire.begin é ativado apenas para mudar data e hora
  //==========================================================================================================
  //Ethernet
  // Ethernet.begin(mac, ip);
  //==========================================================================================================
  //I2C OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setRotation(2);//Rotação do display.
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.clearDisplay();
  display.print("Robot One");
  display.display();
  delay(delayRobotOne); display.clearDisplay();
  //==========================================================================================================
  //Verifica de tem SD-CARD
  while (!SD.begin(53)) {
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
    display.display();
  }
  display.clearDisplay();
  flagSD = true;
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  //display.clearDisplay();
  display.print("Cartao SD Iniciado");
  display.display();
  delay(delayCartaoIniciado); display.clearDisplay();
}
//Fim Setup

void loop() {
  //==========================================================================================================
  //Inicio RTC
  //==========================================================================================================
  //----------------------------------------------------------------------------------------------------------
  // Le os valores (data e hora) do modulo DS130
  WIRE(&segundos, &minutos, &horas, &diadasemana, &diadomes, &mes, &ano);
  //==========================================================================================================
  if (flagSD) {
    switch (diadasemana) {
      case 0: diadasemana2 = "Domingo";
        break;
      case 1: diadasemana2 = "Segunda";
        break;
      case 2: diadasemana2 = "Terca";
        break;
      case 3: diadasemana2 = "Quarta";
        break;
      case 4: diadasemana2 = "Quinta";
        break;
      case 5: diadasemana2 = "Sexta";
        break;
      case 6: diadasemana2 = "Sabado";
        break;
    }
    //Fim RTC
    //----------------------------------------------------------------------------------------------------------
    //==========================================================================================================
    //Inicio EEPROM Energia
    //==========================================================================================================
    //-----------------------------------------------------------------------------------------------------------
    //Rotina para saber se caiu energia ou mudou o dia
    zeraTudo(horas, minutos);
    //==========================================================================================================
    //----------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------
    //==========================================================================================================
    //Faz essa rotina ativar apenas uma vez a cada rest ou queda de luz
    if (flagEEpron) {
      EEPROM.write(255, diadasemana);//Salva o diadasemana na EEPROM
      for (int i = 1; i < 255; i++) {//i==1 pois a posição zero esta reservada
        posEpron[i] = EEPROM.read(i);//Realizar a leitura dos endereços da EEPRON e guarda no vetor
        //Serial.println(posEpron[i]);
      }
      for (int i = 1; i < 255; i++) {
        //Soma os vetores acima de zero em somaPosEEpron
        if (posEpron[i] > 0) {
          somaPosEEpron += posEpron[i];
          //debug => Serial.print("i= "); Serial.print(i); Serial.print(" - "); Serial.print("endEEpron2= ");
          //debug => Serial.println(EEPROM.read(0)); endEEpron = i; //Lembra onde o endereço que parou a contagem
        }
        //Verifica lembra o numero que está guardado na ultima posição
        if (posEpron[i] > 0 && posEpron[i] < 255) {
          countAgua = posEpron[i];
          endEEpron = i;
        }
        //Caso o numero da ultima posição seja 255 significa que ele foi para o proximo endereço da eeprom
        //Entao se posição atual é 0 e a leitura do endereço anterior é 255 guardamos a posição atual e o valor
        //atual do endereço em countAgua para posterior soma com somaPosEEpron
        if (posEpron[i] == 0 && EEPROM.read(i - 1) == 255) {
          countAgua = 0;
          endEEpron = i;
        }
      }
    } flagEEpron = false;
    //debug => Serial.print(endEEpron); Serial.print(" - "); Serial.print(countAgua); Serial.print(" - "); Serial.println(somaPosEEpron);
    //Fim EEPRON
    //----------------------------------------------------------------------------------------------------------
    //==========================================================================================================
    //Inicio LDR
    //==========================================================================================================
    //----------------------------------------------------------------------------------------------------------
    //Acionamento de contagem pelo LDR
    //Serial.println(digitalRead(sensorReflexivo));
    while (digitalRead(sensorReflexivo) != detecObj) {
      flagAgua = true; //flag de verificações
      WIRE(&segundos, &minutos, &horas, &diadasemana, &diadomes, &mes, &ano);//Atualiza data e hora
      zeraTudo(horas, minutos); //Zera dependedo das ccondição e dados atualizados
      display2(somaPosEEpron, segundos,  minutos,  horas,  diadasemana,  diadomes,  mes,  ano);//Imprimi Display
    }
    if (flagAgua) {
      somaPosEEpron++;
      countAgua++;//conta as garrafas
      WriteSDEE(horas,  minutos, segundos, diadomes, mes, ano, countAgua);//Passa os parametros para escrita no SD
      if (somaPosEEpron == limiteCont) {
        flagSD = 0x00;  //Para todo o processo e vai para erro() de contagem Maxima;
        erro();
      }
    }
    //Fim Ultrasson
    display2( somaPosEEpron, segundos,  minutos,  horas,  diadasemana,  diadomes,  mes,  ano);//Imprimi Display
  }

}
//Fim do Loop
//==========================================================================================================
//----------------------------------------------------------------------------------------------------------
//Controle de EEPRON + SD escrita
void WriteSDEE(int horas, int minutos, int segundos, int diadomes, int mes, int ano, int countAgua) {
  SD.begin(53);/*Tem que estar aqui para fazer as verificação de permancia do SD-CARD. Caso contrario quando houve a retirado do SD e logo apos
                 vc inserir as novas leituras nao serão mais gravadas no SD.*/
  //if (horas - lastHoras == 1){ if(SD.exists("LogHora.csv")==true)SD.remove("LogHora.csv");} //Apaga para atualizar a tabela
  if (flagWriteSDEE == true) {
    if (SD.exists("LogData.csv")) SD.remove("LogData.csv"); //Apaga para atualizar a tabela
    arquivo = SD.open("LogData.csv", FILE_WRITE);//escreve no SD
    //if (horas - lastHoras == 1)arquivo = SD.open("LogHora.csv", FILE_WRITE); //escreve no SD
    arquivo.seek(0x00);
    arquivo.print("Quantidade: "); arquivo.println(somaPosEEpron);
    arquivo.print("Horario: "); arquivo.print(horas); arquivo.print(":"); arquivo.println(minutos);
    arquivo.print("Data: "); arquivo.print(diadomes); arquivo.print("/"); arquivo.print(mes); arquivo.print("/"); arquivo.print(ano);
    arquivo.close();
    //lastHoras = horas;//Guarda a ultima hora para depois gravar no LogHora de hora em hora
    //Rotina que salva no primeiro endereço da EEPRON==1 quando countAgua é menor que 255
    if (countAgua > 0 && countAgua <= 255)EEPROM.write(endEEpron, countAgua); //Na posição zero ja tem que esta gravado a posição inicial de leitura
    if (countAgua > 254) {
      endEEpron++;
      EEPROM.write(0, endEEpron);//coloca a posição 0 o endereço da EEPRON atual
      countAgua = 0x00;
      somaPosEEpron = 0x00;
      EEPROM.write(endEEpron, countAgua);//Pra validar uma das condição if da EEPRON salva ZERO no proximo endereço
      flagEEpron = true;
    }//debug => Serial.print(endEEpron); Serial.print(" - "); Serial.print(countAgua); Serial.print(" - "); Serial.println(somaPosEEpron);
  }
}

//==========================================================================================================
void SelecionaDataeHora() { //Seta a data e a hora do DS1307
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 29; //Valores de 0 a 59
  byte horas = 7; //Valores de 0 a 23
  byte diadasemana = 3; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 16; //Valores de 1 a 31
  byte mes = 9; //Valores de 1 a 12
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
}
//==========================================================================================================
//Conversão
//==========================================================================================================
byte ConverteParaBCD(byte val) {
  //Converte o número de decimal para BCD
  return ( (val / 10 * 16) + (val % 10) );
}
//==========================================================================================================
//Conversão
//==========================================================================================================
byte ConverteparaDecimal(byte val) {
  //Converte de BCD para decimal
  return ( (val / 16 * 10) + (val % 16) );
}
//==========================================================================================================
void erro() {
  //Imprime no display a contagem
  if (limiteCont <= 999) {
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(40, 0);
    display.clearDisplay();
    display.print(limiteCont);
  }
  //==========================================================================================================
  else if (limiteCont > 999) { //Deslocamento dos numeros no display
    //==========================================================================================================
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.clearDisplay();
    display.print(limiteCont);
  }
  //==========================================================================================================
  else if (limiteCont > 9999) { //Deslocamento dos numeros no display
    //==========================================================================================================
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(30, 0);
    display.clearDisplay();
    display.print(limiteCont);
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(25, 30);
  display.print("Contagem");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 50);
  display.print("Maxima");
  display.display();
}

void WIRE(int* segundos, int* minutos, int* horas, int* diadasemana, int* diadomes, int* mes, int* ano) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  *segundos = ConverteparaDecimal(Wire.read());
  *minutos = ConverteparaDecimal(Wire.read());
  *horas = ConverteparaDecimal(Wire.read() & 0b111111);
  *diadasemana = ConverteparaDecimal(Wire.read());
  *diadomes = ConverteparaDecimal(Wire.read());
  *mes = ConverteparaDecimal(Wire.read());
  *ano = ConverteparaDecimal(Wire.read());
}
void display2(int somaPosEEpron, int segundos, int minutos, int horas, int diadasemana, int diadomes, int mes, int ano) {

  //----------------------------------------------------------------------------------------------------------
  //==========================================================================================================
  //Imprimi Display
  //==========================================================================================================
  //----------------------------------------------------------------------------------------------------------
  //Imprime no display a contagem
  if (somaPosEEpron <= 999) {
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(40, 0);
    display.clearDisplay();
    display.print(somaPosEEpron);
  }
  //==========================================================================================================
  else if (somaPosEEpron > 999 && somaPosEEpron <= 9999) { //Deslocamento dos numeros no display
    //==========================================================================================================
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.clearDisplay();
    display.print(somaPosEEpron);
  }
  //==========================================================================================================
  else if (somaPosEEpron > 9999 && somaPosEEpron <= 63000) { //Deslocamento dos numeros no display
    //==========================================================================================================
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(25, 0);
    display.clearDisplay();
    display.print(somaPosEEpron);
  }
  //==========================================================================================================
  //Data
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 40);
  display.print(diadasemana2); display.print(" - ");
  if (diadomes < 10) {
    display.print("0");
    display.print(diadomes);
  } else display.print(diadomes);
  display.print("/");
  if (mes < 10) {
    display.print("0");
    display.print(mes);
  } else display.print(mes);
  display.print("/"); display.print(ano);
  //==========================================================================================================
  //Hora
  display.setCursor(40, 50);
  if (horas < 10) {
    display.print("0");
    display.print(horas);
  } else display.print(horas);
  display.print(":");
  if (minutos < 10) {
    display.print("0");
    display.print(minutos);
  } else display.print(minutos);
  display.print(":");
  if (segundos < 10) {
    display.print("0");
    display.print(segundos);
  } else display.print(segundos);
  display.display(); display.clearDisplay();
  //==========================================================================================================
  delay(timeFiltro);
}

void zeraTudo(int horas, int minutos) {
  if (EEPROM.read(255) != diadasemana) {
    EEPROM.write(255, diadasemana);//Salva o diadasemana na EEPROM
    countAgua = 0x00;
    somaPosEEpron = 0x00;
    for (int i = 0; i < 255; i++) {
      EEPROM.write(i, addr); //zera todos os endereços da EEPRON
    }
    //Serial.println(minutos);
  }
}
void ZERA(boolean flag2) {
  if (flag2) {
    countAgua = 0x00;
    somaPosEEpron = 0x00;
    EEPROM.write(0, 1);//Inicializador que vai lembrar a ultima posição da contagem. So ativar uma vez e depois comentar
    for (int i = 0; i < 255; i++) EEPROM.write(i, addr);
  }
}


