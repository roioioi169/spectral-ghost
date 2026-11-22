#pragma once
#include <cmath>
#include <juce_core/juce_core.h>

namespace Safety
{
    // Replace NaN/Inf with 0 and flush denormal-range values to 0.
    inline float sanitize(float x) noexcept
    {
        if (! std::isfinite(x)) return 0.0f;
        return std::fabs(x) < 1.0e-30f ? 0.0f : x;
    }

    inline float clampAudio(float x) noexcept
    {
        return juce::jlimit(-1.0f, 1.0f, x);
    }
}
