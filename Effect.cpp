#include "Effect.h"

Effect::Effect(LedState initialState){
  _internalState = initialState;
}

void Effect::update(Adafruit_NeoPixel &strip, LedState state) {
  int color = strip.Color(state.red, state.green, state.blue);
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    if (state.enabled) {
      strip.setPixelColor(i, color);
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  if (state.enabled) {
    strip.setBrightness(state.brightness);
  } else {
    strip.setBrightness(0);
  }
}

String Effect::name() {
  return "fade";  
}
