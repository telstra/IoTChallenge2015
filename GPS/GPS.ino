//-------------------------------------------------------------------------------
//  GPS Sensor Example Sketch
//  Using UBX-G7020-KT solution
//
//-------------------------------------------------------------------------------

#include <SoftwareSerial.h>
#include <TinyGPS.h> // https://www.dfrobot.com/wiki/index.php?title=GPS_Module_With_Enclosure_%28SKU:TEL0094%29
#include <Adafruit_SleepyDog.h>

SoftwareSerial GPSSerial(5, 4); // RX, TX
TinyGPS gps;

// Variables for the GPS readings
long GPSlat, GPSlon;
unsigned long GPSfix_age, GPShdop;
const int hdop_prec = 180;        //Minimum acceptable GPS quality (100ths of horizontal dilution of precision)
char GPSc;

//-------------------------------------------------------------------------------
//  Main Arduino Code Sections
//-------------------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);
 
  // Open GPS comms, but disable the GPS module by keeping IO6 low.
  GPSSerial.begin(9600);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  
  Serial.println("Ready!");
}

void loop() // run over and over
{
  //Wake the GPS module up
  digitalWrite(6, HIGH);

  GPShdop = 0xffffffff;

  //Read data on serial until the GPS returns something sensible
  while (GPShdop > hdop_prec)
  {
    while (GPSSerial.available())
    {
    
      GPSc = GPSSerial.read();                   // Read the GPS data
      // Serial.print(GPSc);                     // Uncomment to see verbose GPS data
      if (gps.encode(GPSc))                      // Feed next GPS char to librarby, returns true if some GPS information changed
      {
        gps.get_position(&GPSlat, &GPSlon, &GPSfix_age);
        GPShdop = gps.hdop();
      }
    }
  }

  //Shut the GPS back down
  digitalWrite(6, LOW);

  //Print GPS details
  Serial.println("\n------------ GPS Fix!------------");
  Serial.print(GPSlat / 1000000.0, 6);
  Serial.print(", ");
  Serial.print(GPSlon / 1000000.0, 6);
  Serial.print(", ");
  Serial.println(GPSfix_age);
  
  delay(1000); // Delay a bit to let everything finish on serial

  //Go to sleep (Adafruit Sleepydog library). Default / max sleep is 8 seconds, put it in a loop for longer periods.
  int sleepMS = Watchdog.sleep();  
}
