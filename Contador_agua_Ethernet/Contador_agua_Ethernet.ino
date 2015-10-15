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
#include <Ethernet.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//================================================
//Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
const char server[] = "www.google.com";    // name address for Google (using DNS)
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 25, 177);
EthernetClient client;
//================================================
//Strings
static String diadasemana2;
//================================================
//EEPROM
#define addr 0x00
volatile uint32_t somaPosEEpron = 0x00;//Guarda o somatorio dos numeros guardados no endereços da EEPRON
uint16_t endEEpron = 0x01;//Guarda os ultimos endereços da EEPRON na posição ZERO
bool flagEEpron = true;
//================================================
//OLED
#define OLED_RESET 8
#define delayRobotOne 200//Tempo de visualização no display
#define delayCartaoIniciado 200//Tempo de visualização no display
Adafruit_SSD1306 display(OLED_RESET);
//================================================
//RTC
#define DS3231_ADDRESS 0x68// Modulo RTC no endereco 0x68
int16_t segundos, minutos, horas, diadasemana, diadomes, mes, ano;
//================================================
//Sd Card
File arquivo;//Objeto da classe File
bool flagSD = true;//Flag de validação do SD-CARD
volatile bool flag3;
//================================================
//Contadores
#define limiteCont 0x7D00//Limita a contagem(Padrão 32.000)
volatile uint32_t countAgua = 0x00; //Contador de agua limite de 65.535 2 bytes

//================================================
//Sensor Reflexivo
#define sensorReflexivo  18

