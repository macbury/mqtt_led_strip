#ifndef RainbowColor_H
#define RainbowColor_H
#include "Effect.h"

class RainbowColor : public Effect {
  public:
    virtual boolean update(Adafruit_NeoPixel &strip) {;
      tick();
      lerpState();
      _accumulator += 1;
      int step = 255 / strip.numPixels();
      for (uint16_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, wheel(strip, i*step+_accumulator));
      }
      strip.setBrightness(_currentState.brightness);
    
      return _currentState.brightness > 0.0;
    }

    virtual String name() {
      return "RainbowColor";  
    }
  private:
    int _accumulator;

    int wheel(Adafruit_NeoPixel &strip, byte wheelPos) {
      if(wheelPos < 85) {
        return strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
      } else if(wheelPos < 170) {
        wheelPos -= 85;
        return strip.Color(255 - wheelPos * 3, 0, wheelPos * 3);
      } else {
        wheelPos -= 170;
        return strip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
      }
    }
};

#endif
