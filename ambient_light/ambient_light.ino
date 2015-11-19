//-------------------------------------------------------------------------------
//  Ambient Light TinyShield Example Sketch
//  Using AMS TAOS TSL2572
//
//  Based on code by Ken Burns, TinyCircuits http://Tiny-Circuits.com
//
//-------------------------------------------------------------------------------

#include <Wire.h>

// // Constants defined for the light sensor
#define TSL2572_I2CADDR     0x39
#define GAIN_1X 0
#define GAIN_8X 1
#define GAIN_16X 2
#define GAIN_120X 3

const byte gain_val = 8;//set to actual gain value(1,8,16,120)
// // End constants for the light sensor

//-------------------------------------------------------------------------------
//  Main Arduino Code Sections
//-------------------------------------------------------------------------------

void setup()
{
  LedOn(0);//Pass a zero to turn all LEDs off
  
  Wire.begin();
  Serial.begin(9600);
  TSL2572nit(GAIN_8X);
}


void loop()
{
  float AmbientLightLux = Tsl2572ReadAmbientLight();
  int LEDnum = AmbientLightLux / 62;  //Scale from 0 to ~1000 in 16 steps
  Serial.print("Lux: ");
  Serial.print(AmbientLightLux);

  LedOn( min(LEDnum, 16) );
  
  delay(500);
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

//-------------------------------------------------------------------------------
// Ambient Light Helpers
//-------------------------------------------------------------------------------

void TSL2572nit(uint8_t gain)
{
  Tsl2572RegisterWrite( 0x0F, gain );//
  Tsl2572RegisterWrite( 0x01, 0xF6 );//27.3 ms
  Tsl2572RegisterWrite( 0x00, 0x03 );//turn on
}


void Tsl2572RegisterWrite( byte regAddr, byte regData )
{
  Wire.beginTransmission(TSL2572_I2CADDR);
  Wire.write(0x80 | regAddr); 
  Wire.write(regData);
  Wire.endTransmission(); 
}


float Tsl2572ReadAmbientLight()
{     
  uint8_t data[4]; 
  int c0,c1;
  float lux1,lux2,cpl;

  Wire.beginTransmission(TSL2572_I2CADDR);
  Wire.write(0xA0 | 0x14);
  Wire.endTransmission();
  Wire.requestFrom(TSL2572_I2CADDR,4);
  for(uint8_t i=0;i<4;i++)
    data[i] = Wire.read();
     
  c0 = data[1]<<8 | data[0];
  c1 = data[3]<<8 | data[2];
  
  //see TSL2572 datasheet
  cpl = 27.3 * (float)gain_val / 60.0;
  lux1 = (c0 - (1.87 * c1)) / cpl;
  lux2 = ((0.63 * c0) - c1) / cpl;
  cpl = max(lux1, lux2);
  return max(cpl, 0.0);
}
