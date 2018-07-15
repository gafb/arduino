/*=====================================AUTOGUARD - Projeto Competição de Embarcados 2017=======================================================
||       Componentes: Gabriel Augusto                                                                                                         ||
||                   Gabriel Gadelha                                                                                                          ||
||                    Igor Carneiro                                                                                                           ||
||                                                                                                                                            ||
|| O que já foi desenvolvido:                                                                                                                 ||
|| Manuseio básico de todos os sensores, controle remoto do carro via Celular                                                                 ||
||                                                                                                                                            ||
||                                                                                                                                            ||
|| O que funciona nesse código:                                                                                                               ||
|| Módulo SD, acelerômetro e giroscópio, sensor de impacto e vibração                                                                         ||
|| Para salvar os dados dos sensores para possível calibração                                                                                 ||
||                                                                                                                                            ||
||                                                                                                                                            ||
|| Ressalvas:                                                                                                                                 ||
|| Verificar melhor valores do giroscópio                                                                                                     ||
||                                                                                                                                            ||
|| Não tão bem calibrado, faltando análise com o módulo SD e movimentação via bluetooth                                                       ||
||    -Colocar um arduino nano para separar o módulo bluetooth do SD para testes                                                              ||
||                                                                                                                                            ||
|| Problemas com o peso do carro, possíveis soluções:                                                                                         ||
||   - Trocar bateria, para uma mais leve ou uma que forneça maior potência aos motores                                                       ||
||                                                                                                                                            ||
|| Lembrando que o módulo SD é apenas para análise de dados, sendo retirado do circuito final                                                 ||
||                                                                                                                                            || 
|| Problemas para amarrar sensores:                                                                                                           ||
||   -  ???                                                                                                                                   ||
||==============================================================================================================================================*/

#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 11, TXPin = 12;
//Objeto TinyGPS++
TinyGPSPlus gps;

//Conexao serial do modulo GPS
SoftwareSerial Serial_GPS(RXPin, TXPin);

File myFile;
//Ajuste o timezone de acordo com a regiao
const int UTC_offset = -3;
const int chipSelect = 4;

#define LIMITEACC 2
#define LIMITEGIR 100
long Tmp;
float AcX, AcY, AcZ, varAcX = 0, varAcY = 0, varAcZ = 0;
float vecAcX[10], vecAcY[10], vecAcZ[10];
float rotX, rotY, rotZ;
const int MPU = 0x68;
char k = 0;
bool sensorVibracao, sensorImpacto;
bool funcionamentoGPS = 0, requisicaoGPS = 1;
int acidente = 0;
bool botaoSeguro, botaoAcidente;

bool nivelAgua, gas;
int gasAnalogico;

int pinoGravador = 6;
int tempoGravador = 0;
int intervaloGravador = 10000;
int tempoAtualGravador = 0;
int repeticoesGravador = 0;
boolean contaGravacao = false;
boolean reproduzindoGravacao = false;//Primeira iteração do loop que marca o inicio de uma reproducao de mensagem

