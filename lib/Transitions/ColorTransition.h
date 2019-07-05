#include <Transition.h>
#include <ParsedColor.h>

#pragma once

class ColorTransition : public Transition {
public:
  ColorTransition(
    size_t id,
    const BulbId& bulbId,
    const ParsedColor& startColor,
    const ParsedColor& endColor,
    uint16_t stepSize,
    size_t duration,
    TransitionFn callback
  );

  static size_t calculateColorPeriod(ColorTransition* t, const ParsedColor& start, const ParsedColor& end, size_t stepSize, size_t duration);
  inline static int16_t calculateStepSizePart(int16_t distance, size_t duration, size_t period);
  virtual bool isFinished() override;

protected:
  struct RgbColor {
    RgbColor();
    RgbColor(const ParsedColor& color);
    bool operator==(const RgbColor& other);

    int16_t r, g, b;
  };

  const RgbColor endColor;
  RgbColor currentColor;
  RgbColor stepSizes;

  // Store these to avoid wasted packets
  uint16_t lastHue;
  uint16_t lastSaturation;

  virtual void step() override;
  virtual void childSerialize(JsonObject& json) override;
  static inline void stepPart(uint16_t& current, uint16_t end, int16_t step);
};