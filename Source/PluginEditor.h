#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ValhallaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ValhallaLookAndFeel()
    {
        // Modern Color Palette
        setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE0E0E0)); // Off-white
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF66F2D5)); // Cyan/Mint accent
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF2A2A2A)); // Dark grey track
        setColour(juce::Label::textColourId, juce::Colour(0xFFCCCCCC)); // Light grey text
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFFFFFFF));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // 1. Background Track (Arc)
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.strokePath(backgroundArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 2. Value Arc (Filled)
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);

            g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 3. Knob Body
        auto knobRadius = radius * 0.6f;
        g.setColour(juce::Colour(0xFF202020)); // Dark knob body
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        g.setColour(juce::Colour(0xFF404040)); // Slight outline
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

        // 4. Pointer Indicator
        juce::Path p;
        auto pointerLength = knobRadius * 0.7f;
        auto pointerThickness = 3.0f;
        p.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(juce::Colours::white);
        g.fillPath(p);
    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll (label.findColour (juce::Label::backgroundColourId));

        if (! label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;
            const juce::Font font (getLabelFont (label));

            g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
            g.setFont (font);

            auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

            g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                              juce::jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                              label.getMinimumHorizontalScale());
        }
        else if (label.isEnabled())
        {
            g.setColour (label.findColour (juce::Label::outlineColourId));
            g.drawRect (label.getLocalBounds());
        }
    }

    juce::Font getLabelFont (juce::Label& label) override
    {
        return juce::Font("Verdana", 12.0f, juce::Font::bold); // cleaner font
    }
};

class VST3OpenValhallaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VST3OpenValhallaAudioProcessorEditor (VST3OpenValhallaAudioProcessor&);
    ~VST3OpenValhallaAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    VST3OpenValhallaAudioProcessor& audioProcessor;
    ValhallaLookAndFeel lookAndFeel;

    // Helper to add slider
    void addSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID, const juce::String& name);

    // Sliders
    juce::Slider mixSlider, widthSlider;
    juce::Slider delaySlider, warpSlider;
    juce::Slider feedbackSlider, densitySlider;
    juce::Slider modRateSlider, modDepthSlider;
    juce::Slider eqHighSlider, eqLowSlider;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAtt, widthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAtt, warpAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAtt, densityAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRateAtt, modDepthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqHighAtt, eqLowAtt;

    // Labels
    std::vector<std::unique_ptr<juce::Label>> labels;

    // Mode Selector
    juce::ComboBox modeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAtt;
    juce::Label modeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3OpenValhallaAudioProcessorEditor)
};