void setup() {
  /*Declaração dos pinos digitais*/
  pinMode(3, INPUT);
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);

  Serial.begin(9600);
  Serial_GPS.begin(9600);

  /*      Chamada de configuração do MPU6050  ACC-GIR*/
  Wire.begin();
  setupMPU();
}

 /*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                                                      Ciclo básico do programa                                                     ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/

void loop() {
 

  if (!acidente) {
    //acelerometro_giroscopio(); //Obtém valores do Acc, Gir e Tmp
    //acidente_acelerometro();  //Detecta acidente a partir do Acc, Gir e Tmp
    vibracaoImpacto();        //Obtém valor do sensor de vibração e de impacto
    //Serial.println("Tche");
    //repeticoesGravador =0;
    verificacaoGPS(); //Sincroniza gps com satelites

  }
  else {
    if (acidente == 1 && botaoSeguro) { //Caso botão seguro não esteja pressionado e os sensores tenham detectado um acidente

      repeticoesGravador = 0;
      acidente = mod_voz();
      //gas_nivelAgua();


    }
    else if (acidente == 3 || acidente == 2) { //Caso não haja resposta a partir do módulo de voz ou seja confirmado o acidente pelo botãoAcidente
      gas_nivelAgua();
    }

    //----------------------------------------------------->Enviar dados que já possui
    //----------------------------------------------------->Checar outros sensores se necessario

  }
  botaoAcidente = digitalRead(8);
  botaoSeguro = digitalRead(9);
  if (!botaoSeguro) { //Se botãoSeguro pressionado não houve acidente
    Serial.println (acidente);
    acidente = 0;
    //Para de reproduzir gravação de voz
    digitalWrite(pinoGravador, LOW);
  }

  if (!botaoAcidente) { //Se botão do Acidente pressionado então é considerado que houve um acidente ou pedido de socorro do passageiro
    Serial.println(acidente);
    acidente = 3;
    //Para de reproduzir gravação de voz
    digitalWrite(pinoGravador, LOW);
  } //----------------------------------------------->Envio de pedido de socorro



}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                           Config. MPU, range dos sensores e endereços necessários para acesso                                     ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
void setupMPU() {
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000); //Setting the accel to +/- 2g //Bit 3 e 4
  Wire.endTransmission();
}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                           Máximo de um intervalo do acelerômetro em determinado eixo                                              ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
float getMaxAcc(float vecAcc[]) {
  float maxAcc = -5;
  for (int i = 0; i < 10; i++)
    if (vecAcc[i % 10] > maxAcc) {
      maxAcc = vecAcc[i % 10];
    }
  return maxAcc;
}
/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                           Mínimo de um intervalo do acelerômetro em determinado eixo                                              ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
float getMinAcc(float vecAcc[]) {
  float minAcc = 5;
  for (int i = 0; i < 10; i++)
    if (vecAcc[i % 10] < minAcc) {
      minAcc = vecAcc[i % 10];
    }
  return minAcc;
}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                                                  Leitura dos sensores ACC, GIR, TMP                                               ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
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
  rotX = Wire.read() << 8 | Wire.read(); //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  rotY = Wire.read() << 8 | Wire.read(); //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  rotZ = Wire.read() << 8 | Wire.read(); //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  processAccGir();
}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                                                  Alteração do range dos sensores                                                  ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
void processAccGir() {
  AcX = AcX / 16384.0;
  AcY = AcY / 16384.0;
  AcZ = AcZ / 16384.0;
  Tmp = (Tmp / 340) + 36.53; //Fórmula para temperatura do acelerometro
  rotX = rotX / 131.0;
  rotY = rotY / 131.0;
  rotZ = rotZ / 131.0;
  //Serial.println(rotX);
  //Serial.println(rotY);
  // Serial.println(rotZ);

  if ((abs(rotX)) > LIMITEGIR || (abs(rotY)) > LIMITEGIR || (abs(rotZ)) > LIMITEGIR) {

    acidente = 1;
  }

}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                               Detecção de acidente a partir da variação do ACC em um determinado intervalo                        ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
void acidente_acelerometro() {
  vecAcX[k] = AcX;
  vecAcY[k] = AcY;
  vecAcZ[k] = AcZ;
  if (k == 10) {
    varAcX = (getMaxAcc(vecAcX) - getMinAcc(vecAcX)); //Pd ser colocado direto do IF abaixo, mas sera mantido para melhorar legibilidade
    varAcY = (getMaxAcc(vecAcY) - getMinAcc(vecAcY));
    varAcZ = (getMaxAcc(vecAcZ) - getMinAcc(vecAcZ));

    //Serial.println(varAcX);
    if (varAcX > LIMITEACC && varAcY > LIMITEACC && varAcZ > LIMITEACC) {
      acidente = 1;
    }
    k = 0;

  }
  else {
    k++;
  }


}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                                           Coleta dos sensores de vibração e impacto                                               ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
void vibracaoImpacto() //Mudar tipo de retorno, caso esta funcao deva indicar a presenca de uma vibracao.

{
  sensorVibracao = digitalRead(3); //Fica em 1 quando agitado
  sensorImpacto = digitalRead(2);

  if (!sensorImpacto) {
    acidente = 1;



    //Fica em 0 qnd pressionado
  }
}

/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                                          Sincronização do GPS                                                                     ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
void verificacaoGPS() {
  if (Serial_GPS.available() > 0) { //Se há informação será lida

    if (gps.encode(Serial_GPS.read()))
      displayInfo();

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      Serial.println(F("GPS nao detectado, olhe os fios."));
      while (true);
    }
    GPS_Timezone_Adjust();
  }

}

void displayInfo()
{
  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
    //Entra se os dados forem validos
    Serial.print(gps.location.lat(), 6); //latitude
    Serial.print(F(","));
    Serial.println(gps.location.lng(), 6); //longitude

    Serial.print(gps.date.day()); //dia
    Serial.print(F("/"));
    Serial.print(gps.date.month()); //mes
    Serial.print(F("/"));
    Serial.println(gps.date.year()); //ano
    Serial.println(acidente);///////////////////////////////////////////////////
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour()); //hora
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute()); //minuto
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second()); //segundo
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.println(gps.time.centisecond());
  }
  else if (gps.date.isValid()) {
    Serial.print(F("Data sincronizada: "));
    Serial.print(gps.date.day()); //dia
    Serial.print(F("/"));
    Serial.print(gps.date.month()); //mes
    Serial.print(F("/"));
    Serial.println(gps.date.year()); //ano
  }
  else {
    Serial.println(F("Tentando sincronizar com os satelites"));
  }


}

void GPS_Timezone_Adjust()
{
  if (Serial_GPS.available())
  {
    if (gps.encode(Serial_GPS.read()))
    {
      int Year = gps.date.year();
      byte Month = gps.date.month();
      byte Day = gps.date.day();
      byte Hour = gps.time.hour();
      byte Minute = gps.time.minute();
      byte Second = gps.time.second();

      //Ajusta data e hora a partir dos dados do GPS
      setTime(Hour, Minute, Second, Day, Month, Year);
      //Aplica offset para ajustar data e hora
      //de acordo com a timezone
      adjustTime(UTC_offset * SECS_PER_HOUR);
    }
  }
}


void gas_nivelAgua() {
  nivelAgua = digitalRead(4);
  gas = digitalRead(7);
  // Le os dados do pino analogico A0 do sensor
  gasAnalogico = analogRead(A0);
  // Mostra os dados no serial monitor
  //Serial.print("Pino D0 : ");
  //Serial.print(gas);
  Serial.print(" Pino A0 : ");
  Serial.println(gasAnalogico);
  //Serial.println(nivelAgua);
}
/*=======================================================================================================================================
  ||                                                                                                                                   ||
  ||                                          Iterações de reprodução de voz em caso de acidente                                       ||
  ||                                                                                                                                   ||
  =====================================================================================================================================*/
