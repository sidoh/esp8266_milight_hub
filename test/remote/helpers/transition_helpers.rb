require 'chroma'

module TransitionHelpers
  module Defaults
    PERIOD = 225
    NUM_PERIODS = 20
    DURATION = PERIOD * NUM_PERIODS
  end

  def highlight_value(a, highlight_ix)
    str = a
      .each_with_index
      .map do |x, i|
        i == highlight_ix ? ">>#{x}<<" : x
      end
      .join(', ')
    "[#{str}]"
  end

  def color_transitions_are_equal(expected:, seen:)
    %i(hue saturation).each do |label|
      e = expected.map { |x| x[label] }
      s = seen.map { |x| x[label] }

      transitions_are_equal(expected: e, seen: s, label: label, allowed_variation: label == :saturation ? 5 : 20)
    end
  end

  def transitions_are_equal(expected:, seen:, allowed_variation: 0, label: nil)
    generate_msg = ->(a, b, i) do
      s = "Transition step value"

      if !label.nil?
        s << " for #{label} "
      end

      s << "at index #{i} "

      s << if allowed_variation == 0
        "should be equal to expected value.  Expected: #{a}, saw: #{b}."
      else
        "should be within #{allowed_variation} of expected value.  Expected: #{a}, saw: #{b}."
      end

      s << "  Steps:\n"
      s << "  Expected : #{highlight_value(expected, i)},\n"
      s << "  Seen     : #{highlight_value(seen, i)}"
    end

    expect(expected.length).to eq(seen.length),
      "Transition was a different length than expected.\n" <<
      "  Expected : #{expected}\n" <<
      "  Seen     : #{seen}"

    expected.zip(seen).each_with_index do |x, i|
      a, b = x
      diff = (a - b).abs
      expect(diff).to be <= allowed_variation, generate_msg.call(a, b, i)
    end
  end

  def rgb_to_hs(*color)
    if color.length > 1
      r, g, b = color
    else
      r, g, b = coerce_color(color.first)
    end

    hsv = Chroma::Converters::HsvConverter.convert_rgb(Chroma::ColorModes::Rgb.new(r, g, b))
    { hue: hsv.h.round, saturation: (100*hsv.s).round }
  end

  def coerce_color(c)
    c.split(',').map(&:to_i) unless c.is_a?(Array)
  end

  def calculate_color_transition_steps(start_color:, end_color:, duration: nil, period: nil, num_periods: Defaults::NUM_PERIODS)
    start_color = coerce_color(start_color)
    end_color = coerce_color(end_color)

    part_transitions = start_color.zip(end_color).map do |c|
      s, e = c
      calculate_transition_steps(start_value: s, end_value: e, duration: duration, period: period, num_periods: num_periods)
    end

    # If some colors don't transition, they'll stay at the same value while others move.
    # Turn this: [[1,2,3], [0], [4,5,6]]
    # Into this: [[1,2,3], [0,0,0], [4,5,6]]
    longest = part_transitions.max_by { |x| x.length }.length
    part_transitions.map! { |x| x + [x.last]*(longest-x.length) }

    # Zip individual parts into 3-tuples
    # Turn this: [[1,2,3], [0,0,0], [4,5,6]]
    # Into this: [[1,0,4], [2,0,5], [3,0,6]]
    transition_colors = part_transitions.first.zip(*part_transitions[1..part_transitions.length])

    # Undergo the RGB -> HSV w/ value = 100
    transition_colors.map do |x|
      r, g, b = x
      rgb_to_hs(r, g, b)
    end
  end

  def calculate_transition_steps(start_value:, end_value:, duration: nil, period: nil, num_periods: Defaults::NUM_PERIODS)
    if !duration.nil? || !period.nil?
      period ||= Defaults::PERIOD
      duration ||= Defaults::DURATION
      num_periods = [1, (duration / period.to_f).ceil].max
    end

    diff = end_value - start_value
    step_size = [1, (diff.abs / num_periods.to_f).ceil].max
    step_size = -step_size if end_value < start_value

    steps = []
    val = start_value

    while val != end_value
      steps << val

      if (end_value - val).abs < step_size.abs
        val += (end_value - val)
      else
        val += step_size
      end
    end

    steps << end_value
    steps
  end
end