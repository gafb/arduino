/*
* O max e o min seriam para plotagem dos gráficos, sendo cosiderados em tempo real para transmições no RF
LEMBRAR DE LIMPAR O ARQUIVO "DADOS.TXT" ANTES DE INICIAR
FALTA AUTOMATIZAR

PARA SINCRONIZAR O RECEPTORRF24FINAL COM O EMISSORRF24FINAL
BASTA CARREGAR O RECEPTOR EM SEGUIDA O EMISSOR E POR FIM O RECEPTOR NOVAMENTE
POIS O RECEPTOR TEM QUE IR PRO ESTADO 0 (CARREGAMENTO 1)
PARA QUE O EMISSOR POSSA IR PRO ESTADO 0 (CARREGAMENTO 2)
E O RECEPTOR MUDOU DE ESTADO PARA QUE O EMISSOR PUDESSE IR PRO ESTADO 0
LOG O RECEPTOR TEM QUE IR PRO ESTADO 0 E OS DADOS ESTARAO SINCRONIZADOS
CASO NAO FUNCIONE EH SO REPETIR O PROCESSO, POIS EXISTE UM CASO EM QUE EH NECESSARIO REFAZER
ONDE O RECEPTOR NAO TERIA COMO FICAR NO ESTADO E REFAZENDO O PROCESSO OS DOIS PROGRAMAS IRIAM PARA O ESTADO 0
*/

#include <SPI.h>
#include "RF24.h"
#include <SD.h>
/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/
File myFile;
byte addresses[][6] = {"1Node","2Node"};
unsigned long tempoEmissor;
// Used to control whether this node is sending or receiving
int confirma =1;
bool role = 0;
int aux;
int maxGauge = 0;
int maxPiezo = 0;
int pluviometro = 0;
int estado = 0;
bool erro= false;
int pkt1[3];

int compUmidade, compPiezo, compGauge, compAcX, compAcY, compAcZ, compGyX, compGyY, compGyZ, compMililitros;
void setup() {
  Serial.begin(9600);
 
  radio.begin();
  
  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
//  radio.setPALevel(RF24_PA_LOW);
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
 radio.startListening(); 
  // Start the radio listening for data
  
}

void recebe_dados(){ 
  //FUNÇAO QUE RECEBE DADOS E ATRIBUI A DEVIDA VARIAVEL ENVIADA COMO PARAMETRO

//Estado de espera até recebe dados
   radio.startListening();
   
   if (radio.available())
   {
      while (radio.available()) {                                   // While there is data ready
       
       radio.read(&pkt1, 12);
        
        radio.stopListening();
 } 
    radio.write (&confirma, sizeof(confirma));
   Serial.println ("X"); 
       radio.startListening(); 
              
   
}

}

void loop() {  
  
  switch (estado){
    case 0:
    recebe_dados();
      estado=1;
      if (estado ==1)
      {
       // Serial.println (1); //PARA QUE O GRAFICO EM PYTHON VERIFIQUE

      }
      
    case 1:
      
     recebe_dados();
     
      compAcX = pkt1[0];
      compAcY = pkt1[1];
      compAcZ = pkt1[2];
      
        if (compAcX > 32768)
        {   compAcX= compAcX - 65536;
        }
        else if (compAcX <-32768)
        {
          compAcX = compAcX +65536;
        }
        
        if (compAcY > 32768)
        {   compAcY=compAcY  - 65536;
        }
        else if (compAcY <-32768)
        {
          compAcY = compAcY + 65536;
        }
        
        if ( compAcZ> 32768)
        { compAcZ   = compAcZ - 65536;
        }
        else if (compAcZ<-32768)
        {
          compAcZ = compAcZ +65536;
        }
        
        if ( compGyX> 32768)
        {  compGyX =  compGyX- 65536;
        }
        else if (compGyX <-32768)
        {
          compGyX = compGyX +65536;
        }
        
        if ( compGyY> 32768)
        {   compGyY= compGyY - 65536;
        }
        else if (compGyY <-32768)
        {
          compGyY = compGyY +65536;
        }
        if ( compGyZ> 32768)
        {   compGyZ= compGyZ - 65536;
        }
       else if (compGyZ< -32768)
       {
        compGyZ = compGyZ +65536;
       }
       
        Serial.println ("Valores");
         //Serial.println(compPiezo);
         //Serial.println(compGauge);
         Serial.println(compAcX);
         Serial.println(compAcY);
         Serial.println(compAcZ);
         //Serial.println(compGyX);
         //Serial.println(compGyY);
         //Serial.println(compGyZ);
         //Serial.println ("Fim");
       
        // Serial.print("pluviometro ");
         // Serial.println(compMililitros);
//}  
 //     break;
      //escrita();
      
  
  }
 radio.startListening();

/****************** Change Roles via Serial Commands ***************************/



} // Loop

