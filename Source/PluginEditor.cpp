#include "PluginProcessor.h"
#include "PluginEditor.h"

FDNRAudioProcessorEditor::FDNRAudioProcessorEditor (FDNRAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);

    // Mix Group
    addSlider(mixSlider, mixAtt, "MIX", "MIX");
    addSlider(msBalanceSlider, msBalanceAtt, "MS_BALANCE", "M/S WIDTH");
    addSlider(duckingSlider, duckingAtt, "DUCKING", "DUCKING");
    addToggle(limiterButton, limiterAtt, "LIMITER", "LIMITER");

    // Time Group
    addSlider(delaySlider, delayAtt, "DELAY", "DELAY");
    addSlider(feedbackSlider, feedbackAtt, "FEEDBACK", "FEEDBACK");
    addSlider(densitySlider, densityAtt, "DENSITY", "DENSITY");
    addSlider(diffusionSlider, diffusionAtt, "DIFFUSION", "DIFFUSION");

    // Mod Group
    addSlider(widthSlider, widthAtt, "WIDTH", "WIDTH");
    addSlider(warpSlider, warpAtt, "WARP", "WARP");
    addSlider(modRateSlider, modRateAtt, "MODRATE", "RATE");
    addSlider(modDepthSlider, modDepthAtt, "MODDEPTH", "DEPTH");
    addSlider(saturationSlider, saturationAtt, "SATURATION", "SAT");
    addSlider(gateThreshSlider, gateThreshAtt, "GATE_THRESH", "GATE");

    // Filter Group
    addSlider(dynFreqSlider, dynFreqAtt, "DYNFREQ", "DYN FREQ");
    addSlider(dynQSlider, dynQAtt, "DYNQ", "DYN Q");
    addSlider(dynGainSlider, dynGainAtt, "DYNGAIN", "DYN GAIN");
    addSlider(dynThreshSlider, dynThreshAtt, "DYNTHRESH", "DYN THR");
    addSlider(eqLowSlider, eqLowAtt, "EQ3_LOW", "LOW");
    addSlider(eqMidSlider, eqMidAtt, "EQ3_MID", "MID");
    addSlider(eqHighSlider, eqHighAtt, "EQ3_HIGH", "HIGH");

    // Utility Group
    preDelaySyncBox.addItemList({"Free", "1/4", "1/8", "1/16"}, 1);
    preDelaySyncBox.setTextWhenNothingSelected("Default");
    addComboBox(preDelaySyncBox, preDelaySyncAtt, "PREDELAY_SYNC", "SYNC");

    addAndMakeVisible(abSwitchButton);
    abSwitchButton.setButtonText("A/B");
    abSwitchButton.setToggleState(audioProcessor.isStateA, juce::dontSendNotification);
    abSwitchButton.onClick = [this]() {
        audioProcessor.toggleAB();
        abSwitchButton.setToggleState(audioProcessor.isStateA, juce::dontSendNotification);
        abSwitchButton.setButtonText(audioProcessor.isStateA ? "A" : "B");
    };
    abSwitchButton.setButtonText(audioProcessor.isStateA ? "A" : "B");

    addAndMakeVisible(modeComboBox);
    modeComboBox.addItemList(audioProcessor.getAPVTS().getParameter("MODE")->getAllValueStrings(), 1);
    modeComboBox.setTextWhenNothingSelected("Default");
    modeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.getAPVTS(), "MODE", modeComboBox);
    modeComboBox.setJustificationType(juce::Justification::centred);

    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF80FFEA));
    modeLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(modeLabel);
    modeLabel.attachToComponent(&modeComboBox, false);

    addAndMakeVisible(clearButton);
    clearButton.onClick = [this]() { audioProcessor.clearTriggered = true; audioProcessor.resetAllParametersToDefault(); };

    addAndMakeVisible(savePresetButton);
    savePresetButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>("Save", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.json");
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode, [this](const juce::FileChooser& c) { audioProcessor.savePreset(c.getResult().withFileExtension("json")); });
    };

    addAndMakeVisible(loadPresetButton);
    loadPresetButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>("Load", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.json");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode, [this](const juce::FileChooser& c) { audioProcessor.loadPreset(c.getResult()); });
    };



    modeComboBox.onChange = [this]() { audioProcessor.setParametersForMode(modeComboBox.getSelectedId() - 1); };

    setSize(1150, 600);
}

