#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
 
#include "wifi_config.h"
#include <WiFi.h>

#define NEOPIXEL_PIN 0
#define NEOPIXEL_NUMPIXELS 14
#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <LIS3DSH.h>

#define PIEZO_PIN 18

#define ADC_PIN 35

#define uS_TO_S_FACTOR 1000000 

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

/* Time Tracking Cube States
 * 1: Cube Tilted Left
 * 2: Cube Tilted Right
 * 3: Cube Neutral, Top
 * 
 * -1: uninitialized
 * 0: power off (usb on top)
 * 1: idle (usb on bottom)
 * 
*/
int8_t cubeState = -1;
// Previous cube orientation state (in RTC Memory)
RTC_DATA_ATTR int8_t prvCubeState = 0;

// battery voltage
uint16_t bat  = 0;

// save system counter over short sleeps
RTC_DATA_ATTR unsigned long timeCounterMillis = 0;

inline bool isInRagen(float value, float plusMinus){
  return value > (value - plusMinus) && value < (value + plusMinus);
}

inline void detectCubeFaceOrientation(sensors_event_t event){
  // Detect cube face orientation
  if (event.acceleration.x > 18.0 && event.acceleration.x < 22.0
            && event.acceleration.y > -2.0 && event.acceleration.y < 2.0
            && event.acceleration.z > -2.0 && event.acceleration.z < 2.0)
  {
    log_i("usb on top --> power off");
    cubeState = 0;
  }
  else if (event.acceleration.x < -18.0 && event.acceleration.x > -22.0
            && event.acceleration.y > -2.0 && event.acceleration.y < 2.0
            && event.acceleration.z > -2.0 && event.acceleration.z < 2.0)
  {
    log_i("usb on bottom --> idle/power on");
    if (prvCubeState == 0){
      cubeState = 1;
    } else {
      cubeState = 2;
    }
  }
  else if (event.acceleration.y > 18.0 && event.acceleration.y < 22.0
            && event.acceleration.x > -2.0 && event.acceleration.x < 2.0
            && event.acceleration.z > -2.0 && event.acceleration.z < 2.0)
  {
    log_i("state flat 3");
    cubeState = 3;
  }
  else if (event.acceleration.y < -18.0 && event.acceleration.y > -22.0
            && event.acceleration.x > -2.0 && event.acceleration.x < 2.0
            && event.acceleration.z > -2.0 && event.acceleration.z < 2.0)
  {
    log_i("state flat 4");
    cubeState = 4;
  }
  else if (event.acceleration.z > 18.0 && event.acceleration.z < 22.0
            && event.acceleration.x > -2.0 && event.acceleration.x < 2.0
            && event.acceleration.y > -2.0 && event.acceleration.y < 2.0)
  {
    log_i("state flat 5");
    cubeState = 5;
  }
  else if (event.acceleration.z < -18.0 && event.acceleration.z > -22.0
            && event.acceleration.x > -2.0 && event.acceleration.x < 2.0
            && event.acceleration.y > -2.0 && event.acceleration.y < 2.0)
  {
    log_i("state flat 6");
    cubeState = 6;
  }
  else if (event.acceleration.x < -10.0 && event.acceleration.x > -14.0
            && event.acceleration.y < -10.0 && event.acceleration.y > -14.0
            && event.acceleration.z < -10.0 && event.acceleration.z > -14.0)
  {
    log_i("state corner 1 (7)");
    cubeState = 7;
  }
  else if (event.acceleration.x > 10.0 && event.acceleration.x < 14.0
            && event.acceleration.y > 10.0 && event.acceleration.y < 14.0
            && event.acceleration.z > 10.0 && event.acceleration.z < 14.0)
  {    
    log_i("state corner 1-opposide (8)");
    cubeState = 8;
  }
  else if (event.acceleration.x > 10.0 && event.acceleration.x < 14.0
            && event.acceleration.y < -10.0 && event.acceleration.y > -14.0
            && event.acceleration.z < -10.0 && event.acceleration.z > -14.0)
  {
    log_i("state corner 2 (9)");
    cubeState = 9;
  }
  else if (event.acceleration.x < -10.0 && event.acceleration.x > -14.0
            && event.acceleration.y > 10.0 && event.acceleration.y < 14.0
            && event.acceleration.z > 10.0 && event.acceleration.z < 14.0)
  {    
    log_i("state corner 2-opposide (10)");
    cubeState = 10;
  }
  else if (event.acceleration.y > 10.0 && event.acceleration.y < 14.0
            && event.acceleration.x < -10.0 && event.acceleration.x > -14.0
            && event.acceleration.z < -10.0 && event.acceleration.z > -14.0)
  {
    log_i("state corner 3 (11)");
    cubeState = 11;
  }
  else if (event.acceleration.y < -10.0 && event.acceleration.y > -14.0
            && event.acceleration.x > 10.0 && event.acceleration.x < 14.0
            && event.acceleration.z > 10.0 && event.acceleration.z < 14.0)
  {    
    log_i("state corner 3-opposide (12)");
    cubeState = 12;
  } 
  else if (event.acceleration.z > 10.0 && event.acceleration.z < 14.0
            && event.acceleration.y < -10.0 && event.acceleration.y > -14.0
            && event.acceleration.x < -10.0 && event.acceleration.x > -14.0)
  {
    log_i("state corner 4 (13)");
    cubeState = 13;
  }
  else if (event.acceleration.z < -10.0 && event.acceleration.z > -14.0
            && event.acceleration.y > 10.0 && event.acceleration.y < 14.0
            && event.acceleration.x > 10.0 && event.acceleration.x < 14.0)
  {    
    log_i("state corner 4-opposide (14)");
    cubeState = 14;
  }  
  // TODO add edges here
  else
  { 
    log_w("unknown orientation");    
    cubeState = -2;
  }
}

