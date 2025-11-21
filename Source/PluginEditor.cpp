#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VST3OpenValhallaAudioProcessorEditor::VST3OpenValhallaAudioProcessorEditor (VST3OpenValhallaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);

    addSlider(mixSlider, mixAtt, "MIX", "MIX");
    addSlider(widthSlider, widthAtt, "WIDTH", "WIDTH");

    addSlider(duckingSlider, duckingAtt, "DUCKING", "DUCKING");

    addSlider(delaySlider, delayAtt, "DELAY", "DELAY");
    addSlider(warpSlider, warpAtt, "WARP", "WARP");

    addSlider(feedbackSlider, feedbackAtt, "FEEDBACK", "FEEDBACK");
    addSlider(densitySlider, densityAtt, "DENSITY", "DENSITY");

    addSlider(modRateSlider, modRateAtt, "MODRATE", "MOD RATE");
    addSlider(modDepthSlider, modDepthAtt, "MODDEPTH", "MOD DEPTH");

    // Dynamic EQ
    addSlider(dynFreqSlider, dynFreqAtt, "DYNFREQ", "DYN FREQ");
    addSlider(dynQSlider, dynQAtt, "DYNQ", "DYN Q");
    addSlider(dynGainSlider, dynGainAtt, "DYNGAIN", "DYN GAIN");
    addSlider(dynDepthSlider, dynDepthAtt, "DYNDEPTH", "DYN DEPTH");
    addSlider(dynThreshSlider, dynThreshAtt, "DYNTHRESH", "DYN THRESH");

    // Mode
    addAndMakeVisible(modeComboBox);
    modeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.getAPVTS(), "MODE", modeComboBox);
    modeComboBox.addItemList(audioProcessor.getAPVTS().getParameter("MODE")->getAllValueStrings(), 1);
    modeComboBox.setJustificationType(juce::Justification::centred);

    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));
    addAndMakeVisible(modeLabel);
    modeComboBox.setTooltip("Selects the reverb algorithm.");

    // Clear Button
    addAndMakeVisible(clearButton);
    clearButton.setTooltip("Resets reverb buffer and parameters.");
    clearButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF444444));
    clearButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    clearButton.onClick = [this]()
    {
        audioProcessor.clearTriggered = true;
        audioProcessor.resetAllParametersToDefault();
    };

    modeComboBox.onChange = [this]()
    {
        audioProcessor.setParametersForMode(modeComboBox.getSelectedId() - 1);
    };

    // Preset Buttons
    addAndMakeVisible(savePresetButton);
    savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF444444));
    savePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    savePresetButton.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>("Save Preset",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.json");

        auto folderChooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file != juce::File{})
            {
                // Ensure .json extension
                if (!file.hasFileExtension("json"))
                    file = file.withFileExtension("json");

                audioProcessor.savePreset(file);
            }
        });
    };

    addAndMakeVisible(loadPresetButton);
    loadPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF444444));
    loadPresetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadPresetButton.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>("Load Preset",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.json");

        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file != juce::File{})
            {
                audioProcessor.loadPreset(file);
            }
        });
    };

    // Website Link
    addAndMakeVisible(websiteLink);
    websiteLink.setButtonText("ov.stanchen.ca");
    websiteLink.setURL(juce::URL("https://ov.stanchen.ca"));
    websiteLink.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xFF888888));

    setSize (850, 450);
}

VST3OpenValhallaAudioProcessorEditor::~VST3OpenValhallaAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void VST3OpenValhallaAudioProcessorEditor::addSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setPopupDisplayEnabled(true, false, this); // Show value on drag

    if (paramID == "MIX") slider.setTooltip("Controls the wet/dry balance.");
    else if (paramID == "WIDTH") slider.setTooltip("Adjusts stereo width.");
    else if (paramID == "DELAY") slider.setTooltip("Sets pre-delay time (ms).");
    else if (paramID == "WARP") slider.setTooltip("Adds modulation character.");
    else if (paramID == "FEEDBACK") slider.setTooltip("Controls decay time.");
    else if (paramID == "DENSITY") slider.setTooltip("Adjusts diffusion density.");
    else if (paramID == "MODRATE") slider.setTooltip("LFO speed.");
    else if (paramID == "MODDEPTH") slider.setTooltip("LFO intensity.");
    else if (paramID == "DYNFREQ") slider.setTooltip("Dynamic EQ Frequency.");
    else if (paramID == "DYNQ") slider.setTooltip("Dynamic EQ Bandwidth (Q).");
    else if (paramID == "DYNGAIN") slider.setTooltip("Dynamic EQ Static Gain.");
    else if (paramID == "DYNDEPTH") slider.setTooltip("Dynamic EQ Modulation Depth.");
    else if (paramID == "DYNTHRESH") slider.setTooltip("Dynamic EQ Threshold.");
    else if (paramID == "DUCKING") slider.setTooltip("Compresses reverb when input is loud.");

    addAndMakeVisible(slider);

    auto label = std::make_unique<juce::Label>();
    label->setText(name, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    label->setFont(juce::Font(12.0f, juce::Font::bold));
    label->attachToComponent(&slider, false); // Attached above
    addAndMakeVisible(*label);
    labels.push_back(std::move(label));

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), paramID, slider);
}

