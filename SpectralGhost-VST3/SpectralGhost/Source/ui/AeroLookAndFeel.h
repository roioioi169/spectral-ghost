#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Vista-inspired glass styling: frosted slate panels, glossy metal knobs,
// cyan value arcs with an amber pointer, glossy toggle buttons.
class AeroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AeroLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawGlassPanel(juce::Graphics&, juce::Rectangle<float> bounds,
                        juce::Colour accent) const;

    static constexpr juce::uint32 cyanArgb  = 0xff3ad0ffu;
    static constexpr juce::uint32 amberArgb = 0xffffb347u;
    static constexpr juce::uint32 textArgb  = 0xffe6f1ffu;
};
