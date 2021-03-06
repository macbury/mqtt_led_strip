#ifndef SingleColor_H
#define SingleColor_H
#include "Effect.h"

class SingleColor : public Effect {
  public:
    virtual boolean update(Adafruit_NeoPixel &strip) {
      tick();
      lerpState();
      int color = strip.Color(_currentState.red, _currentState.green, _currentState.blue);
      for (uint16_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
      }
      strip.setBrightness(_currentState.brightness);
      strip.show();
      return _currentState.brightness > 0.0;
    }

    virtual String name() {
      return "SingleColor";  
    }
};

#endif
