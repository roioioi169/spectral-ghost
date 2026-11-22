#pragma once
#include <juce_dsp/juce_dsp.h>

// Linear per-sample ramp for a control value (avoids zipper noise).
class Smoothed
{
public:
    void reset(double sampleRate, float initial, double rampSeconds = 0.03)
    {
        v.reset(sampleRate, rampSeconds);
        v.setCurrentAndTargetValue(initial);
    }

    void set(float target) { v.setTargetValue(target); }
    float next()           { return v.getNextValue(); }

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> v;
};
