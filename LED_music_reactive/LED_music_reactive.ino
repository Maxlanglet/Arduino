#include "arduinoFFT.h"
#define samples 16
#define sampling_frequency 20000 

#include <FastLED.h>
#include <math.h>

#define LED_PIN     11
#define NUM_LEDS    60
#define BRIGHTNESS  45
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period;
unsigned long microseconds;

double vReal[samples];
double vIm[samples];

int micpin = A0;
int ledpin = 11;
int micvalue1 = 0;
int micvalue2 = 0;

CRGBPalette16 currentPalette;

void setup() {
  // put your setup code here, to run once:
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  pinMode(ledpin,OUTPUT);
  pinMode(micpin, INPUT);
  Serial.begin(9600);
  sampling_period = round(10000*(1/sampling_frequency));

  for ( int i=0; i<NUM_LEDS;i++){
    leds[i]=CRGB(0,0,0);
  }
  FastLED.show();

}

void loop() {
  // put your main code here, to run repeatedly:
  //leds[1]=CRGB::YellowGreen;
  
  micvalue1 = analogRead(micpin);
  //Serial.println(micvalue1);
  delay(10);
  micvalue2 = analogRead(micpin);
  //Serial.println(micvalue2);
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

    
    for(int i = NUM_LEDS - 1; i >= 1; i--) {
      leds[i] = leds[i - 1];
  }
    if (micvalue2-micvalue1>1){
      leds[0]=CRGB(frequencytoRed(peak), frequencytoGreen(peak), frequencytoBlue(peak));
  //leds[i] += CRGB( 20, 0, 0);
      
    }
    else{
      leds[0]=CRGB::Black;
    }
    FastLED.show();
    delay(1);
}

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