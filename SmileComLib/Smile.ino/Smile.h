#ifndef SMILE_H//Essas diretivas não deixarão as declarações/definições da biblioteca serem inseridas mais de uma vez em um projeto.
#define SMILE_H
 
#include <Arduino.h>

class PID{
public:
//PID
//=============================================================================================================  
  PID(double _kP, double _kI, double _kD);//Recebe os parametros das constantes
  void addNewSample(double _sample);//Recebe as amostras lidas de algo Exemplo.: distancia, temperatura...
  void setSetPoint(double _setPoint);//Defini qual o offset/inicio
  double process();// retorna o valor calculado do PID
//=============================================================================================================
private:
//PID
//=============================================================================================================
  double setPoint;//Variavel que é acessadas e inicializadas pela função void setSetPoint
  double sample;//Variavel que é acessadas e inicializadas pela função void addNewSample
  double kP, kI, kD;//Variavel que é acessadas e inicializadas pelo construtor PID
  double error;//Variavel que é acessadas e inicializadas pela função void process
  double lastSample;//Variavel que é acessadas e inicializadas pela função void process     
  double P, I, D;//Variavel que é acessadas e inicializadas pela função void process
  double pid;//Variavel que é acessadas e inicializadas pela função void process
  long lastProcess;//Variavel que é acessadas e inicializadas pela função void process
//=============================================================================================================
};
class LED{
public: 
//LED RGB
//=============================================================================================================  
  LED(short int _reD,short int _bluE,short int _greeN);//Recebe os parametros das constantes
  void LigaTudo(short int _rgb);//Recebe as amostras lidas de algo Exemplo.: distancia, temperatura...
  void statusRed(short int _statusR);//Defini qual o offset/inicio
  void statusBlue(short int _statusB);//Defini qual o offset/inicio
  void statusGreen(short int _statusG);//Defini qual o offset/inicio
  void statusRGB();
//=============================================================================================================
private:
//LED RGB
//=============================================================================================================
 short int rgb;
 short int PinRed;
 short int PinBlue;  
 short int PinGreen;
};
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
 double duracao,soma;
 int disT,i;
};

class Ultrasonic
{
  public:
    Ultrasonic(int TP, int EP);
    long Timing();
    long Ranging(int sys);

    private:
    int Trig_pin;
    int Echo_pin;
    long  duration,distacne_cm,distance_inc;
    
};
 
#endif



