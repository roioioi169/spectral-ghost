#include "AeroLookAndFeel.h"

AeroLookAndFeel::AeroLookAndFeel()
{
    setColour(juce::Label::textColourId, juce::Colour(textArgb));
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(textArgb));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    setColour(juce::TextButton::textColourOffId, juce::Colour(textArgb));
}

void AeroLookAndFeel::drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> b,
                                     juce::Colour accent) const
{
    juce::ColourGradient body(juce::Colour(0xff2a3646), b.getX(), b.getY(),
                              juce::Colour(0xff121a24), b.getX(), b.getBottom(), false);
    g.setGradientFill(body);
    g.fillRoundedRectangle(b, 10.0f);

    // Upper glass reflection.
    juce::ColourGradient gloss(juce::Colours::white.withAlpha(0.22f), b.getX(), b.getY(),
                               juce::Colours::white.withAlpha(0.0f), b.getX(), b.getCentreY(), false);
    g.setGradientFill(gloss);
    g.fillRoundedRectangle(b.reduced(1.0f).withHeight(b.getHeight() * 0.5f), 9.0f);

    // Accent-tinted inner border.
    g.setColour(accent.withAlpha(0.35f));
    g.drawRoundedRectangle(b.reduced(0.5f), 10.0f, 1.0f);
}

void AeroLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float startAngle, float endAngle,
                                       juce::Slider&)
{
    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(6.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  centre = bounds.getCentre();
    const float angle  = startAngle + sliderPos * (endAngle - startAngle);

    // Dim track.
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0x30ffffffu));
    g.strokePath(track, juce::PathStrokeType(3.0f));

    // Cyan value arc.
    juce::Path value;
    value.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, startAngle, angle, true);
    g.setColour(juce::Colour(cyanArgb));
    g.strokePath(value, juce::PathStrokeType(3.0f));

    // Metal body.
    const auto body = bounds.reduced(6.0f);
    juce::ColourGradient metal(juce::Colour(0xffdfe7f0), body.getX(), body.getY(),
                               juce::Colour(0xff4a5870), body.getRight(), body.getBottom(), true);
    g.setGradientFill(metal);
    g.fillEllipse(body);

    // Glossy highlight across the upper half.
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.fillEllipse(body.reduced(3.0f).withHeight(body.getHeight() * 0.45f));

    // Amber pointer.
    juce::Path pointer;
    pointer.addRectangle(-1.5f, -radius * 0.75f, 3.0f, radius * 0.45f);
    g.setColour(juce::Colour(amberArgb));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void AeroLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b,
                                       bool highlighted, bool)
{
    auto bounds = b.getLocalBounds().toFloat().reduced(2.0f);
    const bool on = b.getToggleState();

    drawGlassPanel(g, bounds, on ? juce::Colour(cyanArgb) : juce::Colour(0xff6a7a90u));

    if (highlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRoundedRectangle(bounds, 10.0f);
    }

    g.setColour(on ? juce::Colour(cyanArgb) : juce::Colour(0xffa0b0c8u));
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText(b.getButtonText(), bounds, juce::Justification::centred);
}
