#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VST3OpenValhallaAudioProcessor::VST3OpenValhallaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

VST3OpenValhallaAudioProcessor::~VST3OpenValhallaAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VST3OpenValhallaAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Mix
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "MIX", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));

    // Width
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "WIDTH", "Width", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));

    // Delay
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "DELAY", "Delay", juce::NormalisableRange<float>(0.0f, 1000.0f, 0.1f), 100.0f));

    // Warp
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "WARP", "Warp", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    // Feedback
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "FEEDBACK", "Feedback", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));

    // Density
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "DENSITY", "Density", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    // Mod Rate
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "MODRATE", "Mod Rate", juce::NormalisableRange<float>(0.0f, 5.0f, 0.01f), 0.5f));

    // Mod Depth
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "MODDEPTH", "Mod Depth", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));

    // EQ High
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "EQHIGH", "EQ High", juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.3f), 5000.0f));

    // EQ Low
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "EQLOW", "EQ Low", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.3f), 200.0f));

    // Mode
    juce::StringArray modes;
    modes.add("Twin Star");         // Gemini
    modes.add("Sea Serpent");       // Hydra
    modes.add("Horse Man");         // Centaurus
    modes.add("Archer");            // Sagittarius
    modes.add("Void Maker");        // Great Annihilator
    modes.add("Galaxy Spiral");     // Andromeda
    modes.add("Harp String");       // Lyra
    modes.add("Goat Horn");         // Capricorn
    modes.add("Nebula Cloud");      // Large Magellanic Cloud
    modes.add("Triangle");          // Triangulum
    modes.add("Cloud Major");       // Cirrus Major
    modes.add("Cloud Minor");       // Cirrus Minor
    modes.add("Queen Chair");       // Cassiopeia
    modes.add("Hunter Belt");       // Orion
    modes.add("Water Bearer");      // Aquarius
    modes.add("Two Fish");          // Pisces
    modes.add("Scorpion Tail");     // Scorpio
    modes.add("Balance Scale");     // Libra
    modes.add("Lion Heart");        // Leo
    modes.add("Maiden");            // Virgo
    modes.add("Seven Sisters");     // Pleiades

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "MODE", "Mode", modes, 0));

    return layout;
}

//==============================================================================
const juce::String VST3OpenValhallaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VST3OpenValhallaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VST3OpenValhallaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VST3OpenValhallaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VST3OpenValhallaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VST3OpenValhallaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VST3OpenValhallaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VST3OpenValhallaAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VST3OpenValhallaAudioProcessor::getProgramName (int index)
{
    return {};
}

void VST3OpenValhallaAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VST3OpenValhallaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    reverbProcessor.prepare(spec);
}

void VST3OpenValhallaAudioProcessor::releaseResources()
{
    reverbProcessor.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VST3OpenValhallaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VST3OpenValhallaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto mix = apvts.getRawParameterValue("MIX")->load();
    auto width = apvts.getRawParameterValue("WIDTH")->load();
    auto delay = apvts.getRawParameterValue("DELAY")->load();
    auto warp = apvts.getRawParameterValue("WARP")->load();
    auto feedback = apvts.getRawParameterValue("FEEDBACK")->load();
    auto density = apvts.getRawParameterValue("DENSITY")->load();
    auto modRate = apvts.getRawParameterValue("MODRATE")->load();
    auto modDepth = apvts.getRawParameterValue("MODDEPTH")->load();
    auto eqHigh = apvts.getRawParameterValue("EQHIGH")->load();
    auto eqLow = apvts.getRawParameterValue("EQLOW")->load();
    auto mode = static_cast<int>(apvts.getRawParameterValue("MODE")->load());

    reverbProcessor.setParameters(
        mix, width, delay, warp, feedback, density,
        modRate, modDepth, eqHigh, eqLow, mode
    );

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Check for Clear Trigger
    if (clearTriggered.exchange(false))
    {
        reverbProcessor.reset();
    }

    reverbProcessor.process(context);
}

