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
    Effect(LedState initialState);
    void update(Adafruit_NeoPixel &strip, LedState state);
    String name();
  private:
    LedState _internalState;
};
#endif
