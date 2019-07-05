#include <ColorTransition.h>
#include <Arduino.h>

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

bool ColorTransition::RgbColor::operator==(const RgbColor& other) {
  return r == other.r && g == other.g && b == other.b;
}

ColorTransition::ColorTransition(
  size_t id,
  const BulbId& bulbId,
  const ParsedColor& startColor,
  const ParsedColor& endColor,
  uint16_t stepSize,
  size_t duration,
  TransitionFn callback
) : Transition(id, bulbId, stepSize, calculateColorPeriod(this, startColor, endColor, stepSize, duration), callback)
  , endColor(endColor)
  , currentColor(startColor)
  , lastHue(400)         // use impossible values to force a packet send
  , lastSaturation(200)
{
  int16_t dr = endColor.r - startColor.r
        , dg = endColor.g - startColor.g
        , db = endColor.b - startColor.b;
  // Calculate step sizes in terms of the period
  stepSizes.r = calculateStepSizePart(dr, duration, period);
  stepSizes.g = calculateStepSizePart(dg, duration, period);
  stepSizes.b = calculateStepSizePart(db, duration, period);
}

size_t ColorTransition::calculateColorPeriod(ColorTransition* t, const ParsedColor& start, const ParsedColor& end, size_t stepSize, size_t duration) {
  int16_t dr = end.r - start.r
        , dg = end.g - start.g
        , db = end.b - start.b;

  int16_t max = std::max(std::max(dr, dg), db);
  int16_t min = std::min(std::min(dr, dg), db);
  int16_t maxAbs = std::abs(min) > std::abs(max) ? min : max;

  return Transition::calculatePeriod(maxAbs, stepSize, duration);
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

  Transition::stepValue(currentColor.r, endColor.r, stepSizes.r);
  Transition::stepValue(currentColor.g, endColor.g, stepSizes.g);
  Transition::stepValue(currentColor.b, endColor.b, stepSizes.b);
}

bool ColorTransition::isFinished() {
  return currentColor == endColor;
}

void ColorTransition::childSerialize(JsonObject& json) {
  json["type"] = "color";

  JsonArray currentColorArr = json.createNestedArray("current_color");
  currentColorArr.add(currentColor.r);
  currentColorArr.add(currentColor.g);
  currentColorArr.add(currentColor.b);

  JsonArray endColorArr = json.createNestedArray("end_color");
  endColorArr.add(endColor.r);
  endColorArr.add(endColor.g);
  endColorArr.add(endColor.b);

  JsonArray stepSizesArr = json.createNestedArray("step_sizes");
  stepSizesArr.add(stepSizes.r);
  stepSizesArr.add(stepSizes.g);
  stepSizesArr.add(stepSizes.b);
}