//==============================================================================
void VST3OpenValhallaAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Gradient Background
    juce::ColourGradient bgGradient(juce::Colour(0xFF0F0F1A), 0, 0, juce::Colour(0xFF1A1A2E), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Header
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font("Futura", 28.0f, juce::Font::bold));
    g.drawFittedText ("OPEN VALHALLA", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);

    // Draw Group Panels
    for (const auto& rect : columnRects)
    {
        g.setColour(juce::Colour(0xFF252535).withAlpha(0.6f));
        g.fillRoundedRectangle(rect.toFloat(), 10.0f);
        g.setColour(juce::Colour(0xFF353545));
        g.drawRoundedRectangle(rect.toFloat(), 10.0f, 1.0f);
    }
}

void VST3OpenValhallaAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto topBar = area.removeFromTop(60); // Title area
    auto bottomBar = area.removeFromBottom(50); // Mode selector

    // Website Link (Centered under title)
    websiteLink.setBounds(0, 45, getWidth(), 20);
    websiteLink.setJustificationType(juce::Justification::centred);

    // Mode Selector centered
    auto modeArea = bottomBar.reduced(0, 5);
    modeLabel.setBounds(modeArea.removeFromLeft(60));
    modeComboBox.setBounds(modeArea.removeFromLeft(200));
    // Center the combobox logic:
    modeComboBox.setBounds(bottomBar.getCentreX() - 100, bottomBar.getY() + 10, 200, 30);
    modeLabel.setBounds(modeComboBox.getX() - 50, modeComboBox.getY(), 45, 30);

    // Clear Button
    clearButton.setBounds(modeComboBox.getRight() + 10, modeComboBox.getY(), 60, 30);

    // Preset Buttons
    savePresetButton.setBounds(clearButton.getRight() + 10, modeComboBox.getY(), 60, 30);
    loadPresetButton.setBounds(savePresetButton.getRight() + 10, modeComboBox.getY(), 60, 30);

    // 5 Columns, 3 Rows
    columnRects.clear();

    int margin = 10;
    int cols = 5;
    int colWidth = (area.getWidth() - (margin * (cols - 1))) / cols;

    auto getCol = [&](int index) -> juce::Rectangle<int> {
        auto r = area.withWidth(colWidth).translated(index * (colWidth + margin), 0);
        columnRects.push_back(r);
        return r;
    };

    // Layout helper for 3 rows
    auto layoutColumn3 = [&](int colIndex, juce::Slider& row1, juce::Slider& row2, juce::Slider* row3) {
        auto col = getCol(colIndex);
        int knobHeight = (col.getHeight() - 20) / 3;

        row1.setBounds(col.removeFromTop(knobHeight).reduced(5)); // Smaller padding
        row2.setBounds(col.removeFromTop(knobHeight).reduced(5));
        if (row3)
            row3->setBounds(col.reduced(5));
    };

    // Col 1: Mix, Width, Ducking
    layoutColumn3(0, mixSlider, widthSlider, &duckingSlider);

    // Col 2: Delay, Warp, Feedback
    layoutColumn3(1, delaySlider, warpSlider, &feedbackSlider);

    // Col 3: Density, ModRate, ModDepth
    layoutColumn3(2, densitySlider, modRateSlider, &modDepthSlider);

    // Col 4: DynFreq, DynQ, DynGain
    layoutColumn3(3, dynFreqSlider, dynQSlider, &dynGainSlider);

    // Col 5: DynDepth, DynThresh
    layoutColumn3(4, dynDepthSlider, dynThreshSlider, nullptr);
}
