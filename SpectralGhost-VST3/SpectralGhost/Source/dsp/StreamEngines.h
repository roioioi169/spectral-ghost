#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>

// Per-channel processing after the split: Stream Swap, Bite, Drift (placeholder),
// Cross-Mod ducking, and Mix. No allocation after prepare().
class StreamEngines
{
public:
    static constexpr int maxDelay = 4096;

    struct Params
    {
        bool  swap      = false;
        float bite      = 0.0f;  // 0..1
        float drift     = 0.0f;  // 0..1
        float crossMod  = 0.0f;  // 0..1
        float mix       = 1.0f;  // 0..1
    };

    void prepare(double sampleRate);
    void reset();

    // Takes one transient and one harmonic sample, returns one output sample.
    float process(float transient, float harmonic, const Params& p);

private:
    double sr = 44100.0;
    float env = 0.0f;
    double lfoPhase = 0.0;
    std::vector<float> delayLine;
    int writeIdx = 0;
};
