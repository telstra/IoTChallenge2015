/*
 * Class example for communicating the the Telstra LPWAN modem.
 */
 
#include <SoftwareSerial.h>
#include <Regexp.h> // gammon.com.au/Arduino/Regexp.zip

// Remove this define to keep the USB comms quiet
#define DEBUG

#define rx_timeout 8000
#define rx_timeout_fast 500
#define rx_buffer 200

#define idDevAddr "bb 49 10 00"
#define idDevEui "11 11 11 11 bb 49 10 00"
#define idNwSKey "7B 98 20 5A CA 2D B4 F4 A6 1C 73 BB 3A C5 C5 A6"
#define idAppSKey "CF B9 90 F6 21 C6 B6 9A D0 A9 8D 23 F7 D1 9D 4A"

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

SoftwareSerial _LoRaSerial(2,3);

/***************************************************************
 * 
 * LoRaModem Class
 * 
 * LoRaModem.checkAT() -> Send AT to the modem, expect AT+OK back.
 * LoRaModem.checkID() -> Retrieve the current ID parameters from the modem (updates LoRaModem.DevAddr / DevEui / AppEui)
 * LoRaModem.setID()   -> Set the ID parameters in the modem
 * LoRaModem.setKeys() -> Set the NW and App ciphering / integrity keys
 * LoRaModem.setPort() -> Set the port # for App payloads
 * LoRaModem.cMsg()    -> Send a message, wait for an ACK
 * LoRaModem.Msg()     -> Send a message, no ACK (uplink only)
 * LoRaModem.Reset()   -> Reset the modem. This is useful if it hangs, sometimes happens after sending a few hundred messages
 * LoRaModem.getAscii()-> If a Downlink payload is received, extract it and decode from hex to Ascii (use after Msg or cMsg).
 * 
 * LoRaModem.modemResp -> char buffer with the last message received on UART
 *  
 */

class LoRaModem
{
  public:
    LoRaModem();
    int checkAT();
    int checkID();
    int setID(String addr, String dev);
    int setKeys(String NWKey, String AppKey);
    int setPort(String portNum);
    int cMsg(String message);
    int Msg(String message);
    int Reset();

    String getAscii();
    
    char modemResp[rx_buffer];
    char DevAddr[12];
    char DevEui[24];
    char AppEui[24];

  private:
    MatchState _rspMs;
    void _sendSerial(String message);
    int _checkresponse(char* checkVal, int call_timeout);
};

LoRaModem::LoRaModem()
{
  _LoRaSerial.begin(9600);
  _LoRaSerial.setTimeout(100);
};

/******************
 * _sendSerial
 * Generic private method for sending serial data to the modem, used by everything. 
 */

void LoRaModem::_sendSerial(String message)
{
  DEBUG_PRINT("> " + message);
  _LoRaSerial.read();
  _LoRaSerial.println(message);
};

/******************
 * _checkresponse
 * Generic private method for reading back from the modem, and qualifying the response against a Regex 
 */

int LoRaModem::_checkresponse(char* checkVal, int call_timeout = rx_timeout)
{
  modemResp[0] = 0;
  
  unsigned long startTime = millis();

  int i = 0;
  
  while (i < rx_buffer-1)
  {
    int rxcount = _LoRaSerial.readBytes(&modemResp[i], rx_buffer-i);
    i += rxcount;
    modemResp[i] = 0;
    
    if (millis() - startTime > call_timeout)
    {
        _rspMs.Target(modemResp, i);
        char result = _rspMs.Match(checkVal, 0);
        if (result == REGEXP_MATCHED)
        {
          //Terminate the string
          DEBUG_PRINT(modemResp);
          return 0;
        }
        else
        {
          DEBUG_PRINT("Unexpected response / no response");
          DEBUG_PRINT(modemResp);
          return 1;
        }
    }
   
  }
  
};


// Check AT, just check if the modem is alive
int LoRaModem::checkAT(){
  _sendSerial("AT");
  return _checkresponse("%+AT: OK", rx_timeout_fast);  
};

