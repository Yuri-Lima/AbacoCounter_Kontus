#ifndef AGUA_H//Essas diretivas não deixarão as declarações/definições da biblioteca serem inseridas mais de uma vez em um projeto.
#define AGUA_H
 
#include <Arduino.h>

class ULTRA{
public: 
//Ultrassom
//=============================================================================================================  
  ULTRA( int _echoPin, int _trigPin);//Recebe os parametros das constantes
  int distancia();//Recebe as amostras lidas de algo Exemplo.: distancia, temperatura...
//=============================================================================================================
private:
//Ultrasson
//=============================================================================================================
 int echoPin;
 int trigPin;
 double duracao=0;
 double disT=0;
};

#endif
