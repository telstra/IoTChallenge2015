/*
  Battery level reading example

  Assumes that the DFRobot LiPo 1000mAh 3.7V battery is in the stack.

  Sends readings to the serial interface. To test the value of the battery, unplug from USB power for about
  seconds, plug back in, then open the Serial Monitor to look at readings taken when running purely off battery.

  Battery code based on https://codebender.cc/sketch:91833#TinyScreen_Smartwatch.ino by TinyCircuits
 */

// EEPROM used to store legacy battery values that will persist when TinyDuino plugged back into USB power
#include <EEPROM.h>

// number of values to store in EEPROM
#define VALS 4
// this will be in addition to the index value

int getBatteryLevel()
{
  const long InternalReferenceVoltage = 1100L;
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
  delay(10);
  ADCSRA |= _BV( ADSC );
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
  int result = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L;
  // When charging off 5V USB from the laptop, the figure comes through around 486-488 for me
  // When operating on battery, the figure is between 300-420
  // Device will power off by the point the battery level reaches 300
  result=constrain(result-300,0,125);
  // A figure of 125 can be used to detect charging, otherwise should be between 0-120
  return result;
}

void setup()
{
  int i;
  
  // Open serial communications
  Serial.begin(9600);
  // Check index value is in the sensible range
  i = EEPROM.read(0);
  if (0 > i || i >= VALS) {
    // Not in valid range, so clear out the EEPROM
    for (i = 0; i <= VALS; i++) {
      EEPROM.write(i, 0);
    }
  }
}

void loop() // run over and over
{
  int i, last_level;
  int v = getBatteryLevel();

  last_level = (EEPROM.read(0) + 1) % VALS;
  EEPROM.write(0, last_level);
  EEPROM.write(last_level + 1, v);
  Serial.print("Battery readings: ");
  for (i = 0; i < VALS; i++) {
    v = EEPROM.read((last_level - i + VALS) % VALS + 1);
    if (125 == v) {
      Serial.print("CHARGING");
    } else {
      Serial.print((float)(v / 1.2));
      Serial.print('%');
    }
    Serial.print(' ');
  }
  Serial.println();
  delay(5000); // wait 5 seconds
  // go back and read battery again
}

