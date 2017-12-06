#ifndef DualColor_H
#define DualColor_H
#include "Effect.h"

class DualColor : public Effect {
  public:
    virtual boolean update(Adafruit_NeoPixel &strip) {;
      tick();
      lerpState();

      float pixelCount = strip.numPixels();
      for (int i=0; i < pixelCount; i++) {
        float xi = map(((float)i/pixelCount) * 100, 0, 100, -100, 100) / 100.0;
        float power = pow(xi, 4);
        int color = strip.Color(
          lerp(power, _currentState.red, _initialState.red), 
          lerp(power, _currentState.green, _initialState.green), 
          lerp(power, _currentState.blue, _initialState.blue) 
        );

        strip.setPixelColor(i, color);
      }
      strip.setBrightness(_currentState.brightness);
      strip.show();
      return _currentState.brightness > 0.0;
    }

    virtual String name() {
      return "DualColor";  
    }
};

#endif
