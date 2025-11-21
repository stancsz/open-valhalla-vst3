#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VST3OpenValhallaAudioProcessorEditor::VST3OpenValhallaAudioProcessorEditor (VST3OpenValhallaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);

    addSlider(mixSlider, mixAtt, "MIX", "MIX");
    addSlider(widthSlider, widthAtt, "WIDTH", "WIDTH");

    addSlider(delaySlider, delayAtt, "DELAY", "DELAY");
    addSlider(warpSlider, warpAtt, "WARP", "WARP");

    addSlider(feedbackSlider, feedbackAtt, "FEEDBACK", "FEEDBACK");
    addSlider(densitySlider, densityAtt, "DENSITY", "DENSITY");

    addSlider(modRateSlider, modRateAtt, "MODRATE", "MOD RATE");
    addSlider(modDepthSlider, modDepthAtt, "MODDEPTH", "MOD DEPTH");

    addSlider(eqHighSlider, eqHighAtt, "EQHIGH", "EQ HIGH");
    addSlider(eqLowSlider, eqLowAtt, "EQLOW", "EQ LOW");

    addSlider(duckingSlider, duckingAtt, "DUCKING", "DUCKING");

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
    clearButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A2A));
    clearButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    clearButton.onClick = [this]()
    {
        audioProcessor.clearTriggered = true;
        audioProcessor.resetAllParametersToDefault();
    };

    modeComboBox.onChange = [this]()
    {
        // When user changes mode via UI, update defaults
        // Note: onChange is called when the combo box item is selected.
        // The attachment might also trigger parameter change.
        // We want to update AFTER the parameter is set, or just forcefully set parameters.
        // Since setParametersForMode updates parameters via APVTS, it will update the UI sliders too.

        // However, ComboBoxAttachment syncs the parameter with the ComboBox.
        // If we change other parameters here, it should be fine.

        // Issue: onChange might be called before the parameter is updated by the attachment?
        // Actually, attachment listens to the parameter and updates the box, AND listens to the box and updates the parameter.
        // If we rely on the box index:

        audioProcessor.setParametersForMode(modeComboBox.getSelectedId() - 1);
    };

    // Preset Buttons
    addAndMakeVisible(savePresetButton);
    savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A2A));
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
    loadPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A2A));
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
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); // Clean look, no text box
    slider.setPopupDisplayEnabled(true, false, this); // Show value on drag

    if (paramID == "MIX") slider.setTooltip("Controls the wet/dry balance.");
    else if (paramID == "WIDTH") slider.setTooltip("Adjusts stereo width.");
    else if (paramID == "DELAY") slider.setTooltip("Sets pre-delay time (ms).");
    else if (paramID == "WARP") slider.setTooltip("Adds modulation character.");
    else if (paramID == "FEEDBACK") slider.setTooltip("Controls decay time.");
    else if (paramID == "DENSITY") slider.setTooltip("Adjusts diffusion density.");
    else if (paramID == "MODRATE") slider.setTooltip("LFO speed.");
    else if (paramID == "MODDEPTH") slider.setTooltip("LFO intensity.");
    else if (paramID == "EQHIGH") slider.setTooltip("High-cut filter.");
    else if (paramID == "EQLOW") slider.setTooltip("Low-cut filter.");
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
    auto drawPanel = [&](juce::Rectangle<int> bounds, juce::String title)
    {
        g.setColour(juce::Colour(0xFF252535).withAlpha(0.6f));
        g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
        g.setColour(juce::Colour(0xFF353545));
        g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 1.0f);

        // Subtle Label at bottom of panel? Or maybe not needed if knobs are labeled.
    };

    // We need the layout coordinates here, which is tricky without recalculating.
    // A better way is to use a component for groups, but for now we approximate or store rects.
    // Or we can just draw 'separation' lines.
}

void VST3OpenValhallaAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto topBar = area.removeFromTop(40); // Title area
    auto bottomBar = area.removeFromBottom(50); // Mode selector

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

    // Website Link
    websiteLink.setBounds(bottomBar.getRight() - 110, modeComboBox.getY(), 100, 30);

    // Main panels layout
    // 5 Columns: Mix | Delay | Feedback | Mod | EQ

    int margin = 10;
    int cols = 6;
    int colWidth = (area.getWidth() - (margin * (cols - 1))) / cols;

    auto getCol = [&](int index) -> juce::Rectangle<int> {
        return area.withWidth(colWidth).translated(index * (colWidth + margin), 0);
    };

    // Helper to layout 2 knobs in a column
    auto layoutColumn = [&](int colIndex, juce::Slider& top, juce::Slider& bottom) {
        auto col = getCol(colIndex);
        int knobHeight = (col.getHeight() - 20) / 2;
        top.setBounds(col.removeFromTop(knobHeight).reduced(10));
        bottom.setBounds(col.reduced(10));
    };

    layoutColumn(0, mixSlider, widthSlider);
    layoutColumn(1, delaySlider, warpSlider);
    layoutColumn(2, feedbackSlider, densitySlider);
    layoutColumn(3, modRateSlider, modDepthSlider);
    layoutColumn(4, eqHighSlider, eqLowSlider);

    // Column 5: Ducking (Top)
    auto col5 = getCol(5);
    int knobHeight = (col5.getHeight() - 20) / 2;
    duckingSlider.setBounds(col5.removeFromTop(knobHeight).reduced(10));
}
