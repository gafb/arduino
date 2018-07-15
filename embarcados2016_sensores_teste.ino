//Carrega a biblioteca Wire
#include <Wire.h>
#include <SD.h>
#include <TimerOne.h>

File myFile;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//                                                                          Variaveis globais                                                                              //
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

//Endereco I2C do MPU6050
const int MPU = 0x68;
//Variaveis para armazenar valores dos sensores
int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
const int REED = 8;              //The reed switch outputs to digital pin 8

int val = 0;                    //Current value of reed switch

int old_val = 0;                //Old value of reed switch

int REEDCOUNT = 0;              //This is the variable that hold the count of switching

int mililitros = 0;
int umidade = 0;

int piezo = 0;
int gauge = 0;

int aux =0;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//                                                        SETUP - Vai executar apenas uma vez                                                                              //
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

void setup()
{
  // Some time to open serial monitor
  delay(3000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print("Initializing SD card...");


  Serial.begin(9600);
  pinMode (REED, INPUT_PULLUP); //This activates the internal pull up resistor (to pluviometer)

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);

  //Inicializa o MPU-6050
  Wire.write(0);
  Wire.endTransmission(true);


//Timer1.initialize(1000000); //micros
//Timer1.attachInterrupt(escrita);
}


void acelerometro_giroscopio()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  //Solicita os dados do sensor
  Wire.requestFrom(MPU, 14, true);
  //Armazena o valor dos sensores nas variaveis correspondentes
  AcX = Wire.read() << 8 | Wire.read(); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

}

void piezo_gauge()
{

  piezo = analogRead(A1);
  gauge = analogRead(A2);

}

void umidadeSolo() //verificar de tempo em tempo (nao eh necessario uma atualizacao rapida
{

  umidade = analogRead (A0);

}

void escrita()
{
  system("touch /media/card/dados.txt");

  
  myFile = SD.open ("dados.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to dados.txt...\n");
  
  
    myFile.print("mililitros= ");

    myFile.println(mililitros);

    myFile.print("piezo= ");
    myFile.println(piezo);

    myFile.print("gauge= ");
    myFile.println(gauge);

    myFile.print("umidadeSolo= ");
    myFile.println(umidade);

    myFile.print("AcX= ");
    myFile.print(AcX);

    myFile.print(" AcY= ");
    myFile.print(AcY);

    myFile.print(" AcZ= ");
    myFile.print(AcZ);

    myFile.print(" GyX= ");
    myFile.print(GyX);

    myFile.print(" GyY= ");
    myFile.print(GyY);

    myFile.print(" GyZ= ");
    myFile.println(GyZ);

    //myFile.flush();
    // close the file:
    
    myFile.close();

  //  Serial.println("done.");  
aux= aux+1;  
Serial.println(aux); //para verificar quantidade de ciclos
/*
  Serial.print ("Mililitros: "); //
  Serial.println(mililitros);


  Serial.print ("Umidade do solo: "); //
  Serial.println(umidade);

  Serial.print ("Piezo: "); //
  Serial.println(piezo);

  Serial.println ("Gauge: "); //
  Serial.println(gauge);

  //Envia valor X do acelerometro para a serial e o LCD
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  //Envia valor Z do acelerometro para a serial e o LCD
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | GyX = "); Serial.print(GyX);
  //Envia valor Y do giroscopio para a serial e o LCD
  Serial.print(" | GyY = "); Serial.print(GyY);
  //Envia valor Z do giroscopio para a serial e o LCD
  Serial.print(" | GyZ = "); Serial.println(GyZ);
*/
  } 
  
  else {
    // if the file didn't open, print an error:
    Serial.println("error (opening dados.txt)");
  }
  
}
void loop()
{

  val = digitalRead(REED);      //Read the status of the Reed switch


  if ((val == LOW) && (old_val == HIGH)) {   //Check to see if the status has changed

    delay(10);                   // Delay put in to deal with any "bouncing" in the switch.

    REEDCOUNT = REEDCOUNT + 1;   //Add 1 to the count of bucket tips

    old_val = val;              //Make the old value equal to the current value^

    Serial.print("Count = ");

    Serial.println(REEDCOUNT);  //Output the count to the serial monitor
    mililitros = REEDCOUNT * (5.65);
  }
  else {

    old_val = val;              //If the status hasn't changed then do nothing

  }

  //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
  //                                                        A cada tempo de atualizacao executa                                                                              //
  //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

  acelerometro_giroscopio();

  piezo_gauge();

  umidadeSolo();

  escrita();
}