FDNRAudioProcessorEditor::~FDNRAudioProcessorEditor() { setLookAndFeel(nullptr); }

void FDNRAudioProcessorEditor::addSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 60, 14);
    slider.setPopupDisplayEnabled(true, false, this);
    slider.setTooltip(name);
    addAndMakeVisible(slider);

    auto label = std::make_unique<juce::Label>();
    label->setText(name, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setFont(juce::Font(11.0f, juce::Font::bold));
    label->attachToComponent(&slider, false);
    addAndMakeVisible(*label);
    labels.push_back(std::move(label));

    if (audioProcessor.getAPVTS().getParameter(paramID))
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), paramID, slider);
}

void FDNRAudioProcessorEditor::addComboBox(juce::ComboBox& box, std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attachment, const juce::String& paramID, const juce::String& name)
{
    box.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(box);
    if (audioProcessor.getAPVTS().getParameter(paramID))
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.getAPVTS(), paramID, box);

    auto label = std::make_unique<juce::Label>();
    label->setText(name, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setFont(juce::Font(11.0f, juce::Font::bold));
    label->attachToComponent(&box, false);
    addAndMakeVisible(*label);
    labels.push_back(std::move(label));
}

void FDNRAudioProcessorEditor::addToggle(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& attachment, const juce::String& paramID, const juce::String& name)
{
    button.setButtonText(name);
    addAndMakeVisible(button);
    if (audioProcessor.getAPVTS().getParameter(paramID))
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.getAPVTS(), paramID, button);
}

void FDNRAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bgGradient(juce::Colour(0xFF101010), 0, 0, juce::Colour(0xFF202028), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    auto titleArea = getLocalBounds().removeFromTop(60);
    titleArea.removeFromTop(15); // Top margin

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Futura", 24.0f, juce::Font::bold));
    g.drawText("FND Reverb", titleArea.removeFromTop(25), juce::Justification::centred, false);
    
    g.setFont(juce::Font("Futura", 13.0f, juce::Font::plain));
    g.drawText("Nap-Tech", titleArea, juce::Justification::centred, false);

    auto area = getLocalBounds().reduced(15);
    area.removeFromTop(50);
    area.removeFromBottom(50);

    int cols = 5;
    int colWidth = area.getWidth() / cols;

    juce::String titles[] = { "MIX / LEVEL", "TIME / SIZE", "MODULATION", "FILTERS / EQ", "UTILITY" };

    for (int i=0; i<cols; ++i)
    {
        auto r = area.withWidth(colWidth - 10).translated(i * colWidth + 5, 0);

        g.setColour(juce::Colour(0xFF1E1E1E));
        g.fillRoundedRectangle(r.toFloat(), 8.0f);
        g.setColour(juce::Colour(0xFF303030));
        g.drawRoundedRectangle(r.toFloat(), 8.0f, 1.5f);

        auto header = r.removeFromTop(30);
        g.setColour(juce::Colour(0xFF80FFEA));
        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.drawText(titles[i], header, juce::Justification::centred, false);
    }
}

void FDNRAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);
    area.removeFromTop(50);
    auto bottomBar = area.removeFromBottom(50);

    int cols = 5;
    int colWidth = area.getWidth() / cols;

    auto getGroup = [&](int i) {
        return area.withWidth(colWidth - 10).translated(i * colWidth + 5, 0).reduced(5, 35); // Margin for title
    };

    // 1. MIX / LEVEL
    {
        auto r = getGroup(0);
        auto top = r.removeFromTop(r.getHeight() * 0.32f);
        mixSlider.setBounds(top.reduced(10));

        int h = r.getHeight() / 3;
        msBalanceSlider.setBounds(r.removeFromTop(h).reduced(15, 4));
        duckingSlider.setBounds(r.removeFromTop(h).reduced(15, 4));
        
        auto limiterRow = r.removeFromTop(h);
        int buttonWidth = 80;
        int buttonHeight = (limiterRow.getHeight() - 8) / 2;
        limiterButton.setBounds(limiterRow.getCentreX() - buttonWidth / 2, 
                                limiterRow.getCentreY() - buttonHeight / 2, 
                                buttonWidth, 
                                buttonHeight);
    }

    // 2. TIME / SIZE
    {
        auto r = getGroup(1);
        auto top = r.removeFromTop(r.getHeight() * 0.32f);
        delaySlider.setBounds(top.reduced(10));

        int h = r.getHeight() / 3;
        feedbackSlider.setBounds(r.removeFromTop(h).reduced(15, 4));
        densitySlider.setBounds(r.removeFromTop(h).reduced(15, 4));
        diffusionSlider.setBounds(r.removeFromTop(h).reduced(15, 4));
    }

    // 3. MODULATION
    {
        auto r = getGroup(2);
        auto top = r.removeFromTop(r.getHeight() * 0.32f);
        widthSlider.setBounds(top.reduced(10));

        int rowH = r.getHeight() / 3;
        int colW = r.getWidth() / 2;

        auto row1 = r.removeFromTop(rowH);
        warpSlider.setBounds(row1.removeFromLeft(colW).reduced(5, 4));
        modRateSlider.setBounds(row1.reduced(5, 4));

        auto row2 = r.removeFromTop(rowH);
        modDepthSlider.setBounds(row2.removeFromLeft(colW).reduced(5, 4));
        saturationSlider.setBounds(row2.reduced(5, 4));

        auto row3 = r.removeFromTop(rowH);
        gateThreshSlider.setBounds(row3.removeFromLeft(colW).reduced(5, 4));
    }

    // 4. FILTERS / EQ
    {
        auto r = getGroup(3);
        int rowHeight = r.getHeight() / 3;
        
        auto placeTight = [&](juce::Slider& s, juce::Rectangle<int> zone) {
            int maxS = juce::jmin(zone.getWidth(), zone.getHeight());
            int knobSize = (int)(maxS * 0.75f); 
            int sliderH = knobSize + 20; 
            
            s.setBounds(zone.getCentreX() - knobSize / 2, 
                        zone.getCentreY() - sliderH / 2 + 10, 
                        knobSize, 
                        sliderH);
        };

        auto row1 = r.removeFromTop(rowHeight);
        int halfW = row1.getWidth() / 2;
        
        placeTight(dynFreqSlider, row1.removeFromLeft(halfW).reduced(2));
        placeTight(dynQSlider, row1.reduced(2));

        auto row2 = r.removeFromTop(rowHeight);
        
        placeTight(dynGainSlider, row2.removeFromLeft(halfW).reduced(2));
        placeTight(dynThreshSlider, row2.reduced(2));

        auto row3 = r;
        int thirdW = row3.getWidth() / 3;
        
        placeTight(eqLowSlider, row3.removeFromLeft(thirdW).reduced(2));
        placeTight(eqMidSlider, row3.removeFromLeft(thirdW).reduced(2));
        placeTight(eqHighSlider, row3.reduced(2));
    }

    // 5. UTILITY
    {
        auto r = getGroup(4);
        int h = r.getHeight() / 4;

        modeComboBox.setBounds(r.removeFromTop(h).reduced(5, 15));
        preDelaySyncBox.setBounds(r.removeFromTop(h).reduced(5, 15));

        auto row3 = r.removeFromTop(h);
        int w = row3.getWidth() / 2;
        
        auto b1 = row3.removeFromLeft(w).reduced(5);
        abSwitchButton.setBounds(b1.getX(), b1.getCentreY() - b1.getHeight() / 4, b1.getWidth(), b1.getHeight() / 2);
        
        auto b2 = row3.reduced(5);
        savePresetButton.setBounds(b2.getX(), b2.getCentreY() - b2.getHeight() / 4, b2.getWidth(), b2.getHeight() / 2);

        auto row4 = r.removeFromTop(h);
        
        auto b3 = row4.removeFromLeft(w).reduced(5);
        loadPresetButton.setBounds(b3.getX(), b3.getCentreY() - b3.getHeight() / 4, b3.getWidth(), b3.getHeight() / 2);
        
        auto b4 = row4.reduced(5);
        clearButton.setBounds(b4.getX(), b4.getCentreY() - b4.getHeight() / 4, b4.getWidth(), b4.getHeight() / 2);
    }

    // Bottom Bar

}
