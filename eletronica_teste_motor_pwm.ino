byte motorPin = 3;

void setup()
{
pinMode(motorPin, OUTPUT);
Serial.begin(9600);

}

void loop()
{
  
    Serial.println("25%");
    delay(5000);
    analogWrite(motorPin, 128);
    Serial.println("50%");
    delay(5000);
    analogWrite(motorPin, 255);
    Serial.println("100%");
    delay(5000);
    
  /*  
    analogWrite(motorPin, 150);
    Serial.println("150");
    delay(4000);
    analogWrite(motorPin, 0);
    Serial.println("0");
    delay(4000);
 
    digitalWrite(motorPin,LOW);
    */
}
