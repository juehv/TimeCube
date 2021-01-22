#include <Arduino.h>
 
#include "wifi_config.h"
#include <WiFi.h>

#define NEOPIXEL_PIN 0
#define NEOPIXEL_NUMPIXELS 7
#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define PIEZO_PIN 18

#define ADC_PIN 35

//TODO
// * time tracking
// * better beeping (in second thread)
// * submit events through wifi
// * setup web server (for wifi nw / labels for cube sides)
// * print a cube
// * basic battery management
// * applications ?

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

/* Time Tracking Cube States
 * 1: Cube Tilted Left
 * 2: Cube Tilted Right
 * 3: Cube Neutral, Top
*/
int cubeState = 0;
 
// Previous cube orientation state
int prvCubeState = 0;

void setup() {
  
  Serial.begin(115200);
  
  ledcSetup(0,200.0,8);
  ledcAttachPin(PIEZO_PIN, 0);

  // WIFI
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.println("Connecting to WiFi..");
  // }
 
  // Serial.println("Connected to the WiFi network");

  // NEOPIXEL
  pixels.begin();
  pixels.show();
  pixels.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  // Initialize LIS3DH
  if (!lis.begin(0x18))
  {
    Serial.println("Couldnt initialize accelerometer");
    while (1)
      ;
  }
  Serial.println("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_4_G);
  
  delay(500);

}


// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
    pixels.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    pixels.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}



int bat  = 0;
void loop() {

  // colorWipe(pixels.Color(255,   0,   0), 50); // Red
  // colorWipe(pixels.Color(  0, 255,   0), 50); // Green
  
  // ledcWriteTone(0, 2000);
  
  // for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle=dutyCycle+10){
  
  //   Serial.println(dutyCycle);
  
  //   ledcWrite(0, dutyCycle);
  //   delay(200);
  // }
  
  // ledcWrite(0, 125);
  
  // for (int freq = 255; freq < 10000; freq = freq + 250){
  
  //    Serial.println(freq);
  
  //    ledcWriteTone(0, freq);
  //    delay(200);
  // }
  
  bat = analogRead(ADC_PIN);
  
  // Get a normalized sensor reading
  sensors_event_t event;
  lis.getEvent(&event);

  Serial.print("x: ");
  Serial.print(event.acceleration.x); 
  Serial.print(", y: ");
  Serial.print(event.acceleration.y); 
  Serial.print(", z: ");
  Serial.print(event.acceleration.z); 
 
  Serial.print(", bat: ");  
  Serial.println(bat);
  // Detect cube face orientation
  if (event.acceleration.x > 8.0 && event.acceleration.x < 11.0)
  {
    //Serial.println("Cube TILTED: Left");
    cubeState = 1;
  }
  else if (event.acceleration.y > 8.0 && event.acceleration.y < 11.0)
  {
    //Serial.println("Cube TILTED: Right");
    cubeState = 2;
  }
  else if (event.acceleration.z > 8.0 && event.acceleration.z < 11.0)
  {
    cubeState = 3;
  }
  else
  { // orientation not specified
    //Serial.println("Cube Idle...");
    
    cubeState = 6;
  }
 
  // return if the orientation hasn't changed
  if (cubeState == prvCubeState)
    return;

  // Fill along the length of the strip in various colors...
  //colorWipe(pixels.Color(255,   0,   0), 50); // Red
  //colorWipe(pixels.Color(  0, 255,   0), 50); // Green
  //colorWipe(pixels.Color(  0,   0, 255), 50); // Blue
  // Send to Adafruit IO based off of the orientation of the cube
  switch (cubeState)
  {
  case 1:
    Serial.println("Switching to Task 1");
    // update the neopixel strip
    colorWipe(pixels.Color(255,   0,   0), 50); // Red
    
    ledcWriteTone(0, 2000);
    delay(50);
    ledcWriteTone(0, 0);

    break;
  case 2:
    Serial.println("Switching to Task 2");
    // update the neopixel strip
    colorWipe(pixels.Color(  0, 255,   0), 50); // Green
    
    ledcWriteTone(0, 10000);
    delay(50);
    ledcWriteTone(0, 0);
  
    break;
  case 3:
    Serial.println("Switching to Task 3");
    colorWipe(pixels.Color(  0,   0, 255), 50); // Blue
    
    ledcWriteTone(0, 800);
    delay(50);
    ledcWriteTone(0, 0);
   
    break;
  default:
    colorWipe(pixels.Color(  0,   0,  0), 50);
  }
 
  // save cube state
  prvCubeState = cubeState;
 
  // Delay the send to Adafruit IO
  delay(250);

}