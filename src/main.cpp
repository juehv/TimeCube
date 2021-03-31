#include <Arduino.h>
 
#include "wifi_config.h"

#include "system_helper.h"
#include "app_helper.h"

// hints
// original prject see https://learn.adafruit.com/time-tracking-cube/overview
// We used a different accelerometer --> LIS3DSH (instead of LIS3DH)
// We try to use adafruit library as the sensor is very similar
// Alternative library --> https://github.com/yazug/LIS3DSH
// logging see https://thingpulse.com/esp32-logging/
// infos for deep slee --> https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
// infos for interrupts --> https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library/blob/master/examples/IntUsage/IntUsage.ino
// and --> https://learn.sparkfun.com/tutorials/lis3dh-hookup-guide/all#example-i2c-analog-and-interrupts

//TODO
// * update code architecture
// * better beeping (in second thread)
// * prepare standard led actions as functions
// * submit events through wifi / BT
// * setup web server (for wifi nw / labels for cube sides)
// * applications ?
// * documentation
// * blog article

void setup() {  
  //Serial.begin(115200);

  // check battery voltage  
  bat = analogRead(ADC_PIN);
  // fast exit for dead battery
  if (batteryDead == 0 && bat < BAT_CRITICAL_TH){
    log_w("Exit cause of dead battery");
    goToBatteryDeadPowerDown();
  }

  // Initialize LIS3D*S*H
  if (!lis.begin(0x1d, 0x3F)){
    while(1){
      delay(1000);
      log_i("Couldnt initialize accelerometer");
    }
  }
  log_i("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setDataRate(LIS3DH_DATARATE_1_HZ);
  
  // NEOPIXEL
  pixels.begin();

  // PIEZO 
  ledcSetup(0,200.0,8);
  ledcAttachPin(PIEZO_PIN, 0);
  
  // Time tracking
  timeCounterMillis += millis();
  log_i("time counter: %d", timeCounterMillis);
}

void loop() {
  // battery management
  scheduleBatteryManagement();

  // Get a normalized sensor reading
  sensors_event_t event;
  if (!lis.getEvent(&event)){
    log_w("error reading sensor event.");
    delay(200);
    return;
  }

  log_i("x: %.2f, y: %.2f, z: %.2f, Bat: %d", 
    event.acceleration.x,
    event.acceleration.y,
    event.acceleration.z,
    bat);

  detectCubeFaceOrientation(event);
 
  // if the orientation hasn't changed, manage power states and schedule running apps  
  log_v("CUBE-STATE prev: %d, now: %d", prvCubeState, cubeState);
  if (cubeState == prvCubeState){
    if (cubeState == 0){
      // back to power off without animation
      log_i("keep power down");
      goToPowerDown();
    } else {
      scheduleRunningApp(cubeState);
      goToShortDeepSleep();
    }
    return;
  } else {
    if (prvCubeState == 0 && cubeState != 1){
      // no valud power on sequence --> keep sleeping
      log_i("no valid power on sequence --> keep sleeping");
      goToPowerDown();
    }
    
    if (cubeState >= 0){
      // save cube state if valid
      log_i("update cube state");
      prvCubeState = cubeState;
    }
  }

  // state transition --> aktivating new app
  switch (cubeState)
  {    
  case 0:
    log_i("Switching off cube");
    colorWipe(pixels.Color(255,   0,   0), 50); // Red
    beepPowerdown();
    colorWipe(pixels.Color(  0,   0,  0), 50);
    goToPowerDown();
    break;
    
  case 1:
    log_i("Wakeup Sequence");
    colorWipe(pixels.Color(255,   0,   0), 20); // Red
    colorWipe(pixels.Color(255,   255,   0), 20); // yellow
    colorWipe(pixels.Color(0,   255,   0), 20); // green
    beepWakeup();
    colorWipe(pixels.Color(  0,   0,  0), 50);
    break;

  case 2:
    log_i("idle sequence");
    scheduleIdle();
    beepStateEngaged();
    break;

  case 3:
    log_i("");
    colorWipe(pixels.Color(0,   255,   0), 50); // green
    beepStateEngaged();
    break;
        
  case 4:
    log_i("");
    colorWipe(pixels.Color(255,   0,   0), 50); // red
    setupTimer(20);
    beepStateEngaged();
    colorWipe(pixels.Color(0,   0,   0), 50); // red
    break;
        
  case 5:
    log_i("");
    colorWipe(pixels.Color(0,   0,   255), 50); // blue
    beepStateEngaged();
    break;
        
  case 6:
    log_i("");
    colorWipe(pixels.Color(255,   0,   255), 50); // lila
    beepStateEngaged();
    break;
  // default:
  //   colorWipe(pixels, pixels.Color(  0,   0,  0), 50);
  }
 
  // if we land here just go to a sleep mode
  log_v("finished loop");
  goToShortDeepSleep();
}