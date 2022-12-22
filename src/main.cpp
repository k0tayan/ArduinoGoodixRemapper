
#include <Wire.h>
#include "Goodix.h"
#include "HID-Project.h"

#define INT_PIN 0
#define RST_PIN 8

#define XY_SWAP 0
#define X_INVERT 1
#define Y_INVERT 1
#define Y_UP 0

const uint32_t TOUCH_SCREEN_MIN_X = 1000;
const uint32_t TOUCH_SCREEN_MAX_X = 9000;
const uint32_t TOUCH_SCREEN_Y = Y_UP ? 2000 : 8000;

const uint32_t TOUCH_PANEL_SIZE_X = 1920.;
const uint32_t TOUCH_PANEL_SIZE_Y = 515.;

float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh);

Goodix touch = Goodix();

void handleTouch(int8_t contacts, GTPoint *points) {
  static bool curTrackIds[10] = {0};
  static bool prevTrackIds[10] = {0};
  for(int i=0; i<contacts; i++){
    float x, y;
    if(!X_INVERT)
      x = mapFloat(touch.points[i].x, 0., TOUCH_PANEL_SIZE_X, TOUCH_SCREEN_MIN_X, TOUCH_SCREEN_MAX_X);
    else
      x = mapFloat(touch.points[i].x, TOUCH_PANEL_SIZE_X, 0., TOUCH_SCREEN_MIN_X, TOUCH_SCREEN_MAX_X);
    if(!Y_INVERT)
      y = mapFloat(touch.points[i].y, 0., TOUCH_PANEL_SIZE_Y, TOUCH_SCREEN_Y-1000, TOUCH_SCREEN_Y+1000);
    else
      y = mapFloat(touch.points[i].y, TOUCH_PANEL_SIZE_Y, 0., TOUCH_SCREEN_Y-1000, TOUCH_SCREEN_Y+1000);
    if(XY_SWAP){
      float tmp = x;
      x = y;
      y = tmp;
    }
    Touchscreen.setFinger(touch.points[i].trackId, x, y, 100);
    curTrackIds[touch.points[i].trackId] = true;
  }
  for(int i=0; i<10; i++){
    if(prevTrackIds[i] == true && curTrackIds[i] == false)
      Touchscreen.releaseFinger(i);
    prevTrackIds[i] = curTrackIds[i];
    curTrackIds[i] = false;
  }

  // 現在のタッチ状況を出力
  int state = 0;
  for(int i=0; i<contacts; i++){
    state |= 1 << touch.points[i].trackId;
  }
  for(int i=0; i<10; i++){
    if(state & (1 << i))
      Serial.print(1);
    else
      Serial.print(0);
  }
    Serial.println();
} 

void touchStart() {
 if (touch.begin(INT_PIN, RST_PIN)!=true) {
    Serial.println("! Module reset failed");
  } else {
    Serial.println("Module reset OK");
  }
  
  Serial.print("Check ACK on addr request on 0x");
  Serial.print(touch.i2cAddr, HEX);
  
  Wire.beginTransmission(touch.i2cAddr);  
  int error = Wire.endTransmission();
  if (error == 0) {    
    Serial.println(": SUCCESS");   
  } else {
    Serial.print(": ERROR #");
    Serial.println(error);
  }
}

float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow; 
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nGoodix GT911x touch driver");

  Wire.setClock(400000);
  Wire.begin();
  delay(300);

  touch.setHandler(handleTouch);
  touchStart();
}


void loop() {
  touch.loop();
  Touchscreen.send();
  delayMicroseconds(250);
}