//#####################################
//#####################################
// LED Management
//#####################################
//#####################################

RTC_DATA_ATTR uint32_t lastColor = 0;
// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  lastColor = color;
  for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
    pixels.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    pixels.setBrightness(10);
    if (wait > 0){
      pixels.show();                          //  Update strip to match
      delay(wait);                           //  Pause for a moment
    }
  }
  if (wait == 0){
    pixels.show();
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
    pixels.setBrightness(10);
    pixels.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

//#####################################
//#####################################
// END LED Management
//#####################################
//#####################################

//#####################################
//#####################################
// beep Management
//#####################################
//#####################################

void beepBatteryDown(){  
    ledcWriteTone(0, 2000);
    delay(50);
    ledcWriteTone(0, 1000);
    delay(50);
    ledcWriteTone(0, 800);
    delay(50);
    ledcWriteTone(0, 400);
    delay(50);
    ledcWriteTone(0, 0);
}

void beepBatteryLow(){  
    ledcWriteTone(0, 2000);
    delay(50);
    ledcWriteTone(0, 1000);
    delay(50);
    ledcWriteTone(0, 800);
    delay(50);
    ledcWriteTone(0, 400);
    delay(50);
    ledcWriteTone(0, 0);
}

void beepWakeup(){
    ledcWriteTone(0, 400);
    delay(50);
    ledcWriteTone(0, 800);
    delay(50);
    ledcWriteTone(0, 1000);
    delay(50);  
    ledcWriteTone(0, 2000);
    delay(50);
    ledcWriteTone(0, 0);
}


void beepPowerdown(){
    ledcWriteTone(0, 2000);
    delay(50);
    ledcWriteTone(0, 1000);
    delay(50);
    ledcWriteTone(0, 800);
    delay(50);
    ledcWriteTone(0, 400);
    delay(50);
    ledcWriteTone(0, 0);
}

void beepStateEngaged(){    
    ledcWriteTone(0, 4000);
    delay(50);
    ledcWriteTone(0, 0);
}

//#####################################
//#####################################
// END beep Management
//#####################################
//#####################################

//#####################################
//#####################################
// POWER Management
//#####################################
//#####################################

#define BAT_OFF_TH        1950
#define BAT_CRITICAL_TH   2050
#define BAT_DEAD_CNT_RST  -100
RTC_DATA_ATTR int8_t batteryDead = BAT_DEAD_CNT_RST; // ==reset_value good, <0 counting on dead, >0 warning, ==0 fast exit because of dead

void goToShortDeepSleep(){  
  esp_sleep_enable_timer_wakeup(0.7 * uS_TO_S_FACTOR); // half second
  timeCounterMillis += millis() + 0.7 * uS_TO_S_FACTOR / 1000;
  esp_deep_sleep_start();
}

void goToPowerDown(){
  lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);
  esp_sleep_enable_timer_wakeup(10 * uS_TO_S_FACTOR); // 30 seconds
  timeCounterMillis += millis() + 10 * uS_TO_S_FACTOR / 1000;
  esp_deep_sleep_start();
}

void goToBatteryDeadPowerDown(){
  colorWipe(pixels.Color(0,   0,   0), 50); // off
  lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(30 * 60 * uS_TO_S_FACTOR); // 30 Minutes
  esp_deep_sleep_start();
}

void scheduleBatteryManagement(){
  if (bat < BAT_OFF_TH){
    log_i("BATTERY very low");
    batteryDead += 10;
    if (batteryDead >= 0){
      log_i("--> shutdown");
      // enable fast exit
      batteryDead = 0;
      // power down
      beepBatteryDown();
      goToBatteryDeadPowerDown();
    }
  } else if (bat < BAT_CRITICAL_TH){
    log_i("BATTERY low");
    batteryDead++;
    // action (warning) if longer than one minute under theashold and repeat regularly
    if (batteryDead == 0 || batteryDead > 100){ // roughly reset every minute
      batteryDead = 1;
    }
    if (batteryDead == 1){
      log_i("--> warning");      
      uint32_t tmpColor = lastColor;
      colorWipe(pixels.Color(255,   0,   0), 50); // Red
      beepBatteryLow();
      delay(1000);
      colorWipe(tmpColor, 0);
    }
  } else {
    batteryDead = BAT_DEAD_CNT_RST;
  }
}

//#####################################
//#####################################
// END POWER Management
//#####################################
//#####################################

//#####################################
//#####################################
// WEB Management
//#####################################
//#####################################
// Set web server port number to 80
WiFiServer server(80);
String header="";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


void setupWebServer(){  
  // Connect to Wi-Fi network with SSID and password
  log_i("Connect to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    log_v(".");
  }
  // Print local IP address and start web server
  log_i("WiFi connected.");
  log_i("IP address: %s", WiFi.localIP());
  server.begin();
}

void scheduleWebServerRoutine(){
  


WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.print("<p>bat value:");
            client.print(bat);
            client.println(" </p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }

}

//#####################################
//#####################################
// END WEB Management
//#####################################
//#####################################
#endif