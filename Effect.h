#ifndef Effect_H
#define Effect_H
#include <Adafruit_NeoPixel.h>
typedef struct {
  byte red;
  byte green;
  byte blue;
  byte brightness;
  boolean enabled;
} LedState;

/**
 * Base effect class
 */
class Effect {
  public:
    Effect() {
      _initialState = { 0, 0, 0, 0, false };
      _currentState = _initialState;
      _targetState = _initialState;
      _alpha = 0.0f;
    }

    /**
     * Run every tick, if done return true, updates state of strip
     */
    virtual boolean update(Adafruit_NeoPixel &strip) {
      return false;
    }
    
    void begin(LedState targetState) {
      _initialState = _currentState;
      _targetState = targetState;
      _alpha = 0.0f;  
    }
    
    void end() {
      _initialState = _currentState;
      _targetState = { 0, 0, 0, 0, false };
      _alpha = 0.0f;  
    }
    
    virtual String name() {
      return "Fake";  
    }
    
    LedState getCurrentState() {
      return _currentState;    
    }

    void resume(LedState currentState) {
      _currentState = _initialState = currentState;
    }

  protected:
    float _alpha;
    LedState _targetState;
    LedState _currentState;
    LedState _initialState;

    /**
     * Helper function
     */
    byte lerp(float progress, float fromValue, float toValue){
      return (int)(fromValue + (toValue - fromValue) * progress);
    }

    /**
     * Update state lerp
     */
    void lerpState() {
      float a = min(1.0, _alpha);
      _currentState.red = lerp(a, _initialState.red, _targetState.red);
      _currentState.green = lerp(a, _initialState.green, _targetState.green);
      _currentState.blue = lerp(a, _initialState.blue, _targetState.blue);
      _currentState.brightness = lerp(a, _initialState.brightness, _targetState.brightness);
      _currentState.enabled = _initialState.enabled;
    }

    void tick() {
      _alpha += 0.01f;
      _alpha = min(_alpha, 1.1f);
    }

    float min(float a, float b) {
      if (a > b) {
        return b;  
      } else {
        return a;
      }
    }
};
#endif