//==============================================================================
bool VST3OpenValhallaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VST3OpenValhallaAudioProcessor::createEditor()
{
    return new VST3OpenValhallaAudioProcessorEditor (*this);
}

//==============================================================================
void VST3OpenValhallaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VST3OpenValhallaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

void VST3OpenValhallaAudioProcessor::resetAllParametersToDefault()
{
    // Resets all parameters to their default values
    for (auto* param : getParameters())
    {
        if (auto* p = dynamic_cast<juce::AudioProcessorParameter*>(param))
        {
            // Skip the MODE parameter if we don't want to reset it on Clear
            // But usually "Clear" clears audio, maybe not settings.
            // However, the user said "knobs ... didn't change" when Clear is clicked.
            // This implies they expect them to change.
            // If I reset MODE, it jumps back to Twin Star. That might be annoying.
            // I will skip MODE.

            // Checking if the parameter is "MODE"
            // The parameterID might be prefixed.
            // apvts parameters usually have the ID we gave them.

            // Actually, getParameters() returns AudioProcessorParameter*.
            // We can check name or rely on order, but ID is safer.
            // But AudioProcessorParameter doesn't always expose ID easily without casting to AudioProcessorParameterWithID.

            if (auto* pid = dynamic_cast<juce::AudioProcessorParameterWithID*>(p))
            {
                if (pid->paramID == "MODE") continue;
            }

            p->setValueNotifyingHost(p->getDefaultValue());
        }
    }
}

void VST3OpenValhallaAudioProcessor::setParametersForMode(int modeIndex)
{
    // Sets parameters to "sensible defaults" for the selected mode.
    // Since we don't have a definitive list, we will map a few known characteristics
    // and default the rest.

    // Helper to set parameter by ID
    auto setParam = [&](juce::String id, float value) {
        if (auto* p = apvts.getParameter(id))
        {
             // setValueNotifyingHost expects normalized 0-1
             p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(value));
        }
    };

    // Default starting point for all modes
    setParam("MIX", 50.0f);
    setParam("WIDTH", 100.0f);
    setParam("DELAY", 300.0f);
    setParam("WARP", 0.0f);
    setParam("FEEDBACK", 50.0f);
    setParam("DENSITY", 50.0f); // Default middle density
    setParam("MODRATE", 0.5f);
    setParam("MODDEPTH", 50.0f);
    setParam("EQHIGH", 8000.0f);
    setParam("EQLOW", 200.0f);

    switch (modeIndex)
    {
        case 0: // Twin Star (Gemini)
            setParam("DELAY", 100.0f);
            setParam("FEEDBACK", 40.0f);
            break;

        case 1: // Sea Serpent (Hydra)
            setParam("DELAY", 200.0f);
            setParam("MODDEPTH", 70.0f); // More modulation
            break;

        case 2: // Horse Man (Centaurus)
            setParam("FEEDBACK", 60.0f);
            break;

        case 3: // Archer (Sagittarius)
             setParam("DELAY", 500.0f);
             setParam("DENSITY", 20.0f);
             break;

        case 4: // Void Maker (Great Annihilator)
             setParam("DELAY", 600.0f);
             setParam("FEEDBACK", 85.0f); // Long decay
             setParam("DENSITY", 80.0f);
             break;

        case 5: // Galaxy Spiral (Andromeda)
             setParam("DELAY", 800.0f);
             setParam("MODRATE", 0.2f);
             break;

        case 10: // Cloud Major
             setParam("WARP", 40.0f);
             break;

        case 14: // Water Bearer (EchoVerb)
             setParam("MIX", 40.0f);
             setParam("DELAY", 400.0f);
             setParam("FEEDBACK", 30.0f);
             break;

        // Add more as needed, or stick to defaults.
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST3OpenValhallaAudioProcessor();
}
