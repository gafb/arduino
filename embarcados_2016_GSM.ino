//Programa: Arduino GSM Shield SIM900 - Acesso internet
//Autor: Arduino e Cia

#include <SoftwareSerial.h>
#include "inetGSM.h"
#include <string.h>

InetGSM inet;

boolean started = false;
char msg[10];
int numdata, numdata2;

void setup()
{
    Serial.begin(9600);
    powerUpOrDown();
    Serial.println(F("Testando GSM Shield SIM900"));
    
    if (gsm.begin(9600))
    {
        Serial.println(F("\nstatus=READY"));
        started = true;
    }
    else Serial.println(F("\nstatus=IDLE"));
    
    if (started)
    {
        if (inet.attachGPRS("timbrasil.br", "tim", "tim"))
            Serial.println(F("status=Conectado..."));
        else Serial.println(F("status=Nao conectado!!"));
    
        delay(100);
        
        //numdata2 = inet.httpPOSTinit("slipalert.comxa.com", 80/*, "/add.php"*/);
    }
};

void loop()
{
    if (started)
    {
        //Aguarda novos dados e envia para o servidor web
        if ((Serial.available() > 0)/* && (numdata2 != 0)*/)
        {
            char* data = readStringSerial();
            
            if(!strcmp(data,"MSG_Texto1=desconnectTCP"))
            {
                free(data);
                inet.disconnectTCP();
                /*numdata = inet.httpPOSTend(msg, 50);
                Serial.print(F("numdata="));
                Serial.println(numdata);*/
                started = 0;
            }
            else
            {
                numdata = inet.httpPOST("url", 80, "/add.php", data, msg, 50);
                
                //delay(5000);
                
                msg[0] = '\0';
                
                free(data);
                //delay(10000);
            }
        }
        
        //delay(1000);
    }
}

void powerUpOrDown()
{
    //Liga o GSM Shield
    //D9 - Used for software control the power up or down of the SIM900
    Serial.print(F("Liga GSM..."));
    
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);
    delay(1000);
    
    digitalWrite(9, HIGH);
    delay(1000);
    
    Serial.println(F("OK!"));
    digitalWrite(9, LOW);
    delay(500);
}

/**
 * Função que lê uma string da Serial
 * e retorna-a
 **/
char* readStringSerial()
{
    String conteudo = "";
    char caractere;
    char* temp_string;
    temp_string = (char*) malloc(55*sizeof(char));
    
    // Enquanto receber algo pela serial
    while(Serial.available() > 0)
    {
        // Lê byte da serial
        caractere = Serial.read();
        
        // Ignora caractere de quebra de linha
        if (caractere != '\n')
        {
            // Concatena valores
            conteudo.concat(caractere);
        }
        
        // Aguarda buffer serial ler próximo caractere
        delay(5);
    }
      
    Serial.print("Recebi: ");
    Serial.println(conteudo);
    
    String valor = "MSG_Texto1=" + conteudo;
    valor.toCharArray(temp_string, 55);
    
    return temp_string;
}

/*void envia_GSM(char* texto)
{
    char msg[10];
    int numdata;

    numdata = inet.httpPOST("url", 80, "/add.php", texto, msg, 50);
    delay(5000);
}*/
