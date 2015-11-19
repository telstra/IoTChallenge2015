//-------------------------------------------------------------------------------
//  Proximity Sensor Example Sketch
//  Using SEN0171
//
//-------------------------------------------------------------------------------

byte enPin = 9; // VCC pin for the SEN0171 (assuming it's not wired to VCC)
byte sensorPin = 8;
byte indicator = 13;
 
void setup()
{
  pinMode(sensorPin,INPUT);
  pinMode(indicator,OUTPUT);
  pinMode(enPin,OUTPUT);
  digitalWrite(enPin,HIGH); // Turn the sensor on, you would probably do this just before you used it
  Serial.begin(9600);
}
 
void loop()
{
  byte state = digitalRead(sensorPin);
  digitalWrite(indicator,state);
  if(state == 1)
  {
    Serial.println("Motion!");
  }
  else if(state == 0)
  {
    Serial.println("Nothing...");
  }
  delay(500);
}
