#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class ValhallaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ValhallaLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF80FFEA)); // Brighter Cyan
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF606060)); // Visible Grey
        setColour(juce::Label::textColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFFFFFFF));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        setColour(juce::ToggleButton::tickColourId, juce::Colours::white);
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        // Reduce knob size by 25%
        auto radius = ((float) juce::jmin (width / 2, height / 2) - 4.0f) * 0.75f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;

        // Track
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.strokePath(backgroundArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);

            g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(valueArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Knob Body
        auto knobRadius = radius * 0.6f;
        g.setColour(juce::Colour(0xFF252525));
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        g.setColour(juce::Colour(0xFF505050));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

        // Pointer
        juce::Path p;
        auto pointerLength = knobRadius * 0.8f;
        auto pointerThickness = 3.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        p.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(findColour(juce::Slider::thumbColourId));
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
    }

    juce::Font getLabelFont (juce::Label& label) override
    {
        return juce::Font("Verdana", 12.0f, juce::Font::bold);
    }

    juce::BorderSize<int> getLabelBorderSize (juce::Label& label) override
    {
        // Minimal border to bring label text very close to value text
        return juce::BorderSize<int>(0, 0, 0, 0);
    }
};

class VST3OpenValhallaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VST3OpenValhallaAudioProcessorEditor (VST3OpenValhallaAudioProcessor&);
    ~VST3OpenValhallaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    VST3OpenValhallaAudioProcessor& audioProcessor;
    ValhallaLookAndFeel lookAndFeel;

    void addSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID, const juce::String& name);
    void addComboBox(juce::ComboBox& box, std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attachment, const juce::String& paramID, const juce::String& name);
    void addToggle(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& attachment, const juce::String& paramID, const juce::String& name);

    // Sliders & Controls
    juce::Slider mixSlider, widthSlider, duckingSlider;
    juce::ComboBox preDelaySyncBox;

    juce::Slider delaySlider, warpSlider, feedbackSlider, saturationSlider;

    juce::Slider densitySlider, modRateSlider, modDepthSlider, diffusionSlider;

    juce::Slider dynFreqSlider, dynQSlider, dynGainSlider, dynThreshSlider;

    juce::Slider eqLowSlider, eqMidSlider, eqHighSlider;
    juce::Slider msBalanceSlider, gateThreshSlider, abMorphSlider;
    juce::ToggleButton limiterButton, abSwitchButton;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAtt, widthAtt, duckingAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> preDelaySyncAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAtt, warpAtt, feedbackAtt, saturationAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAtt, modRateAtt, modDepthAtt, diffusionAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dynFreqAtt, dynQAtt, dynGainAtt, dynThreshAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eqLowAtt, eqMidAtt, eqHighAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> msBalanceAtt, gateThreshAtt, abMorphAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAtt, abSwitchAtt;

    std::vector<std::unique_ptr<juce::Label>> labels;

    // Mode
    juce::ComboBox modeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAtt;
    juce::Label modeLabel;
    juce::Label syncLabel; // Explicit label for sync

    // Buttons
    juce::TextButton clearButton { "CLEAR" };
    juce::TextButton savePresetButton { "SAVE" };
    juce::TextButton loadPresetButton { "LOAD" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::HyperlinkButton websiteLink;
    juce::TooltipWindow tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3OpenValhallaAudioProcessorEditor)
};
