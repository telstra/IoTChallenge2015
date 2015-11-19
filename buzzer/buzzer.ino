//-------------------------------------------------------------------------------
//  Buzzer Example Sketch
//  Based on Sparkfun SIK Experiment Guide v3.2
//
//-------------------------------------------------------------------------------

const int buzzerPin = 9;    // select the output pin for the buzzer
const int songLength = 10;
char notes[] = "aC aC C DC";  //Telstra tune :P
int tempo = 150;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(buzzerPin, OUTPUT);
}

void loop() 
{
  int i, duration;

  for (i = 0; i < songLength; i++)
  {
    if (notes[i] == ' ')
    {
      delay(tempo);            // Just pause for a blank
    }
    else                      
    {
      tone(buzzerPin, frequency(notes[i]), tempo); //The actual part where a tone is played
      delay(tempo);                                //tone() is Asyncronous, so pause execution while it runs
    }
    delay(tempo/10);              // brief pause between notes
  }

  while(true){}            //Pause forever at the end (comment out to play on repeat!)

}


int frequency(char note) 
{

  int i;
  const int numNotes = 9;

  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C', 'D' };
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523, 587};

  for (i = 0; i < numNotes; i++) 
  {
    if (names[i] == note)      
    {
      return(frequencies[i]);   
    }
  }
  return(0); 
}
