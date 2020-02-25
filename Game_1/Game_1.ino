#include "pitches.h"
 /*
// notes in the melody:
int melody[] = {
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6};
int duration = 500;  // 500 miliseconds
 
void setup() {
 
}
 
void loop() {  
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // pin8 output the voice, every scale is 0.5 sencond
    tone(8, melody[thisNote], duration);
     
    // Output the voice after several minutes
    delay(1000);
  }
   
  // restart after two seconds 
  delay(2000);
}
*/

int melody[] = {
  NOTE_FS5, NOTE_FS5, NOTE_G5, NOTE_FS5, NOTE_C5, NOTE_G5, NOTE_B4, NOTE_G5};//XXXTENTATION-LOOK AT ME
int len = 8;

int tDelay = 60; //70 pas mal essayer de faire varier, implementer avec random
int initDelay = tDelay;
int songtime = 700;
int latchPin = 11;      // (11) ST_CP [RCK] on 74HC595
int clockPin = 9;      // (9) SH_CP [SCK] on 74HC595
int dataPin = 12;     // (12) DS [S1] on 74HC595
int buttonApin = 6;
int buttonBpin = 5;

byte leds = 0;

int score1=0;
int score2=0;

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

void setup() 
{
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(buttonApin, INPUT_PULLUP);  
  pinMode(buttonBpin, INPUT_PULLUP);  
  Serial.begin(9600);
  for ( int k=0;k<len;k++){
    tone(8, melody[k], songtime);
    delay(songtime);
  }
}

void loop() 
{
  leds = 0;
  updateShiftRegister();
  //delay(tDelay);
  for (int i = 0; i < 8; i++)
  {
    bitSet(leds, i);
    //tone(8, melody[i], tDelay);
    updateShiftRegister();
    if (digitalRead(buttonApin) == LOW ){
      tDelay=initDelay;
      tDelay-=random(0,40);
      Serial.println(tDelay);
      if (i==0){
        score1++;
        //Serial.println(score1);
        for (int j=0;j<score1;j++){
          tone(8, melody[j], songtime);
          delay(songtime);
        }
      }
      delay(1500);
    }
    if (digitalRead(buttonBpin) == LOW){
      tDelay=initDelay;
      tDelay-=random(0,40);
      Serial.println(tDelay);
      if (i==7){
        score2++;
        //Serial.println(score2);
        for (int j=0;j<score2;j++){
          tone(8, melody[j], songtime);
          delay(songtime);
        }
      }
      delay(1500);
    }
    delay(tDelay);
    leds=0;
  }
  for (int i = 6; i >= 1; i--)
  {
    bitSet(leds, i);
    //tone(8, melody[i], tDelay);
    updateShiftRegister();
    if (digitalRead(buttonApin) == LOW ){
      tDelay=initDelay;
      tDelay-=random(0,40);
      Serial.println(tDelay);
      delay(2500);
    }
    if (digitalRead(buttonBpin) == LOW){
      tDelay=initDelay;
      tDelay-=random(0,40);
      Serial.println(tDelay);
      delay(2500);
    }
    delay(tDelay);
    leds=0;
  }
  if(score1==len || score2==len){
    score1=0;
    score2=0;
  }
}
