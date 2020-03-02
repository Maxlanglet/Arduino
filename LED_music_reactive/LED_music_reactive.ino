#include "IRremote.h"
#include "arduinoFFT.h"

#define samples 16  //number of samples for reconstructing the frequency wave, does what i want wich is kind of change color with frequency. 
#define sampling_frequency 20000 

#include <FastLED.h>
#include <math.h>

#define LED_PIN     13
#define NUM_LEDS    60
#define BRIGHTNESS  45
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

FASTLED_USING_NAMESPACE
// for fire like animation

#define COOLING  55
#define SPARKING 120

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


#define FRAMES_PER_SECOND2 60

bool gReverseDirection = false;

#define FRAMES_PER_SECOND  120

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period;
unsigned long microseconds;

double vReal[samples];
double vIm[samples];

int micpin = A0;
int digitpin = 7;
int ledpin = 13;
int micvalue1 = 0;
int micvalue2 = 0;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


int cycle=0; //variable defined for rotating animations using IR remote
int receiver = 2; // Signal Pin of IR receiver to Arduino Digital Pin 2

#define IR_TOLERANCE 17

IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

int translateIR() // can't get to work the specific key press so this is sufficient and cycle through the animations
{
  switch(results.value){
  case 0xFFFFFFFF: Serial.println(" REPEAT");return 0; break;  
  default: 
    Serial.println(" other button   "); return 1; break;
  }
  delay(1500); // Do not get immediate repeat
}

CRGBPalette16 gPal;//for fire animation

void setup() {
  irrecv.enableIRIn(); // Start the receiver

  
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  pinMode(ledpin,OUTPUT);
  pinMode(micpin, INPUT);
  pinMode(digitpin, INPUT);
  Serial.begin(9600);
  sampling_period = round(10000*(1/sampling_frequency));
  //Serial.begin(9600);
  //Serial.println("IR Receiver Button Decode"); 

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  gPal = HeatColors_p;
  
  
  for ( int i=0; i<NUM_LEDS;i++){
    leds[i]=CRGB(0,0,0);
  }
  FastLED.show();

}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns



void loop() {
  
  if (irrecv.decode(&results)) // have we received an IR signal?
    {
      int number = translateIR(); 
      if (number ==0 || number==1){
        cycle++;
        if (cycle==6){
          cycle=0;
        }
      }
      delay(1500);
      irrecv.resume(); // receive the next value
    }

  if (cycle == 0){
    for (int i =0;i<NUM_LEDS;i++){
      leds[i]=CRGB::MediumSeaGreen;
    }
    FastLED.show();
  }


//First changing RGB animation toggle, from FastLED library examples
  else if (cycle ==1){
    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1;
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }


//Second changing RGB animation toggle, from FastLED library examples
  else if (cycle ==2){
    // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  }


//Third changing RGB animation toggle, fire like animation, from FastLED library examples
  else if (cycle ==3){
      // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy( random());
  
    // Fourth, the most sophisticated: this one sets up a new palette every
    // time through the loop, based on a hue that changes every time.
    // The palette is a gradient from black, to a dark color based on the hue,
    // to a light color based on the hue, to white.
    //
    //   static uint8_t hue = 0;
    //   hue++;
    //   CRGB darkcolor  = CHSV(hue,255,192); // pure hue, three-quarters brightness
    //   CRGB lightcolor = CHSV(hue,128,255); // half 'whitened', full brightness
    //   gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);
  
  
    Fire2012WithPalette(); // run simulation frame, using palette colors
    
    FastLED.show(); // display this frame
    FastLED.delay(1000 / FRAMES_PER_SECOND2);
  }

//Music Reactive code, from myself
  else if (cycle==4){
    //micvalue1 = analogRead(micpin);
    micvalue1 = digitalRead(micpin);//Works a little better sensibility wise with digital output
    //Serial.println(micvalue1);
    delay(10);
    //micvalue2 = analogRead(micpin);
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
      if (micvalue1==1){//micvalue2-micvalue1>1
      /*
        int x=random(-50,50);
        int y=random(-50,50);
        int z=random(-50,50);
        if (frequencytoRed(peak)-x>=0 && frequencytoGreen(peak)-y>=0 && frequencytoBlue(peak)-z>=0 
            && frequencytoRed(peak)-x<=255 && frequencytoGreen(peak)-y<=255 && frequencytoBlue(peak)-z<=255){
          leds[0]=CRGB(frequencytoRed(peak)-x, frequencytoGreen(peak)-y, frequencytoBlue(peak)-z);
        }
        else{
        */
          leds[0]=CRGB(frequencytoRed(peak), frequencytoGreen(peak), frequencytoBlue(peak));
          //Serial.println("top");
        //}
      }
      else{
        leds[0]=CRGB::Black;
      }
      FastLED.show();
      delay(1);
  }
//Turn the LED strip off
  else{
    for ( int i =0; i<NUM_LEDS;i++){
      leds[i]=CRGB::Black;
    }
    delay(200);
    FastLED.show();
  }
}


//Functions from peak to frequency
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


//From FastLED examples
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}





void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}
