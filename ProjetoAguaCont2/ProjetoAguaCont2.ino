#include "Agua.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS1307.h>
#include <Ethernet.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//================================================
//Ethernet
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 25, 177);
EthernetServer server(80);//(port 80 is default for HTTP):
boolean arrayEstado[1];//Se true entrar para acionar as cargas
byte estadoip = 0x00;//Variavel de teste. Se conexão interna ou externa
//================================================
//Strings
String diadasemana2;
//================================================
//EEPROM
#define addr 0x00
unsigned int somaPosEEpron = 0x00;//Guarda o somatorio dos numeros guardados no endereços da EEPRON
unsigned int endEEpron = 0x01, A = 0x00;//Guarda os ultimos endereços da EEPRON na posição ZERO
unsigned int posEpron[255];
boolean flagEEpron = true, flagEndPron = true, flag2 = true;
//================================================
//OLED
#define OLED_RESET 8
#define delayRobotOne 1000//Tempo de visualização no display
#define delayCartaoIniciado 1000//Tempo de visualização no display
Adafruit_SSD1306 display(OLED_RESET);
//================================================
//RTC
#define DS1307_ADDRESS 0x68// Modulo RTC no endereco 0x68
#define timeERRO 20//Limita o tempo para zerar tudo!
byte lastHoras = 0;//Guarda a ultima leitura da hora
byte horaZero = 23;//Horario para a variavel contAgua ZERAR
byte zero = 0x00;//Serve para recpção de dados no CI do RTC
//================================================
//Sd Card
File arquivo;//Objeto da classe File
char* cleanCel = "";
byte flagSD = 0x01;//Flag de validação do SD-CARD
boolean flagWriteSDEE = true;
//================================================
//Contadores
#define limiteCont 0xFA00//Limita a contagem(Padrão 64000)
byte count = 0x00; // Contador do filtro para sensibilidade do ultrasson na detecção de objetos
byte flagAgua = 0x00; //Sinalizador de estado logico alto
byte countAgua = 0x00; //Contador de agua limite de 65.535 2 bytes
//================================================
//Ultrasson
#define TRIG 2
#define ECO 3
#define detecMax 50 //Limite de detecção em 50 cm
#define filtro 20 //Define a quantidade minima de leituras para distinguir um objeto
#define timeFiltro 10 //Intervalos entre leituras que vai influenciar no filtro
ULTRA ultra(ECO, TRIG); //Objeto da classe ultra
EthernetClient client;
void setup() {
  //Serial.begin(9600);
  //I2C
  short e;
  Wire.begin();
  //SelecionaDataeHora();//Tem que ficar abaixo do Wire.begin é ativado apenas para mudar data e hora
  //==========================================================================================================
  //Ethernet
  Ethernet.begin(mac, ip);
  //server.begin();
  //==========================================================================================================
  //OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.clearDisplay();
  display.print("Robot One");
  display.display();
  delay(delayRobotOne); display.clearDisplay();
  //==========================================================================================================
  //Verifica de tem SD-CARD
  while (!SD.begin(4)) {
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
    //return;
  }
    display.clearDisplay();
    flagSD = 0x01;
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.clearDisplay();
    display.print("Cartao SD Iniciado");
    display.display();
    delay(delayCartaoIniciado); display.clearDisplay(); display.clearDisplay();
  
}
//Fim Setup

