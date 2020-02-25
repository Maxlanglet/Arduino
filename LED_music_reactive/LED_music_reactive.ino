#include "arduinoFFT.h"
#define samples 16
#define sampling_frequency 20000 

#include <FastLED.h>
#include <math.h>

#define LED_PIN     11 //Output pin on the arduino
#define NUM_LEDS    60
#define BRIGHTNESS  45
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
//Using FFT for frequency approximation
arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period;
unsigned long microseconds;

double vReal[samples];
double vIm[samples];

int micpin = A0;
int digitpin = 7;
int ledpin = 11;
int micvalue1 = 0;
int micvalue2 = 0;

CRGBPalette16 currentPalette;

void setup() {
  // setup code
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  pinMode(ledpin,OUTPUT);
  pinMode(micpin, INPUT);
  pinMode(digitpin, INPUT);
  Serial.begin(9600);
  sampling_period = round(10000*(1/sampling_frequency));

  for ( int i=0; i<NUM_LEDS;i++){
    leds[i]=CRGB(0,0,0);
  }
  FastLED.show();

}

void loop() {
  // detection of sounds every 10 ms
  //leds[1]=CRGB::YellowGreen;
  
  //micvalue1 = analogRead(micpin);
  micvalue1 = digitalRead(micpin);//Works a little better sensibility wise with digital output
  //Serial.println(micvalue1);
  delay(10);
  //micvalue2 = analogRead(micpin);
  //Serial.println(micvalue2);
  //Fast Fourier Transform
  for(int i=0; i<samples;i++){
      microseconds = micros();

      vReal[i]=analogRead(micpin);
      vIm[i]=0;
      while(micros() <(microseconds+sampling_period)){}
      
    }
    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal,vIm,samples,FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal,vIm,samples);

    double peak = FFT.MajorPeak(vReal,samples, sampling_frequency);
    //Serial.println(peak);

    //Shifting of every color to the next LED
    for(int i = NUM_LEDS - 1; i >= 1; i--) {
      leds[i] = leds[i - 1];
  }
  //If sound is detected, we have an RGB color assigned to an LED based on peak
    if (micvalue1==1){//micvalue2-micvalue1>1
      leds[0]=CRGB(frequencytoRed(peak), frequencytoGreen(peak), frequencytoBlue(peak));
  //leds[i] += CRGB( 20, 0, 0);
      
    }
    else{
      leds[0]=CRGB::Black;
    }
    FastLED.show();
    delay(1);
}
//Functions from a double to a RGB color
int frequencytoRed(double peak){
  if (peak>70 && peak <7000){
      return round((peak/10000)*255);
    }
  else{
    return 0;
  }
}

int frequencytoGreen(double peak){
  if (peak>5000 && peak <8500){
     return round((peak/10000)*255);
    }
  else{
    return 0;
  }
}

int frequencytoBlue(double peak){
  if (peak>7750 && peak <10000){
      return round((peak/10000)*255);
    }
  else{
    return 0;
  }
}
