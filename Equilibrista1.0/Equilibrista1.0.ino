//Equilibrista1.0.ino

class PID{
public:
	
	double error, sample, lastSample, kP, kI, kD, P, I, D, pid, setPoint;
	long lastProcess;
	
	PID(double _kP, double _kI, double _kD){
		kP = _kP;
		kI = _kI;
		kD = _kD;
	}
	
	void addNewSample(double _sample){
		sample = _sample;
	}
	
	void setSetPoint(double _setPoint){
		setPoint = _setPoint;
	}
	
	double process(){
		// Implementação P ID
		error = setPoint - sample;
		float deltaTime = (millis() - lastProcess) / 1000.0;
		lastProcess = millis();
		
		//P
		P = error * kP;
		
		//I
		I = I + (error * kI) * deltaTime;
		
		//D
		D = (lastSample - sample) * kD / deltaTime;
		lastSample = sample;
		
		// Soma tudo
		pid = P + I + D;
		
		return pid;
	}
};
//================================================================================
//Servo
#include <Servo.h>
Servo myservo;
//================================================================================
//================================================================================
//MPU6050
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"


//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

#define OUTPUT_READABLE_YAWPITCHROLL
// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}
//================================================================================
//================================================================================
//PID
float amostra=0;
float controlePwm  
 
PID meuPid(1.0, 0, 0.10);
//================================================================================

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

 
void setup() {
	Serial.begin(9600);
	//===============================================================================================================
	//Servo
	myservo.attach(12);
	//===============================================================================================================
	//===============================================================================================================
	//PID
	// join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    // initialize device
    Serial.println(F("Inicializando dispositivos I2C..."));
    mpu.initialize();

    // verify connection
    Serial.println(F("Testando dispositivos de conexao."));
    Serial.println(mpu.testConnection() ? F("MPU6050 conectado com sucesso.") : F("MPU6050 canexao falhou."));

    // Aguadando leitura
    Serial.println(F("\nEnvie um caracter para iniciar DMP. "));
    while (Serial.available() && Serial.read()); // Enquanto o buffer estiver vazio
    while (!Serial.available());                 // Aguardando algum dado
    while (Serial.available() && Serial.read()); // Enquanto o buffer estiver vazio novamente

    // Caregamento das configurações do DMP "Processamento de movimento digital"
    Serial.println(F("Inicializando DMP..."));
    devStatus = mpu.dmpInitialize();

    // Coloque os paramentro para definir os pontos que foram calibrado como Offset
    mpu.setXGyroOffset(40);
    mpu.setYGyroOffset(-12);
    mpu.setZGyroOffset(109);
    mpu.setZAccelOffset(1788);
    // Retorna 0 em caso de inicialização.
    if (devStatus == 0) {
        //Habilita o DMP depois de tudo ok.
        Serial.println(F("Habilitando DMP..."));
        mpu.setDMPEnabled(true);

        // Detactacao do pino de interrupcao do arduino
        Serial.println(F("Habilitando detectacao de interrupcao (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // Coloque true quando tudo ok, para entrar no loop
        Serial.println(F("DMP leitura! Aguardando a primeira interrupcao..."));
        dmpReady = true;

        // Guarda o pacote de byte para posterior comparação. Evitando um Overflow
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
    	if(devStatus==1){ Serial.println("Initial memory load failed") }
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Inicialização falhou (codigo "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
    //===============================================================================================================
    //===============================================================================================================
	pinMode(pSENSOR, INPUT);
	pinMode(pCONTROLE, OUTPUT);
}
 
int controlePwm = -12;
 
void loop() {

	 // if programming failed, don't try to do anything
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
        // other program behavior stuff here
        // .
        // .
        // .
        // if you are really paranoid you can frequently test in between other
        // stuff to see if mpuInterrupt is true, and if so, "break;" from the
        // while() loop to immediately process the MPU data
        // .
        // .
        // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
        
        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

            amostra = ypr[1] * 180/M_PI;
            // Manda pro objeto PID!
            meuPid.addNewSample(amostra);
            // Converte para controle
			controlePwm = (meuPid.process() - (-12));
            controlePwm=map(controlePwm, -90, 90, 0, 180);
            myservo.write(controlePwm);
            Serial.println(controlePwm);
            Serial.print("\t");
            //Serial.println(gap);
            //Serial.print("ypr\t");
            //Serial.print(ypr[0] * 180/M_PI);
            //Serial.print("\t");
            //Serial.print(ypr[1] * 180/M_PI);
            //Serial.print("\t");
            //Serial.println(ypr[2] * 180/M_PI);
        #endif
    }

}