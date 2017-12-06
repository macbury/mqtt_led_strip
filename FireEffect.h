#ifndef FireEffect_H
#define FireEffect_H
#include "Effect.h"

class FireEffect : public Effect {
  public:
    virtual boolean update(Adafruit_NeoPixel &strip) {
      _time += 0.1;
      tick();
      lerpState();
        
      int r = 255;
      int g = 160;
      int b = 40;

      for (uint16_t i=0; i < strip.numPixels(); i++) {
        int flicker = random(0,150);
        int r1 = r-flicker;
        int g1 = g-flicker;
        int b1 = b-flicker;
        if(g1<0) g1=0;
        if(r1<0) r1=0;
        if(b1<0) b1=0;
        strip.setPixelColor(i, r1,g1,b1);
      }
      strip.setBrightness(_currentState.brightness);
      strip.show();
      delay(random(17,117));
      return _currentState.brightness > 0.0;
    }

    virtual String name() {
      return "FireEffect";  
    }
  private:
    float _time;
};

#endif