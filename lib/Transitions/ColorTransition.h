#include <Transition.h>
#include <ParsedColor.h>

#pragma once

class ColorTransition : public Transition {
public:
  struct RgbColor {
    RgbColor();
    RgbColor(const ParsedColor& color);
    RgbColor(int16_t r, int16_t g, int16_t b);
    bool operator==(const RgbColor& other);

    int16_t r, g, b;
  };

  class Builder : public Transition::Builder {
  public:
    Builder(size_t id, uint16_t defaultPeriod, const BulbId& bulbId, TransitionFn callback, const ParsedColor& start, const ParsedColor& end);

    virtual std::shared_ptr<Transition> _build() const override;

  private:
    const ParsedColor& start;
    const ParsedColor& end;
    RgbColor stepSizes;
  };

  ColorTransition(
    size_t id,
    const BulbId& bulbId,
    const ParsedColor& startColor,
    const ParsedColor& endColor,
    RgbColor stepSizes,
    size_t duration,
    size_t period,
    size_t numPeriods,
    TransitionFn callback
  );

  static size_t calculateColorPeriod(ColorTransition* t, const ParsedColor& start, const ParsedColor& end, size_t stepSize, size_t duration);
  inline static size_t calculateMaxDistance(const ParsedColor& start, const ParsedColor& end);
  inline static int16_t calculateStepSizePart(int16_t distance, size_t duration, size_t period);
  virtual bool isFinished() override;

protected:
  const RgbColor endColor;
  RgbColor currentColor;
  RgbColor stepSizes;

  // Store these to avoid wasted packets
  uint16_t lastHue;
  uint16_t lastSaturation;
  bool finished;

  virtual void step() override;
  virtual void childSerialize(JsonObject& json) override;
  static inline void stepPart(uint16_t& current, uint16_t end, int16_t step);
};