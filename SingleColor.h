#ifndef SingleColor_H
#define SingleColor_H
#include "Effect.h"

class SingleColor : public Effect {
  public:
    virtual boolean update(Adafruit_NeoPixel &strip) {
      tick();
      if (_alpha <= 1.0f) {
        lerpState();
        int color = strip.Color(_currentState.red, _currentState.green, _currentState.blue);
        for (uint16_t i=0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, color);
        }
        strip.setBrightness(_currentState.brightness);
        return true;
      } else {
        return false;
      }
    }

    virtual String name() {
      return "SingleColor";  
    }
};

#endif