int mod_voz() {

  reproduzindoGravacao = false;
  while (repeticoesGravador < 4 && acidente == 1) { //Qtd de gravacoes < 4
    botaoAcidente = digitalRead(8);
    botaoSeguro = digitalRead(9);
    if (!botaoAcidente) { //Botao de pedido de socorro ativado
      repeticoesGravador = 5; // Para sair do loop
      digitalWrite(pinoGravador, LOW);
      acidente = 3;
      break;
    }
    else if (!botaoSeguro) { //Botao de que esta tudo bem foi pressionado
      repeticoesGravador = 5; //Para sair do loop e vai zerar o valor fora do loop
      acidente = 0;
      digitalWrite(pinoGravador, LOW);
      break;
    }

    if (reproduzindoGravacao = false)//Inicio de reproducao de mensagem
    {
      tempoGravador = millis();
      tempoAtualGravador = millis();
      reproduzindoGravacao = true;
    }

    //Calcula se o intervalo de tempo entre uma reprodução e outra já foi atingido
    if (tempoAtualGravador - tempoGravador < intervaloGravador)
    {
      tempoAtualGravador = millis();
      digitalWrite(pinoGravador, HIGH);
    }
    else
    {
      reproduzindoGravacao = false;
      contaGravacao = false;
    }

    //Seta o pino para LOW, preparando para a próxima reprodução
    if (tempoAtualGravador - tempoGravador > 5000 && contaGravacao == false)
    {

      digitalWrite(pinoGravador, LOW);
      repeticoesGravador++;
      Serial.print("Acidente: ");
      Serial.println(acidente);
      tempoGravador = millis(); //Reatualizando tempo do gravador
      Serial.println(repeticoesGravador);
      contaGravacao == true;

    }

  }

  //Retorna 2 se nao houve resposta
  if (!botaoAcidente) {
    digitalWrite(pinoGravador, LOW);
    return 3;
  }
  else if (!botaoSeguro) {
    digitalWrite(pinoGravador, LOW);
    return 0;
  }
  else {
    return 2;
  }
}