// Find out current modem ID
int LoRaModem::checkID(){
  _sendSerial("AT+ID");
  if (_checkresponse("%+ID: DevAddr (.+)\r\n%+ID: DevEui (.+)\r\n%+ID: AppEui (.+)"))
  {
    return 1;
  }
  _rspMs.GetCapture (DevAddr, 0);
  _rspMs.GetCapture (DevEui, 1);
  _rspMs.GetCapture (AppEui, 2);

   String temp_s;
   temp_s = String(DevAddr);
   temp_s.replace(":", " ");
   temp_s.toCharArray(DevAddr, 12);

   temp_s = String(DevEui);
   temp_s.replace(":", " ");
   temp_s.toCharArray(DevEui, 24);

   temp_s = String(AppEui);
   temp_s.replace(":", " ");
   temp_s.toCharArray(AppEui, 24);

};

// Set the modem ID
int LoRaModem::setID(String addr, String dev){
  _sendSerial("AT+ID=DevAddr,\"" + addr + "\"");
  if (_checkresponse("%+ID: DEVADDR .+", rx_timeout_fast))
  {
    return 1;
  }
  
  _sendSerial("AT+ID=DevEui,\"" + dev + "\"");
  if (_checkresponse("%+ID: DEVEUI .+", rx_timeout_fast))
  {
    return 1;
  }

  return 0;
};

// Set the modem keys
int LoRaModem::setKeys(String NWSKey, String AppSKey){
  _sendSerial("AT+KEY=NWKSKEY,\"" + NWSKey + "\"");
  if (_checkresponse("%+KEY: NWKSKEY .+", rx_timeout_fast))
  {
    return 1;
  }
  
  _sendSerial("AT+KEY=APPSKEY,\"" + AppSKey + "\"");
  if (_checkresponse("%+KEY: APPSKEY .+", rx_timeout_fast))
  {
    return 1;
  }

  return 0;
  
};

// Set the payload port #
int LoRaModem::setPort(String portNum){
  _sendSerial("AT+PORT=" + portNum);
  if (_checkresponse("%+PORT: OK.+", rx_timeout_fast))
  {
    return 1;
  }

  return 0;
  
};

// Send a message, expect an ACK
int LoRaModem::cMsg(String message){
  _sendSerial("AT+CMSG=\"" + message + "\"");

  if (_checkresponse(".+ACK Received.+CMSG: Done"))
  {
    return 1;
  }

  return 0;
}

// Send a message, don't expect an ACK
int LoRaModem::Msg(String message){
  _sendSerial("AT+MSG=\"" + message + "\"");

  if (_checkresponse(".+MSG: Done"))
  {
    return 1;
  }

  return 0;
}

// Reset the modem
int LoRaModem::Reset(){
  _sendSerial("AT+RESET");

  if (_checkresponse(".+RESET: OK.+ 1.7.1"))
  {
    return 1;
  }
  return 0;
}

// Extract the downlink payload (if any) and convert to ASCII
String LoRaModem::getAscii(){

  char buf[100];
  
  char result = _rspMs.Match(".+RX: \"(.+)\".+", 0);
  if (result == REGEXP_MATCHED)
    {
      _rspMs.GetCapture (buf, 0);
    
      String ASCIIValue = "";
      
      for (int i = 0; i < strlen(buf)+1; i += 3) {
          char val = buf[i] > 0x39 ? (buf[i] - 55) * 16 : (buf[i] - '0') * 16;
          val += buf[i+1] > 0x39 ? (buf[i+1] - 55) : (buf[i+1] - '0');
          ASCIIValue += val;
        }
      return ASCIIValue;
    }
    else
    {
      DEBUG_PRINT("No ASCII payload...");
      return "";
    }
}


/***************************************************************
 * Normal Arduino stuff starts here.
 */

LoRaModem modem;

void setup()
{
  #ifdef DEBUG
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    // wait a moment for the serial port to finish starting up
    delay(1000);
  #endif
  
  DEBUG_PRINT("Connected");

  modem.Reset();
  delay(1000);
  modem.setPort("1");
  delay(1000);
  
  modem.setID(idDevAddr, idDevEui);
  delay(1000);
  modem.setKeys(idNwSKey, idAppSKey);
  
  
}

/* Loop ****************************/

void loop() // run over and over
{
  
  modem.cMsg("100,123.45");
  
  delay(1000); 

}
