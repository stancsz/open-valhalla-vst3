#include "PluginProcessor.h"
#include "PluginEditor.h"

VST3OpenValhallaAudioProcessorEditor::VST3OpenValhallaAudioProcessorEditor (VST3OpenValhallaAudioProcessor& p)
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
    // Utility Group
    preDelaySyncBox.addItemList({"Free", "1/4", "1/8", "1/16"}, 1);
    preDelaySyncBox.setTextWhenNothingSelected("Default");
    addComboBox(preDelaySyncBox, preDelaySyncAtt, "PREDELAY_SYNC", "SYNC");

    addAndMakeVisible(abSwitchButton);
    abSwitchButton.setButtonText("A/B");
    abSwitchButton.setToggleState(audioProcessor.isStateA, juce::dontSendNotification);
    abSwitchButton.onClick = [this]() {
        audioProcessor.toggleAB();
        abSwitchButton.setToggleState(audioProcessor.isStateA, juce::dontSendNotification); // This assumes A is checked? Or maybe A is one state and B is off?
        // Let's make it clearer:
        abSwitchButton.setButtonText(audioProcessor.isStateA ? "A" : "B");
    };
    abSwitchButton.setButtonText(audioProcessor.isStateA ? "A" : "B"); // Initialize text

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

    addAndMakeVisible(websiteLink);
    websiteLink.setButtonText("stanchen.ca");
    websiteLink.setURL(juce::URL("https://stanchen.ca"));

    modeComboBox.onChange = [this]() { audioProcessor.setParametersForMode(modeComboBox.getSelectedId() - 1); };

    setSize(1150, 600);
}

VST3OpenValhallaAudioProcessorEditor::~VST3OpenValhallaAudioProcessorEditor() { setLookAndFeel(nullptr); }

void VST3OpenValhallaAudioProcessorEditor::addSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    // Reduced text box height to bring value closer to knob
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 12);
    slider.setPopupDisplayEnabled(true, false, this);
    slider.setTooltip(name);
    addAndMakeVisible(slider);

    auto label = std::make_unique<juce::Label>();
    label->setText(name, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setFont(juce::Font(11.0f, juce::Font::bold));
    // Attach label below slider with minimal offset
    label->attachToComponent(&slider, false);
    addAndMakeVisible(*label);
    labels.push_back(std::move(label));

    if (audioProcessor.getAPVTS().getParameter(paramID))
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), paramID, slider);
}

void VST3OpenValhallaAudioProcessorEditor::addComboBox(juce::ComboBox& box, std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attachment, const juce::String& paramID, const juce::String& name)
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

void VST3OpenValhallaAudioProcessorEditor::addToggle(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& attachment, const juce::String& paramID, const juce::String& name)
{
    button.setButtonText(name);
    addAndMakeVisible(button);
    if (audioProcessor.getAPVTS().getParameter(paramID))
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.getAPVTS(), paramID, button);
}

void VST3OpenValhallaAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bgGradient(juce::Colour(0xFF101010), 0, 0, juce::Colour(0xFF202028), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Futura", 30.0f, juce::Font::bold));
    g.drawFittedText("OPEN VALHALLA", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);

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

void VST3OpenValhallaAudioProcessorEditor::resized()
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
        // Large Mix Knob (32% height) - reduced to give more space below
        auto top = r.removeFromTop(r.getHeight() * 0.32f);
        mixSlider.setBounds(top.reduced(10));

        // M/S Width, Ducking, Limiter in remaining space with minimal vertical padding
        int h = r.getHeight() / 3;
        msBalanceSlider.setBounds(r.removeFromTop(h).reduced(15, 4));
        duckingSlider.setBounds(r.removeFromTop(h).reduced(15, 4));
        
        // Center the limiter button
        auto limiterRow = r.removeFromTop(h);
        int buttonWidth = 80;
        limiterButton.setBounds(limiterRow.getCentreX() - buttonWidth / 2, 
                                limiterRow.getY() + 4, 
                                buttonWidth, 
                                limiterRow.getHeight() - 8);
    }

    // 2. TIME / SIZE
    {
        auto r = getGroup(1);
        // Large Delay Knob (32% height) - reduced to give more space below
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
        // Large Width Knob (32% height) - reduced to give more space below
        auto top = r.removeFromTop(r.getHeight() * 0.32f);
        widthSlider.setBounds(top.reduced(10));

        // Grid for others (Warp, Rate, Depth, Sat, Gate)
        // 5 controls remaining.
        // Row 1: Warp, Rate
        // Row 2: Depth, Sat
        // Row 3: Gate

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
        // Empty slot next to gate
    }

    // 4. FILTERS / EQ
    {
        auto r = getGroup(3);
        int rowHeight = r.getHeight() / 3;
        
        // Helper to center slider vertically but keep it tight to pull label down
        auto placeTight = [&](juce::Slider& s, juce::Rectangle<int> zone) {
            int maxS = juce::jmin(zone.getWidth(), zone.getHeight());
            // Reduce knob size to avoid overlap
            int knobSize = (int)(maxS * 0.75f); 
            int sliderH = knobSize + 20; // Knob + Text + Padding
            
            // Center vertically
            s.setBounds(zone.getCentreX() - knobSize / 2, 
                        zone.getCentreY() - sliderH / 2 + 10, 
                        knobSize, 
                        sliderH);
        };

        // Row 1: Dyn Freq, Dyn Q
        auto row1 = r.removeFromTop(rowHeight);
        int halfW = row1.getWidth() / 2;
        
        placeTight(dynFreqSlider, row1.removeFromLeft(halfW).reduced(2));
        placeTight(dynQSlider, row1.reduced(2));

        // Row 2: Dyn Gain, Dyn Thr
        auto row2 = r.removeFromTop(rowHeight);
        
        placeTight(dynGainSlider, row2.removeFromLeft(halfW).reduced(2));
        placeTight(dynThreshSlider, row2.reduced(2));

        // Row 3: 3-Band EQ
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

        // A/B and Save
        auto row3 = r.removeFromTop(h);
        int w = row3.getWidth() / 2;
        abSwitchButton.setBounds(row3.removeFromLeft(w).reduced(5));
        savePresetButton.setBounds(row3.reduced(5));

        // Load and Clear
        auto row4 = r.removeFromTop(h);
        loadPresetButton.setBounds(row4.removeFromLeft(w).reduced(5));
        clearButton.setBounds(row4.reduced(5));
    }

    // Bottom Bar
    websiteLink.setBounds(bottomBar.reduced(10));
}
