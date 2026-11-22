#include "PluginEditor.h"

namespace
{
    constexpr std::array<const char*, 6> knobIds {
        ParamIDs::sensitivity, ParamIDs::bias, ParamIDs::bite,
        ParamIDs::drift, ParamIDs::crossMod, ParamIDs::mix };

    constexpr std::array<const char*, 6> knobNames {
        "SENSITIVITY", "TONAL / NOISE", "BITE", "DRIFT / CHAOS", "CROSS-MOD", "MIX" };
}

SpectralGhostEditor::SpectralGhostEditor(SpectralGhostProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&aero);
    setSize(720, 400);

    for (int i = 0; i < numKnobs; ++i)
    {
        auto& knob = knobs[(size_t) i];
        knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
        addAndMakeVisible(knob);

        auto& label = knobLabels[(size_t) i];
        label.setText(knobNames[(size_t) i], juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(label);

        sliderAttachments[(size_t) i] = std::make_unique<SliderAttach>(
            processor.apvts, knobIds[(size_t) i], knob);
    }

    addAndMakeVisible(swapButton);
    swapAttachment = std::make_unique<ButtonAttach>(processor.apvts, ParamIDs::swap, swapButton);
}

SpectralGhostEditor::~SpectralGhostEditor()
{
    setLookAndFeel(nullptr);
}

void SpectralGhostEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bg(juce::Colour(0xff0e1622), 0.0f, 0.0f,
                            juce::Colour(0xff1b2a3d), 0.0f, (float) getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();

    // Soft cyan glow in the upper-left, a nod to Aero.
    g.setColour(juce::Colour(AeroLookAndFeel::cyanArgb).withAlpha(0.08f));
    g.fillEllipse(-120.0f, -160.0f, 520.0f, 360.0f);

    aero.drawGlassPanel(g, getLocalBounds().toFloat().reduced(10.0f),
                        juce::Colour(AeroLookAndFeel::cyanArgb));

    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.setColour(juce::Colour(AeroLookAndFeel::textArgb));
    g.drawText("SPECTRAL GHOST", getLocalBounds().removeFromTop(44),
               juce::Justification::centred);
}

void SpectralGhostEditor::resized()
{
    auto area = getLocalBounds().reduced(24).withTrimmedTop(40);

    auto swapArea = area.removeFromBottom(40);
    swapButton.setBounds(swapArea.withSizeKeepingCentre(140, 32));

    auto knobRow = area;
    const int colWidth = knobRow.getWidth() / numKnobs;

    for (int i = 0; i < numKnobs; ++i)
    {
        auto col = knobRow.removeFromLeft(colWidth).reduced(4);
        auto labelArea = col.removeFromTop(18);
        knobLabels[(size_t) i].setBounds(labelArea);
        knobs[(size_t) i].setBounds(col);
    }
}
