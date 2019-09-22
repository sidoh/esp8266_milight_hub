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

  int16_t dr = end.r - start.r
        , dg = end.g - start.g
        , db = end.b - start.b;

  RgbColor stepSizes(
    calculateStepSizePart(dr, duration, period),
    calculateStepSizePart(dg, duration, period),
    calculateStepSizePart(db, duration, period)
  );

  return std::make_shared<ColorTransition>(
    id,
    bulbId,
    start,
    end,
    stepSizes,
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
  const ParsedColor& startColor,
  const ParsedColor& endColor,
  RgbColor stepSizes,
  size_t duration,
  size_t period,
  size_t numPeriods,
  TransitionFn callback
) : Transition(id, bulbId, period, callback)
  , endColor(endColor)
  , currentColor(startColor)
  , stepSizes(stepSizes)
  , lastHue(400)         // use impossible values to force a packet send
  , lastSaturation(200)
  , finished(false)
{
  int16_t dr = endColor.r - startColor.r
        , dg = endColor.g - startColor.g
        , db = endColor.b - startColor.b;
  // Calculate step sizes in terms of the period
  stepSizes.r = calculateStepSizePart(dr, duration, period);
  stepSizes.g = calculateStepSizePart(dg, duration, period);
  stepSizes.b = calculateStepSizePart(db, duration, period);
}

size_t ColorTransition::calculateMaxDistance(const ParsedColor& start, const ParsedColor& end) {
  int16_t dr = end.r - start.r
        , dg = end.g - start.g
        , db = end.b - start.b;

  int16_t max = std::max(std::max(dr, dg), db);
  int16_t min = std::min(std::min(dr, dg), db);
  int16_t maxAbs = std::abs(min) > std::abs(max) ? min : max;

  return maxAbs;
}

size_t ColorTransition::calculateColorPeriod(ColorTransition* t, const ParsedColor& start, const ParsedColor& end, size_t stepSize, size_t duration) {
  return Transition::calculatePeriod(calculateMaxDistance(start, end), stepSize, duration);
}

int16_t ColorTransition::calculateStepSizePart(int16_t distance, size_t duration, size_t period) {
  double stepSize = (distance / static_cast<double>(duration)) * period;
  int16_t rounded = std::ceil(std::abs(stepSize));

  if (distance < 0) {
    rounded = -rounded;
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

  if (currentColor == endColor) {
    finished = true;
  } else {
    Transition::stepValue(currentColor.r, endColor.r, stepSizes.r);
    Transition::stepValue(currentColor.g, endColor.g, stepSizes.g);
    Transition::stepValue(currentColor.b, endColor.b, stepSizes.b);
  }
}

bool ColorTransition::isFinished() {
  return finished;
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