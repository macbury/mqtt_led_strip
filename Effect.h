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
    Effect();
    void update(Adafruit_NeoPixel &strip);
    void begin(LedState targetState);
    String name();
    LedState getCurrentState();
  private:
    float _alpha;
    LedState _targetState;
    LedState _currentState;
    LedState _initialState;
};
#endif
