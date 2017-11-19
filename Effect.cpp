#include "Effect.h"

byte lerp(float progress, float fromValue, float toValue){
  return (int)(fromValue + (toValue - fromValue) * progress);
}

Effect::Effect(){
  _initialState = { 0, 0, 0, 0, false };
  _currentState = _initialState;
  _targetState = _initialState;
  _alpha = 0.0f;
}

void Effect::update(Adafruit_NeoPixel &strip) {
  _alpha += 0.01f;
  if (_alpha >= 1.0f) {
    _alpha = 1.0f; 
  }
  
  _currentState.red = lerp(_alpha, _initialState.red, _targetState.red);
  _currentState.green = lerp(_alpha, _initialState.green, _targetState.green);
  _currentState.blue = lerp(_alpha, _initialState.blue, _targetState.blue);
  _currentState.brightness = lerp(_alpha, _initialState.brightness, _targetState.brightness);
  _currentState.enabled = _initialState.enabled;
  
  int color = strip.Color(_currentState.red, _currentState.green, _currentState.blue);
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.setBrightness(_currentState.brightness);
}

LedState Effect::getCurrentState() {
  return _currentState;  
}

String Effect::name() {
  return "Fade";  
}

void Effect::begin(LedState targetState) {
  _initialState = _currentState;
  _targetState = targetState;
  _alpha = 0.0f;
}

void Effect::end() {
  _initialState = _currentState;
  _targetState = { 0, 0, 0, 0, false };
  _alpha = 0.0f;
}

