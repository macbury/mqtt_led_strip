#ifndef SinColor_H
#define SinColor_H
#include "Effect.h"

class SinColor : public Effect {
  public:
    virtual boolean update(Adafruit_NeoPixel &strip) {
      _time += 0.1;
      tick();
      lerpState();
      
      for (uint16_t i=0; i < strip.numPixels(); i++) {
        float wave = sin(i + _time);
        if (wave < 0.0) {
          wave = 0.0;  
        }
        int color = strip.Color(wave * _currentState.red, wave * _currentState.green, wave * _currentState.blue);
        
        strip.setPixelColor(i, color);
      }
      strip.setBrightness(_currentState.brightness);
    
      return false;
    }

    virtual String name() {
      return "SinColor";  
    }
  private:
    float _time;
};

#endif
