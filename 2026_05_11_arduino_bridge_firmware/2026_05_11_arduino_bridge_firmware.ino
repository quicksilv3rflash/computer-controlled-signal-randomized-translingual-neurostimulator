/* 
This firmware accepts a serial command from the connected computer and pulses the output of the TLNS device.
The serial command is in the following 16-character format: F#####I##D######
F defines frequency from 00000 to 99999 Hz.
I defines stimulation intensity from 0 to 20 (not yet implemented in the TLNS firmware).
D defines duration, in milliseconds, from 000000 to 999999 ms.
*/

#include <string.h> //required library for handling string conversions

double frequency = 0; //frequency of TLNS stimulation, hertz
uint8_t intensity = 0; //intensity of TLNS stimulation (0 to 20, not yet implemented)
double duration = 0; //duration of TLNS simulation, milliseconds

char serialChar = 0; //stores the first character of the serial command sent to the arduino (*should* be 'F')

uint8_t currently_outputting = 0; //set to 1 if an output is to be supplied by TLNS
double remaining_duration = 0; //used to track the remaining time in the current stimulation period (in milliseconds)

String frequency_string; //intermediate string variable storing frequency of TLNS stimulation (Hz)
String intensity_string; //intermediate string variable storing intensity of TLNS stimulation (0 to 20, not yet implemented)
String duration_string; //intermediate string variable storing duration of TLNS stimulation (ms)

void setup() { //runs once at power-on
  Serial.begin(115200); //begin serial communication at 115200 baud
  Serial.setTimeout(50); // set serial timeout to 50ms
  pinMode(9, OUTPUT); // sets pin D9 to output (for using analogWrite() function to set stimulation intensity)
  pinMode(2, OUTPUT); //pin D2 commands an output pulse from the TLNS device with each falling edge
  digitalWrite(2, HIGH); //leaves pin D2 "HIGH" until output is required
}

void loop() { //runs continuously

  if (Serial.available() > 0) { // if serial input is present
    serialChar = Serial.read(); //when the serial command to provide stimulation is applied, 'F' is stored in serialChar
    frequency_string = Serial.readStringUntil('I');
    intensity_string = Serial.readStringUntil('D');
    duration_string = Serial.readString();
    frequency = frequency_string.toDouble();
    intensity = intensity_string.toInt();
    duration = duration_string.toDouble();
    analogWrite(9, (int)((74 * intensity)/20)); // sets output stimulation intensity immediately after command is received
    //note: the stimulation intensity will take ~50ms to stabilize after being set
  }

  if(serialChar != 0){ //if 'F' is stored in serialChar, then TLNS output is requested.
    currently_outputting = 1; //indicate that output is required
    serialChar = 0; //reset serialChar to 0
  }

  if(currently_outputting == 1){ //if an output pulse has been requested
    remaining_duration = duration; //set the remaining duration to the total requested duration

    while(remaining_duration > 0){ //while there is still time remaining in the requested stimulation period
      digitalWrite(2, LOW); //this falling edge commands an output pulse
      //wait for half a frequency cycle
      for(double half_period_in_microseconds = 500000.0/frequency; half_period_in_microseconds > 0; half_period_in_microseconds -= 10){
        delayMicroseconds(10);
      }
      digitalWrite(2, HIGH); //reset pin D2 to default "HIGH" state
      //wait for half a frequency cycle
      for(double half_period_in_microseconds = 500000.0/frequency; half_period_in_microseconds > 0; half_period_in_microseconds -= 10){
        delayMicroseconds(10);
      }
      remaining_duration -= 1000.0/frequency; //decrement remaining_duration by the amount of time taken by one frequency cycle (in ms)
    }

    digitalWrite(2, HIGH); //reset pin D2 to default "HIGH" state
    remaining_duration = 0; //set remaining_duration to exactly zero
    currently_outputting = 0; //set currently_outputting to no (0)
  }

}
