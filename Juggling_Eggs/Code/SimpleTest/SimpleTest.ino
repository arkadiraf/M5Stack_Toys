#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN G0
#define LED_COUNT  15
// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 100 // should not exceed 150, single color draw at 250-300 ma (max output limit of 400 ma SGM6603 datasheet)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// the setup routine runs once when M5StickC starts up
void setup() {
  
  // initialize the M5StickC object
  M5.begin();
  M5.Axp.SetChargeCurrent(CURRENT_450MA);
  M5.Axp.EnableCoulombcounter();
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);

  // text print
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("rainbow!");
  M5.Lcd.setCursor(10,25);
  M5.Lcd.printf("%.2fV",M5.Axp.GetBatVoltage());
  M5.Lcd.setCursor(10,40);
  M5.Lcd.printf("%.2fmA",M5.Axp.GetBatCurrent());
  rainbow(5);             // Flowing rainbow cycle along the whole strip
}

// the loop routine runs over and over again forever
void loop(){
  M5.update();
  colorWipe(strip.Color(255,   0,   0)     , 15); // Red
  updateScreen();
  colorWipe(strip.Color(  0, 255,   0)     , 15); // Green
  updateScreen();
  colorWipe(strip.Color(  0,   0, 255)     , 15); // Blue
  updateScreen();

//  // 0x01 long press(1s), 0x02 press
//  if(M5.Axp.GetBtnPress() == 0x02) 
//  {
//        // close voltage output
//        //M5.Axp.PowerOff();
//        //M5.Axp.SetSleep(); 
//        //output5v_ctrl(true);
//  }
}

void updateScreen(){
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("Battery!");
  M5.Lcd.setCursor(10,25);
  M5.Lcd.printf("%.2fV",M5.Axp.GetBatVoltage());
  M5.Lcd.setCursor(10,40);
  M5.Lcd.printf("%.2fmA",M5.Axp.GetBatCurrent());
}
// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
