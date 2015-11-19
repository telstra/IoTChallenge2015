//-------------------------------------------------------------------------------
//  Temperature Sensor Example Sketch
//  Using TMP36
//
//-------------------------------------------------------------------------------

int sensorPin = A0;    // select the input pin for the potentiometer
int VsPin = 7;    // select the input pin for the potentiometer
float tempVoltage = 0;  // variable to store the value coming from the sensor
float degreesC = 0;
const float ADCtoV = 0.004882814;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(VsPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // turn Vs on the Sensor high
  digitalWrite(VsPin, HIGH);
  
  // Wait a moment for things to settle
  delay(200);

  // read the value from the sensor
  tempVoltage = analogRead(sensorPin) * ADCtoV;
  degreesC = (tempVoltage - 0.5) * 100.0;
  
  // turn Vs on the Sensor low
  digitalWrite(VsPin, LOW);

  Serial.print("voltage: ");
  Serial.print(tempVoltage);
  Serial.print("  deg C: ");
  Serial.println(degreesC);
  
  // Wait a few seconds
  delay(1000);
}