void setup() {
  //Interrupção do Sensor Retro-Reflexivo
  attachInterrupt(digitalPinToInterrupt(sensorReflexivo), Contador, RISING );
  ZERA(0);//Iniciadores, true para zerar tudo
  pinMode(sensorReflexivo, INPUT_PULLUP);
  //Serial.begin(115200);
  //Serial.println(EEPROM.length());
  //I2C RTC
  Wire.begin();
  //SelecionaDataeHora();//Tem que ficar abaixo do Wire.begin é ativado apenas para mudar data e hora
  //==========================================================================================================
  //I2C OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setRotation(2);//Rotação do display.
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.clearDisplay();
  display.print(F("Robot One"));
  display.display();
  delay(delayRobotOne); display.clearDisplay();
  //==========================================================================================================
  //Verifica de tem SD-CARD
  while (!SD.begin(53)) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    display.println(F("Erro ao iniciar."));
    display.setCursor(35, 10);
    display.println(F("Desligue!"));
    display.setCursor(35, 20);
    display.print(F("98769.9288"));
    display.display();
    display.setTextSize(1);
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
  if (flag3) {
    WriteSDEE(horas,  minutos, segundos, diadomes, mes, ano, countAgua);//Passa os parametros para escrita no SD
  } flag3 = false;
  // Le os valores (data e hora) do modulo DS130
  WIRE(&segundos, &minutos, &horas, &diadasemana, &diadomes, &mes, &ano);
  //==========================================================================================================
  if (flagSD) {
    switch (diadasemana) {
      case 0: diadasemana2 = F("Domingo");
        break;
      case 1: diadasemana2 = F("Segunda");
        break;
      case 2: diadasemana2 = F("Terca");
        break;
      case 3: diadasemana2 = F("Quarta");
        break;
      case 4: diadasemana2 = F("Quinta");
        break;
      case 5: diadasemana2 = F("Sexta");
        break;
      case 6: diadasemana2 = F("Sabado");
        break;
    }
    //==========================================================================================================
    //Faz essa rotina ativar apenas uma vez a cada rest ou queda de luz
    uint16_t posEpron[256];

    if (flagEEpron) {
      //Rotina para saber se caiu energia ou mudou o dia
      if (EEPROM.read(255) != diadasemana) {
        zeraTudo();
      }
      EEPROM.update(255, diadasemana);//Salva o diadasemana na EEPROM
      for (int16_t i = 1; i < 255; i++) {//i==1 pois a posição zero esta reservada
        posEpron[i] = EEPROM.read(i);//Realizar a leitura dos endereços da EEPRON e guarda no vetor
        //Soma os vetores acima de zero em somaPosEEpron
        if (posEpron[i] > 0) {
          somaPosEEpron += posEpron[i];
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

    if (somaPosEEpron == limiteCont) {
      flagSD = 0x00;  //Para todo o processo e vai para erro() de contagem Maxima;
      erro();
    }
    //Fim Ultrasson
  }
  display2(somaPosEEpron, minutos,  horas,  diadasemana,  diadomes,  mes,  ano);//Imprimi Display
  //Serial.print(endEEpron); Serial.print(" - "); Serial.print(countAgua); Serial.print(" - "); Serial.println(somaPosEEpron);
 conexao();
}
//Fim do Loop
void conexao() {
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  if (client.connect(server, 80)) {
    Serial.println(F("connected"));
    // Make a HTTP request:
    client.println(F("GET /search?q=arduino HTTP/1.1"));
    client.println(F("Host: www.google.com"));
    client.println(F("Connection: close"));
    client.println();
  }
  else {
    // kf you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }
}


//==========================================================================================================
//----------------------------------------------------------------------------------------------------------
//Controle de EEPRON + SD escrita
void WriteSDEE(int16_t horas, int16_t minutos, int16_t segundos, int16_t diadomes, int16_t mes, int16_t ano, int16_t countAgua) {
  //if (SD.exists("LogData.txt")) SD.remove("LogData.csv"); //Apaga para atualizar a tabela
  arquivo = SD.open(F("LogData.txt"), FILE_WRITE);//escreve no SD
  arquivo.print(F("Quantidade: ")); arquivo.println(somaPosEEpron);
  arquivo.print(F("Horario: ")); arquivo.print(horas); arquivo.print(F(":")); arquivo.println(minutos);
  arquivo.print(F("Data: ")); arquivo.print(diadomes); arquivo.print(F("/")); arquivo.print(mes); arquivo.print(F("/")); arquivo.println(ano); arquivo.println(F("------------------------------------------------------"));
  arquivo.close();
  //Rotina que salva no primeiro endereço da EEPRON==1 quando countAgua é menor que 255
  if (countAgua > 0 && countAgua <= 255)EEPROM.put(endEEpron, countAgua); //Na posição zero ja tem que esta gravado a posição inicial de leitura
  if (countAgua == 255) {
    endEEpron++;
    EEPROM.put(0, endEEpron);//coloca a posição 0 o endereço da EEPRON atual
    countAgua = 0x00;
    somaPosEEpron = 0x00;
    EEPROM.put(endEEpron, countAgua);//Pra validar uma das condição if da EEPRON salva ZERO no proximo endereço
    flagEEpron = true;
  }
}
//==========================================================================================================
void SelecionaDataeHora() { //Seta a data e a hora do DS1307
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 24; //Valores de 0 a 59
  byte horas = 00; //Valores de 0 a 23
  byte diadasemana = 4; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 1; //Valores de 1 a 31
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
  display.print(F("Contagem"));
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 50);
  display.print(F("Maxima"));
  display.display();
}

void WIRE(int16_t* segundos, int16_t* minutos, int16_t* horas, int16_t* diadasemana, int16_t* diadomes, int16_t* mes, int16_t* ano) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 7);
  *segundos = ConverteparaDecimal(Wire.read());
  *minutos = ConverteparaDecimal(Wire.read());
  *horas = ConverteparaDecimal(Wire.read() & 0b111111);
  *diadasemana = ConverteparaDecimal(Wire.read());
  *diadomes = ConverteparaDecimal(Wire.read());
  *mes = ConverteparaDecimal(Wire.read());
  *ano = ConverteparaDecimal(Wire.read());
}
void display2(int16_t somaPosEEpron, int16_t minutos, int16_t horas, int16_t diadasemana, int16_t diadomes, int16_t mes, int16_t ano) {

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
  display.print(diadasemana2); display.print(F(" - "));
  if (diadomes < 10) {
    display.print(F("0"));
    display.print(diadomes);
  } else display.print(diadomes);
  display.print(F("/"));
  if (mes < 10) {
    display.print(F("0"));
    display.print(mes);
  } else display.print(mes);
  display.print(F("/")); display.print(ano);
  //==========================================================================================================
  //Hora
  display.setCursor(45, 50);
  if (horas < 10) {
    display.print(F("0"));
    display.print(horas);
  } else display.print(horas);
  display.print(F(":"));
  if (minutos < 10) {
    display.print(F("0"));
    display.print(minutos);
  } else display.print(minutos);
  /* display.print(":");
   if (segundos < 10) {
     display.print("0");
     display.print(segundos);
   } else display.print(segundos);*/
  display.display(); display.clearDisplay();
  //==========================================================================================================

}

void zeraTudo() {
  EEPROM.put(255, diadasemana);//Salva o diadasemana na EEPROM
  countAgua = 0x00;
  somaPosEEpron = 0x00;
  for (int16_t i = 0; i < 255; i++) {
    EEPROM.put(i, addr); //zera todos os endereços da EEPRON
  }
}

void ZERA(bool flag2) {
  if (flag2) {
    countAgua = 0x00;
    somaPosEEpron = 0x00;
    EEPROM.put(0, 1);//Inicializador que vai lembrar a ultima posição da contagem. So ativar uma vez e depois comentar
    for (int16_t i = 0; i < 255; i++) {
      EEPROM.put(i, addr);
    }
  }
}
void Contador() {
  countAgua++;
  somaPosEEpron++;
  flag3 = true;
}


