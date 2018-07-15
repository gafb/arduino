
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/
#include <Wire.h>
#include <SPI.h>
#include <stdlib.h>
#include <string.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;
char opcao = 'T';               /////////////////////////////

const int MPU = 0x68;
//Variaveis para armazenar valores dos sensores
int auxPiezo=0;
int auxGauge=0;
int auxUmidade;
int tempo;
//strcopy 
char pkt [50] = {};
char parteAcX[7], parteAcY[7], parteAcZ[7], parteGyX[7], parteGyY[7], parteGyZ[7], partePiezo[7], parteGauge[7], parteUmidade[7];
const int REED = 8;              //The reed switch outputs to digital pin 8
int val = 0;                    //Current value of reed switch
int old_val = 0;                //Old value of reed switch
int REEDCOUNT = 0;              //This is the variable that hold the count of switching
int mililitros = 0;

int confirma =1;

int auxAcX, auxAcY, auxAcZ, Tmp, auxGyX, auxGyY, auxGyZ;
int pkt1[3];
int tamanho;
int ACK=0;
int teste =0;
int estado =0;
int comp=0;
unsigned long x =0;//V=1234567890             // AUXILIAR DE CONFIRMACAO DE RECEBIMENTO DO PRIMEIRO DADO

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(9,10);
/**********************************************************/
boolean timeout = true;
byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 1;

void setup() {
  Serial.begin(9600);
    Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);

  //Inicializa o MPU-6050
  Wire.write(0);
  Wire.endTransmission(true);


  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  //radio.setPALevel(RF24_PA_LOW);
  radio.powerUp();
  radio.setDataRate(RF24_250KBPS);
  radio.enableDynamicPayloads();

  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  // Start the radio listening for data
  radio.startListening();
}

void Pluviometro()
{
    val = digitalRead(REED);      //Read the status of the Reed switch
if (val==HIGH){Serial.println("Tche");
}
  if ((val == LOW) && (old_val == HIGH)) {   //Check to see if the status has changed

    delay(10);                   // Delay put in to deal with any "bouncing" in the switch.
    //Serial.print("TCHAAAAAAA");
    REEDCOUNT = REEDCOUNT + 1;   //Add 1 to the count of bucket tips

    old_val = val;              //Make the old value equal to the current value^
    
    mililitros = REEDCOUNT * (5.65);
  }
  else {

    old_val = val;              //If the status hasn't changed then do nothing

  }
}
void Acelerometro_Giroscopio()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  //Solicita os dados do sensor
  Wire.requestFrom(MPU, 14, true);
  //Armazena o valor dos sensores nas variaveis correspondentes
  auxAcX = Wire.read() << 8 | Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  auxAcY = Wire.read() << 8 | Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  auxAcZ = Wire.read() << 8 | Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  auxGyX = Wire.read() << 8 | Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  auxGyY = Wire.read() << 8 | Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  auxGyZ = Wire.read() << 8 | Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

}

void Piezo_Gauge()
{
   auxPiezo = analogRead(A1);
  auxGauge = analogRead(A2); 
}
void Umidade() //verificar de tempo em tempo (nao eh necessario uma atualizacao rapida
{

  auxUmidade = analogRead (A0);
  
}

void EnviarDados()
{

    //Serial.println(F("Now sending"));
    timeout = false;
    
    unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete

    ACK=0; 
    while (ACK!= 1)
 {
 radio.stopListening();

  //ENVIO DOS VALORES
    radio.write( &pkt, sizeof(pkt));
       
    radio.startListening();                                    // Now, continue listening
    
    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
     timeout = false;                                   // Set up a variable to indicate if a response was received or not

    //ESPERA DE ACK, CASO DE TIMEOUT REENVIA O DADO
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at >300000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
 
    if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
    }
    else{  
         
     radio.read( &ACK, sizeof(int) );
   
      Serial.print("ACK: ");
      Serial.println (ACK);
    }

}

}
void loop() {
  

/****************** Ping Out Role ***************************/  
if (role == 1)  {
//Serial.println(F("Now sending"));
        
    radio.stopListening();                                    // First, stop listening so we can talk.

    Umidade(); // envia para as variaveis auxiliares(piezo,gauge,umidade) os valores corretos
   // tempo = micros();
    //Serial.println(tempo);
//Serial.print("Umidade2");
//Serial.println (auxUmidade);
    Piezo_Gauge();
  //  tempo = micros();
  //  Serial.println(tempo);
    Pluviometro();
  //  tempo = micros();
    Serial.println(tempo);
    Acelerometro_Giroscopio();
  //  tempo = micros();
  //  Serial.println(tempo);
    if (estado ==0)
    { 
                       // Take the time, and send it.  This will block until complete
      //EnviarDados(auxUmidade);      
        estado =1;
        
     } 
      
///-------------------------------UMIDADE-----------------------------------///
    else if (estado ==1){
     itoa (auxAcX, parteAcX, 10); //Conversao para string
     itoa (auxAcY, parteAcY, 10);
     itoa (auxAcZ, parteAcZ, 10);
     itoa (auxGyX, parteGyX, 10);
     itoa (auxGyY, parteGyY, 10);
     itoa (auxGyZ, parteGyZ,10);
     itoa (auxPiezo, partePiezo, 10);
     itoa (auxGauge, parteGauge,10);  
     itoa (auxUmidade, parteUmidade,10);
     strcat (parteAcX, "#");
     strcat (parteAcY , "#");
     strcat (parteAcZ, "#");
     strcat (parteGyX, "#");
     strcat (parteGyY , "#");
     strcat (parteGyZ, "#");
     strcat (partePiezo, "#");
     strcat (parteGauge, "#");

     
     strcat (pkt, parteAcX);
     strcat (pkt, parteAcY);
     strcat (pkt, parteAcZ);
     strcat (pkt, parteGyX);
     strcat (pkt, parteGyY);
     strcat (pkt, parteGyZ);
     strcat (pkt, partePiezo);
     strcat (pkt, parteGauge);
     strcat (pkt, parteUmidade);
     
     
     Serial.println("TESTANDO");
     Serial.println (pkt);

     
    EnviarDados();
    Serial.println("TESTE");

    }

}

} // Loop

