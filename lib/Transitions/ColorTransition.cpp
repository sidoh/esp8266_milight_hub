#include <ColorTransition.h>
#include <Arduino.h>

ColorTransition::Builder::Builder(size_t id, uint16_t defaultPeriod, const BulbId& bulbId, TransitionFn callback, const ParsedColor& start, const ParsedColor& end)
  : Transition::Builder(id, defaultPeriod, bulbId, callback, calculateMaxDistance(start, end))
  , start(start)
  , end(end)
{ }

std::shared_ptr<Transition> ColorTransition::Builder::_build() const {
  size_t duration = getOrComputeDuration();
  size_t numPeriods = getOrComputeNumPeriods();
  size_t period = getOrComputePeriod();

  return std::make_shared<ColorTransition>(
    id,
    bulbId,
    start,
    end,
    duration,
    period,
    numPeriods,
    callback
  );
}

ColorTransition::RgbColor::RgbColor()
  : r(0)
  , g(0)
  , b(0)
{ }

ColorTransition::RgbColor::RgbColor(const ParsedColor& color)
  : r(color.r)
  , g(color.g)
  , b(color.b)
{ }

ColorTransition::RgbColor::RgbColor(int16_t r, int16_t g, int16_t b)
  : r(r)
  , g(g)
  , b(b)
{ }

bool ColorTransition::RgbColor::operator==(const RgbColor& other) {
  return r == other.r && g == other.g && b == other.b;
}

ColorTransition::ColorTransition(
  size_t id,
  const BulbId& bulbId,
  const RgbColor& startColor,
  const RgbColor& endColor,
  size_t duration,
  size_t period,
  size_t numPeriods,
  TransitionFn callback
) : Transition(id, bulbId, period, std::move(callback))
  , endColor(endColor)
  , currentColor(startColor)
  , stepSizes(
      calculateStepSizePart(endColor.r - startColor.r, duration, period),
      calculateStepSizePart(endColor.g - startColor.g, duration, period),
      calculateStepSizePart(endColor.b - startColor.b, duration, period))
  , lastHue(400) // use impossible values to force a packet send
  , lastSaturation(200)
  , sentFinalColor(false)
{ }

size_t ColorTransition::calculateMaxDistance(const RgbColor& start, const RgbColor& end) {
  // return max distance between any of R/G/B
  return std::max(
    std::max(
      std::abs(start.r - end.r),
      std::abs(start.g - end.g)
    ),
    std::abs(start.b - end.b)
  );
}

size_t ColorTransition::calculateColorPeriod(ColorTransition* t, const RgbColor& start, const RgbColor& end, size_t stepSize, size_t duration) {
  return Transition::calculatePeriod(calculateMaxDistance(start, end), stepSize, duration);
}

int16_t ColorTransition::calculateStepSizePart(int16_t distance, size_t duration, size_t period) {
  double stepSize = (distance / static_cast<double>(duration)) * period;
  int16_t rounded = std::ceil(std::abs(stepSize));

  if (distance < 0) {
    rounded *= -1;
  }

  return rounded;
}

void ColorTransition::step() {
  ParsedColor parsedColor = ParsedColor::fromRgb(currentColor.r, currentColor.g, currentColor.b);

  if (parsedColor.hue != lastHue) {
    callback(bulbId, GroupStateField::HUE, parsedColor.hue);
    lastHue = parsedColor.hue;
  }

  if (parsedColor.saturation != lastSaturation) {
    callback(bulbId, GroupStateField::SATURATION, parsedColor.saturation);
    lastSaturation = parsedColor.saturation;
  }

  if (!isFinished()) {
    Transition::stepValue(currentColor.r, endColor.r, stepSizes.r);
    Transition::stepValue(currentColor.g, endColor.g, stepSizes.g);
    Transition::stepValue(currentColor.b, endColor.b, stepSizes.b);
  } else {
    this->sentFinalColor = true;
  }
}

bool ColorTransition::isFinished() {
  return this->sentFinalColor;
}

void ColorTransition::childSerialize(JsonObject& json) {
  json[F("type")] = F("color");

  JsonArray currentColorArr = json.createNestedArray(F("current_color"));
  currentColorArr.add(currentColor.r);
  currentColorArr.add(currentColor.g);
  currentColorArr.add(currentColor.b);

  JsonArray endColorArr = json.createNestedArray(F("end_color"));
  endColorArr.add(endColor.r);
  endColorArr.add(endColor.g);
  endColorArr.add(endColor.b);

  JsonArray stepSizesArr = json.createNestedArray(F("step_sizes"));
  stepSizesArr.add(stepSizes.r);
  stepSizesArr.add(stepSizes.g);
  stepSizesArr.add(stepSizes.b);
}
