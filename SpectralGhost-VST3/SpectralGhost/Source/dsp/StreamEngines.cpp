#include "StreamEngines.h"
#include "SafetyUtils.h"
#include <cmath>

void StreamEngines::prepare(double sampleRate)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    delayLine.assign(maxDelay, 0.0f);
    reset();
}

void StreamEngines::reset()
{
    std::fill(delayLine.begin(), delayLine.end(), 0.0f);
    env = 0.0f;
    lfoPhase = 0.0;
    writeIdx = 0;
}

float StreamEngines::process(float t, float h, const Params& p)
{
    // Envelope follower on the transient stream: instant attack, slow release.
    const float a = std::fabs(t);
    env = Safety::sanitize((a > env) ? a : env * 0.9995f);

    // Stream Swap: each stream feeds the other engine.
    const float tIn = p.swap ? h : t;
    const float hIn = p.swap ? t : h;

    // Transient engine: Bite (tanh shaping, normalised so bite=0 is unity).
    const float drive = 1.0f + 9.0f * p.bite;
    const float tOut  = std::tanh(tIn * drive) / std::tanh(drive);

    // Harmonic engine: modulated delay standing in for pitch drift (placeholder).
    lfoPhase += juce::MathConstants<double>::twoPi * 0.4 / sr;
    if (lfoPhase > juce::MathConstants<double>::twoPi)
        lfoPhase -= juce::MathConstants<double>::twoPi;

    delayLine[(size_t) writeIdx] = Safety::sanitize(hIn);
    const float depth = 100.0f * p.drift;
    float rp = (float) writeIdx - depth * (1.0f + (float) std::sin(lfoPhase));
    if (rp < 0.0f) rp += (float) maxDelay;

    const int   i0 = (int) rp;
    const float f  = rp - (float) i0;
    const float hWet = delayLine[(size_t) (i0 % maxDelay)] * (1.0f - f)
                     + delayLine[(size_t) ((i0 + 1) % maxDelay)] * f;
    writeIdx = (writeIdx + 1) % maxDelay;

    // Cross-Mod: transient energy ducks the harmonic layer.
    const float duck = 1.0f - p.crossMod * juce::jlimit(0.0f, 1.0f, env * 4.0f);
    const float hOut = hWet * duck;

    const float wet = tOut + hOut;
    const float dry = t + h;
    return Safety::clampAudio(Safety::sanitize(p.mix * wet + (1.0f - p.mix) * dry));
}
