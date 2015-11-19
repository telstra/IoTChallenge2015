//-------------------------------------------------------------------------------
//  Microphone Example Sketch
//  Using SF 1620 Microphone breakout
//
//-------------------------------------------------------------------------------

int sensorPin = A1;    // select the input pin for the potentiometer
float tempVal = 0;  // variable to store the value coming from the sensor
int LEDnum;

void setup() {

}

void loop() {
  
  float maxVal = 0;
  
  // Wait a moment for things to settle

  // read the value from the sensor
  for(int i=0; i < 50; i++)
  {
    tempVal = abs(analogRead(sensorPin) - 512); //Microphone is biased at VCC / 2, so will read 512 with "no sound".
    if (tempVal > maxVal)
    {
      maxVal = tempVal; //Log the loudest thing in 50 samples
    }
  }

  LEDnum = maxVal / 32; // Scale is from 0 to 512, rescale to 0 to 16

  LedOn(LEDnum);

  //This runs flat out! Not great for battery life :)
  
}

//-------------------------------------------------------------------------------
//  LED Control Helper
//-------------------------------------------------------------------------------

void LedOn(int ledNum)
{
  for(int i=5;i<10;i++){
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  };
  if(ledNum<1 || ledNum>16) return;
  char highpin[16]={5,6,5,7,6,7,6,8,5,8,8,7,9,7,9,8};
  char lowpin[16]= {6,5,7,5,7,6,8,6,8,5,7,8,7,9,8,9};
  ledNum--;
  digitalWrite(highpin[ledNum],HIGH);
  digitalWrite(lowpin[ledNum],LOW);
  pinMode(highpin[ledNum],OUTPUT);
  pinMode(lowpin[ledNum],OUTPUT);
}
