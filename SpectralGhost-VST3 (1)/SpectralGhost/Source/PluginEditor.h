#pragma once
#include <array>
#include <memory>
#include "PluginProcessor.h"
#include "ui/AeroLookAndFeel.h"

class SpectralGhostEditor : public juce::AudioProcessorEditor
{
public:
    explicit SpectralGhostEditor(SpectralGhostProcessor&);
    ~SpectralGhostEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SpectralGhostProcessor& processor;

    // Declared first so it outlives the components that use it.
    AeroLookAndFeel aero;

    static constexpr int numKnobs = 6;
    std::array<juce::Slider, numKnobs> knobs;
    std::array<juce::Label,  numKnobs> knobLabels;
    juce::ToggleButton swapButton { "SWAP" };

    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::array<std::unique_ptr<SliderAttach>, numKnobs> sliderAttachments;
    std::unique_ptr<ButtonAttach> swapAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralGhostEditor)
};
