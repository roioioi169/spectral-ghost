#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

namespace ParamIDs
{
    inline constexpr const char* sensitivity = "sensitivity";
    inline constexpr const char* bias        = "bias";
    inline constexpr const char* swap        = "swap";
    inline constexpr const char* bite        = "bite";
    inline constexpr const char* drift       = "drift";
    inline constexpr const char* crossMod    = "crossMod";
    inline constexpr const char* mix         = "mix";
}

inline juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { ParamIDs::sensitivity, 1 }, "Split Sensitivity",
        NormalisableRange<float>(0.2f, 4.0f, 0.001f, 0.5f), 1.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { ParamIDs::bias, 1 }, "Tonal/Noise Bias",
        NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterBool>(
        ParameterID { ParamIDs::swap, 1 }, "Swap Streams", false));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { ParamIDs::bite, 1 }, "Bite",
        NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { ParamIDs::drift, 1 }, "Drift / Chaos",
        NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { ParamIDs::crossMod, 1 }, "Cross-Mod",
        NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { ParamIDs::mix, 1 }, "Mix",
        NormalisableRange<float>(0.0f, 1.0f), 1.0f));

    return { params.begin(), params.end() };
}