void loop() {
  
  //Inicio RTC
  //==========================================================================================================
  //----------------------------------------------------------------------------------------------------------
  // Le os valores (data e hora) do modulo DS1307
  int segundos, minutos, horas, diadasemana, diadomes, mes, ano;
  WIRE(&segundos, &minutos, & horas, &diadasemana, &diadomes, &mes, &ano);
  //==========================================================================================================
  if (flagSD != 0x01) {
    // Le os valores (data e hora) do modulo DS1307
    WIRE(&segundos, &minutos, & horas, &diadasemana, &diadomes, &mes, &ano);
    erro(segundos);
  }
  if (flagSD != 0x00) {
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
    //Inicio Ethernet
    //==========================================================================================================
    //----------------------------------------------------------------------------------------------------------
    
    //Rotina para saber se caiu energia ou mudou o dia
    EthernetClient client = server.available();   // Verifica se tem alguém conectado
    if (client) {
      boolean currentLineIsBlank = true;       // A requisição HTTP termina com uma linha em branco Indica o fim da linha
      String valPag;                           //Varialvel que vai receber a concat de c
      while (client.connected()) {
        if (client.available()) {              //Esperando dados
          char c = client.read();              //Variável para armazenar os caracteres que forem recebidos
          //Serial.write(c);
          valPag.concat(c);                    // Pega os valor após o IP do navegador ex: 192.168.1.2/0001

          //Compara o que foi recebido
          if (valPag.endsWith("interno")) {    //Acesso interno da rede
            estadoip = 1;
          }
          if (valPag.endsWith("externo")) {    //Acesso externo da rede
            estadoip = 2;
          }
          if (valPag.endsWith("0001")) {       //Se o que for pego após o IP for igual a 0001
            arrayEstado[0] = !arrayEstado[0];  //Inverte o estado do segundo butao "Luz Garangem"
          }
          //=========================================================================================================================
          if (c == '\n' && currentLineIsBlank) {
            //Inicia página HTML
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.print("<HTML> ");
            client.println("<center> <a href='http://www.RobotOne.com.br\'> <img src=http://oi60.tinypic.com/2qsvyfp.jpg width 160 height=120></a>");
            //client.print("<center><h2 style='color: red; font-size:30px; text-align:center;'> ATEN&Ccedil&AtildeO! </h2>");
            //client.print("<center><h4> Pessoal que acender a luz, por favor apague, se perceberem que a luz est&aacute apagando &eacute pq sou eu que apago!</h4>");
            client.println("<center> <a href='http://www.RobotOne.com.br\'> <img src=http://oi58.tinypic.com/qwztic.jpg width 750 height=350></a>");
            client.println("<p style='font-size:15'>by  Yuri Lima"); client.print(" - ");
            client.print(diadasemana2); client.print(" - ");
            if (diadomes < 10) {
              client.print("0");
              client.print(diadomes);
            } else client.print(diadomes);
            client.print("/");
            if (mes < 10) {
              client.print("0");
              client.print(mes);
            } else client.print(mes);
            client.print("/"); client.print(ano); client.print(" - ");
            if (horas < 10) {
              client.print("0");
              client.print(horas);
            }else client.print(horas);
            client.print(":");
            if (minutos < 10) {
              client.print("0");
              client.print(minutos);
            } else client.print(minutos);
            //=========================================================================================================================
            //BOTAO ZERAR
            client.print("<BR>");
            estadoip == 1 ?
            client.print("<center> <button onclick=\"window.location.href='http://192.168.25.177/interno/0001'\">\0</button> > Codigo: 1011 > ") :
            client.print("<center> <button onclick=\"window.location.href='http://arduinoyuri.dyndns.org/externo/0001'\">\0</button> > Codigo: 1011 >");

            if (arrayEstado[0]) {
              countAgua = 0x00;
              somaPosEEpron = 0x00;
              EEPROM.write(0, 1);//Inicializador que vai lembrar a ultima posição da contagem. So ativar uma vez e depois comentar
              for (int i = 0; i < 255; i++) EEPROM.write(i, addr);
              client.print("<B><span style=\"color: #000000;\">");
              client.print("As garrafas foram zeradas - ");
              client.print("</span></B></left>");
              client.print("<B><span style=\"color: #00ff00;\">");
              client.print("ON");
              client.print("</span></B></center>");
            }
            else {
              client.print("<B><span style=\"color: #000000;\">");
              client.print("Zere a contagem aqui - ");
              client.print("</span></B></left>");
              client.print("<B><span style=\"color: #ff0000;\">");
              client.print("OFF");
              client.print("</span></B></center>");
            }
            //=========================================================================================================================
            //Quantidade de Garrafas
            client.print("<BR>");
            estadoip == 1 ?
            client.print(" <center> <style='font-size:20px; 'button onclick=\"window.location.href='http://192.168.25.177/interno'\">\0</button> > Codigo: 1110 > ")
            : client.print("<center> <style='font-size:20px; 'button onclick=\"window.location.href='http://arduinoyuri.dyndns.org/externo'\">\0</button> > Codigo: 1110 >");
            client.print("<B><span style=\"color: #000000;\">");
            client.print(" QUANTIDADE DE GARRAFAS: ");
            client.print("<font size=4>");
            client.println(somaPosEEpron);
            client.print("<B><span style=\"color: #FF0000;\">");
            //client.print("*C </font></font></center>");
            client.print("</span></B></center>");
            client.print("</span></B></center>");
            //=========================================================================================================================
            client.print("<BR>");
            if (estadoip == 1) {
              client.print(" <meta http-equiv=\"refresh\" content=\"3; url=http://192.168.25.177/interno \"> ");
            }
            if (estadoip == 2)  {
              client.print(" <meta http-equiv=\"refresh\" content=\"3; url=http://arduinoyuri.dyndns.org/externo \"> ");
            }
            client.println("</HTML>");
            break;
          } //Fecha if (c == '\n' && currentLineIsBlank)
        } //Fecha if (client.available())
      } //Fecha While (client.connected())
      delay(3);// Espera um tempo para o navegador receber os dados
      client.stop(); // Fecha a conexão
    } //Fecha if(client)
    //Fim Ethernet
    //----------------------------------------------------------------------------------------------------------
    //==========================================================================================================
    //Inicio EEPROM Energia
    //==========================================================================================================
    //-----------------------------------------------------------------------------------------------------------
    //Rotina para saber se caiu energia ou mudou o dia
    if (horas == 23) {
      countAgua = 0x00;
      for (int i = 0; i < 255; i++) EEPROM.write(i, addr); //zera todos os endereços da EEPRON
    }
    //==========================================================================================================
    //----------------------------------------------------------------------------------------------------------
    //Iniciadores true para zerar tudo
    flag2 = false;
    if (flag2) {
      EEPROM.write(0, 1);//Inicializador que vai lembrar a ultima posição da contagem. So ativar uma vez e depois comentar
      for (int i = 0; i < 255; i++) EEPROM.write(i, addr);
    }
    //----------------------------------------------------------------------------------------------------------
    //==========================================================================================================
    if (flagEEpron == true) {//Faz essa rotina ativar apenas uma vez a cada rest ou queda de luz
      for (int i = 1; i < 255; i++) {//i==1 pois a posição zero esta reservada
        posEpron[i] = EEPROM.read(i);//Realizar a leitura dos endereços da EEPRON e guarda no vetor
        //Serial.println(posEpron[i]);
      }
      for (int i = 1; i < 255; i++) {
        if (posEpron[i] > 0) {
          somaPosEEpron += posEpron[i];
          //Serial.print("i= "); Serial.print(i); Serial.print(" - "); Serial.print("endEEpron2= ");
          //Serial.println(EEPROM.read(0)); endEEpron = i; //Lembra onde o endereço que parou a contagem
        }//Soma os vetores acima de zero em x

        if (posEpron[i] > 0 && posEpron[i] < 255) {
          countAgua = posEpron[i];
          endEEpron = i;
        }
        if (posEpron[i] == 0 && EEPROM.read(i - 1) == 255) {
          countAgua = 0;
          endEEpron = i;
        }
      }
    } flagEEpron = false;
    //Serial.print(endEEpron); Serial.print(" - "); Serial.print(countAgua); Serial.print(" - "); Serial.println(somaPosEEpron);
    //Fim EEPRON
    //----------------------------------------------------------------------------------------------------------
    //==========================================================================================================
    //Inicio Ultrasson
    //==========================================================================================================
    //----------------------------------------------------------------------------------------------------------
    //Acionamento de contagem pelo ultrasson
    flagAgua = 0x00;//flag de verificações
    while (ultra.distancia() < detecMax && ultra.distancia() != 999) {
      count++;//até atingir uma quantidade de leituras pre estabelecida
      if (count > filtro) flagAgua = 0x01; //flag de verificações
    }
    if (count > filtro && flagAgua == 0x01 && ultra.distancia() == 999) {
      somaPosEEpron++;
      countAgua++;//conta as garrafas
      if (somaPosEEpron == limiteCont)flagSD = 0x00;//Para todo o processo e vai para erro();
      WriteSDEE(horas,  minutos, segundos, diadomes, mes, ano, countAgua);//Passa os parametros para escrita no SD
    }
    count = 0x00;//zera o filtro
    //Fim Ultrasson
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
    else if (somaPosEEpron > 999) { //Deslocamento dos numeros no display
      //==========================================================================================================
      display.setTextSize(4);
      display.setTextColor(WHITE);
      display.setCursor(25, 0);
      display.clearDisplay();
      display.print(somaPosEEpron);
    }
    //==========================================================================================================
    else if (somaPosEEpron > 9999) { //Deslocamento dos numeros no display
      //==========================================================================================================
      display.setTextSize(3);
      display.setTextColor(WHITE);
      display.setCursor(30, 0);
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
}
//Fim do Loop
//==========================================================================================================
//----------------------------------------------------------------------------------------------------------
//Controle de EEPRON + SD escrita
void WriteSDEE(int horas, int minutos, int segundos, int diadomes, int mes, int ano, int countAgua) {
  if (flagWriteSDEE == true) {
    arquivo = SD.open("LogData.csv", FILE_WRITE);//escreve no SD
    if (horas - lastHoras == 1)arquivo = SD.open("LogHora.csv", FILE_WRITE); //escreve no SD
    arquivo.seek(0x00);
    arquivo.print("Quantidade: ");
    arquivo.println(somaPosEEpron);
    arquivo.print("Horario: ");
    arquivo.print(horas); arquivo.print(":"); arquivo.println(minutos);
    arquivo.print("Data: "); arquivo.print(diadasemana2); arquivo.print("  --  ");
    arquivo.print(diadomes); arquivo.print("/"); arquivo.print(mes); arquivo.print("/"); arquivo.print(ano);
    arquivo.close();
    lastHoras = horas;//Guarda a ultima hora para depois gravar no LogHora de hora em hora
    //Rotina que salva no primeiro endereço da EEPRON==1 quando x é menor que 255
    if (countAgua <= 255  && countAgua > 0)EEPROM.write(endEEpron, countAgua);//Na posição zero ja tem que esta gravado a posição inicial de leitura
    if (countAgua > 254) {
      endEEpron++;
      EEPROM.write(0, endEEpron);//coloca a posição j==1 no endereço 0 da EEPRON
      countAgua = 0x00;
      somaPosEEpron = 0x00;
      EEPROM.write(endEEpron, countAgua);//Pra validar uma das condição if da EEPRON salva ZERO no proximo endereço
      flagEEpron = 1;
    }// Serial.print(endEEpron); Serial.print(" - "); Serial.print(countAgua); Serial.print(" - "); Serial.println(somaPosEEpron);
  }
}

//==========================================================================================================
void SelecionaDataeHora() { //Seta a data e a hora do DS1307
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 39; //Valores de 0 a 59
  byte horas = 16; //Valores de 0 a 23
  byte diadasemana = 3; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 19; //Valores de 1 a 31
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
void erro(int segundos) {
  int F = 0, somaSegundos = 0;
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
void zeraTUDO() {
  EEPROM.write(0, 1);//Inicializador que vai lembrar a ultima posição da contagem. So ativar uma vez e depois comentar
  for (int i = 0; i < 255; i++) EEPROM.write(i, addr);
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
