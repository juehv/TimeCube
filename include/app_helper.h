#ifndef APPS_H
#define APPS_H

#include <Arduino.h>

#include "system_helper.h"

// idle function
void scheduleIdle(){
    rainbow(5);
}


// timer
RTC_DATA_ATTR unsigned long usrTimerEnd = 0; 

void setupTimer(uint16_t runtimeInSeconds){
    usrTimerEnd = timeCounterMillis + runtimeInSeconds*1000;
    log_i("set user timer to %d seconds (%d)", runtimeInSeconds, usrTimerEnd);
    // todo implement beeper depending un time
     
    ledcWriteTone(0, 4000);
    delay(50);
    ledcWriteTone(0, 0);
    delay(20);
    ledcWriteTone(0, 4000);
    delay(50);
    ledcWriteTone(0, 0);
    delay(20);
    ledcWriteTone(0, 4000);
    delay(50);
    ledcWriteTone(0, 0);
    delay(20);
}
void scheduleTimer(){
    if (usrTimerEnd != 0){
        if (timeCounterMillis >= usrTimerEnd){        
            colorWipe(pixels.Color(255,   0,   0), 50); // red
            ledcWriteTone(0, 4000);
            delay(50);
            ledcWriteTone(0, 0);
            delay(20);
            ledcWriteTone(0, 4000);
            delay(50);
            ledcWriteTone(0, 0);
            delay(200);
            ledcWriteTone(0, 4000);
            delay(50);
            ledcWriteTone(0, 0);
            delay(20);
            ledcWriteTone(0, 4000);
            delay(50);
            ledcWriteTone(0, 0);
            delay(200);
            ledcWriteTone(0, 4000);
            delay(50);
            ledcWriteTone(0, 0);
            delay(20);
            ledcWriteTone(0, 4000);
            delay(50);
            ledcWriteTone(0, 0);
            delay(200);

            usrTimerEnd = 0;
            colorWipe(pixels.Color(0,   0,   0), 100);
        } else {
            colorWipe(pixels.Color(0,   255,   0), 100); 
            colorWipe(pixels.Color(0,   0,   0), 0);
        }        
    }
}

// management
void scheduleRunningApp(int8_t state){
    switch(state){
        case 2:
            scheduleIdle();
            break;
        case 4:
            scheduleTimer();
            break;
        
    }
}

#endif