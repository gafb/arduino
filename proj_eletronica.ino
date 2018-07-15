#include <Servo.h>
//Projeto de eletrônica

Servo servo; // Variável Servo
int posServo; // Posição Servo

// LDR
byte luminosityPin = 4; //LDR no pino analógico 0
unsigned int luminosity = 0; //Valor lido do LDR 

byte emergencyLight = 13;

byte servoMotor = 6;

// Sensor de Temperatura
byte temperaturePin = 1;//Pino do sensor da temperatura
byte temperature;

byte MotorPin = 5;
unsigned int moisture=0;

enum luminosityLevel {
  dark,
  bright,
  ok
};

enum temperatureLevel {
  cold,
  hot,
  superHot,
  cool
};

enum MoistureLevel {
  dry,
  normal,
  wet
  };
 
void setup() {
  
  pinMode(MotorPin, OUTPUT);

  // pinos do buzzer
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(emergencyLight, OUTPUT);
  
  pinMode(7,OUTPUT);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  
  pinMode(A5, INPUT);

  pinMode(servoMotor, OUTPUT);
  servo.attach(servoMotor);
  servo.write(0);
   
  Serial.begin(9600); //Inicia a comunicação serial
}

void loop() {
    // ler o valor do LDR
    luminosity = analogRead(luminosityPin); // O valor lido será entre 0 e 1023

    moisture = analogRead(A5);

    // temperatura
    temperature = checkTemp();
    
    Serial.print("Temperatura: ");
    Serial.println(temperature);
    Serial.print("Umidade: ");
    Serial.println(moisture);

    bargraph(cool);
    analogWrite(MotorPin, 64);

    //Solo umido
    if (moisture > 0 && moisture < 400)
    {
        Serial.println(" Status: Solo umido");
        Serial.println(posServo);
         
        if(posServo > 80)
        {
          for(posServo = 90; posServo >= 0; posServo--)
          {
            servo.write(posServo);
            delay(15);
          }
        }  
    }
    //Solo seco
    if (moisture > 800 && moisture < 1024)
    {
        Serial.println(" Status: Solo seco");
        Serial.println(posServo);
        
        if(posServo < 10)
        {
          for(posServo = 0; posServo < 90; posServo++)
          {
            servo.write(posServo);
            delay(15);
          }
        }        
    }
    
    if(checkLuminosity(luminosity, ok) && checkTemperature(temperature, cool)){
      analogWrite(MotorPin, 64);// motor a 25%pwm;
      bargraph(cool);
       sirene(0);
    }
    
    // temperatura no amarelo
    if(checkTemperature(temperature, hot)){
      bargraph(hot);
      analogWrite(MotorPin, 128);// motor a 50%pwm
      sirene(8);
    }
    
    // temperatura faixa vermelha
    if(checkTemperature(temperature, superHot)){
      analogWrite(MotorPin, 255);// motor a 100%pwm
      bargraph(superHot);
      digitalWrite(emergencyLight,HIGH);      
      sirene(16);
    }
    
    if(checkLuminosity(luminosity, dark)){
      // acionar lâmpada de emergência
      digitalWrite(emergencyLight,HIGH);
      sirene(0);
    }
    else{
      digitalWrite(emergencyLight,LOW);
     digitalWrite(9,LOW);
    }
  
    delay(1000);
}

boolean checkLuminosity(int luminosity, luminosityLevel level){
  boolean ret = false;

  const int LUMINOSITY_LOW = 400;
  const int LUMINOSITY_HIGH = 100;

  if((luminosity > LUMINOSITY_LOW && level == dark) ||
     (luminosity < LUMINOSITY_HIGH && level == bright) ||
     (luminosity > LUMINOSITY_HIGH && luminosity < LUMINOSITY_LOW && level == ok)){
    ret = true;
  }
  
  return ret;
}

void bargraph(temperatureLevel level){
  if(level == cold){
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
  } else if(level == cool){
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, HIGH);
  } else if(level == hot){
      digitalWrite(2, LOW);
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
  } else if(level == superHot){
      digitalWrite(2, HIGH);
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
  }
}

boolean checkTemperature(int temperature, temperatureLevel level){
  boolean ret = false;

  const int COLD = 23;
  const int HOT = 25;
  const int SUPER_HOT = 28;

  if((temperature <= COLD && level == cold) ||
     (temperature >= SUPER_HOT && level == superHot) ||
     (temperature > COLD && temperature < HOT && level == cool) ||
     (temperature > HOT && temperature < SUPER_HOT && level == hot)){
    ret = true;
  }
  
  return ret;
}

void sirene(int p){
  for(int i = 0 ; i < p; i++){   
      digitalWrite(11,bitRead(i,3));
      digitalWrite(10,bitRead(i,2));
      digitalWrite(9,bitRead(i,1));
      digitalWrite(8,bitRead(i,0));
      delay(50);
  }
  for(int i = p ; i > 0; i--){  
      digitalWrite(11,bitRead(i,3));
      digitalWrite(10,bitRead(i,2));
      digitalWrite(9,bitRead(i,1));
      digitalWrite(8,bitRead(i,0));
      delay(50);
  }
}

void checkMoisture(int moisture, MoistureLevel level ){
boolean ret = false;

  const int DRY = 1023;
  const int NORMAL = 800;
  const int WET = 400;

   if((moisture >= DRY && level == DRY) ||
     (moisture <= WET && level == wet) ||
     (moisture < DRY && moisture > NORMAL) ||
     (moisture < NORMAL && moisture > WET && level == wet)){
    ret = true;
  }
  //return ret;
}

int checkTemp(){
  int myTemperature = 0;
  int conv = 0;
    for(int i = 0; i < 10; i++){
      int temperatureAnalogic = analogRead(temperaturePin);
      int conv = ( 5 * temperatureAnalogic * 100) / 1024;
      Serial.print("CONV ");
      Serial.println(conv);
      if(conv > 35 || conv < 15){
        conv = 28;
      }
      myTemperature += conv;
    } 
  return myTemperature / 10;
}

