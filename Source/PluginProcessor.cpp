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
    stateA = apvts.copyState();
    stateB = apvts.copyState();
}

VST3OpenValhallaAudioProcessor::~VST3OpenValhallaAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VST3OpenValhallaAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.0f, 100.0f, 50.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("WIDTH", "Width", 0.0f, 100.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY", "Delay", 0.0f, 1000.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("WARP", "Warp", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("FEEDBACK", "Feedback", 0.0f, 100.0f, 50.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DENSITY", "Density", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MODRATE", "Mod Rate", 0.0f, 5.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MODDEPTH", "Mod Depth", 0.0f, 100.0f, 50.0f));

    // Dynamic EQ
    layout.add(std::make_unique<juce::AudioParameterFloat>("DYNFREQ", "Dyn Freq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DYNQ", "Dyn Q", 0.1f, 10.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DYNGAIN", "Dyn Gain", -18.0f, 18.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DYNDEPTH", "Dyn Depth", -18.0f, 18.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DYNTHRESH", "Dyn Thresh", -60.0f, 0.0f, -20.0f));

    // New Features
    layout.add(std::make_unique<juce::AudioParameterFloat>("DUCKING", "Ducking", 0.0f, 100.0f, 0.0f));

    juce::StringArray syncOptions;
    syncOptions.add("Free"); syncOptions.add("1/4"); syncOptions.add("1/8"); syncOptions.add("1/16");
    layout.add(std::make_unique<juce::AudioParameterChoice>("PREDELAY_SYNC", "Sync", syncOptions, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("SATURATION", "Saturation", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DIFFUSION", "Diffusion", 0.0f, 100.0f, 100.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("GATE_THRESH", "Gate Thresh", -100.0f, 0.0f, -100.0f));

    // 3-Band EQ
    layout.add(std::make_unique<juce::AudioParameterFloat>("EQ3_LOW", "Low Gain", -12.0f, 12.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("EQ3_MID", "Mid Gain", -12.0f, 12.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("EQ3_HIGH", "High Gain", -12.0f, 12.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("MS_BALANCE", "M/S Bal", 0.0f, 100.0f, 50.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("LIMITER", "Limiter", true));

    // A/B Switch
    layout.add(std::make_unique<juce::AudioParameterBool>("AB_SWITCH", "A/B", false));

    // Mode
    juce::StringArray modes;
    modes.add("Twin Star"); modes.add("Sea Serpent"); modes.add("Horse Man"); modes.add("Archer");
    modes.add("Void Maker"); modes.add("Galaxy Spiral"); modes.add("Harp String"); modes.add("Goat Horn");
    modes.add("Nebula Cloud"); modes.add("Triangle"); modes.add("Cloud Major"); modes.add("Cloud Minor");
    modes.add("Queen Chair"); modes.add("Hunter Belt"); modes.add("Water Bearer"); modes.add("Two Fish");
    modes.add("Scorpion Tail"); modes.add("Balance Scale"); modes.add("Lion Heart"); modes.add("Maiden");
    modes.add("Seven Sisters");

    layout.add(std::make_unique<juce::AudioParameterChoice>("MODE", "Mode", modes, 0));

    return layout;
}

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
    return 1;
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
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    ReverbParameters params;
    params.mix = apvts.getRawParameterValue("MIX")->load();
    params.width = apvts.getRawParameterValue("WIDTH")->load();
    params.delay = apvts.getRawParameterValue("DELAY")->load();
    params.warp = apvts.getRawParameterValue("WARP")->load();
    params.feedback = apvts.getRawParameterValue("FEEDBACK")->load();
    params.density = apvts.getRawParameterValue("DENSITY")->load();
    params.modRate = apvts.getRawParameterValue("MODRATE")->load();
    params.modDepth = apvts.getRawParameterValue("MODDEPTH")->load();

    params.dynFreq = apvts.getRawParameterValue("DYNFREQ")->load();
    params.dynQ = apvts.getRawParameterValue("DYNQ")->load();
    params.dynGain = apvts.getRawParameterValue("DYNGAIN")->load();
    params.dynDepth = apvts.getRawParameterValue("DYNDEPTH")->load();
    params.dynThresh = apvts.getRawParameterValue("DYNTHRESH")->load();

    params.ducking = apvts.getRawParameterValue("DUCKING")->load();
    params.preDelaySync = (int)apvts.getRawParameterValue("PREDELAY_SYNC")->load();
    params.saturation = apvts.getRawParameterValue("SATURATION")->load();
    params.diffusion = apvts.getRawParameterValue("DIFFUSION")->load();
    params.gateThresh = apvts.getRawParameterValue("GATE_THRESH")->load();

    params.eq3Low = apvts.getRawParameterValue("EQ3_LOW")->load();
    params.eq3Mid = apvts.getRawParameterValue("EQ3_MID")->load();
    params.eq3High = apvts.getRawParameterValue("EQ3_HIGH")->load();

    params.msBalance = apvts.getRawParameterValue("MS_BALANCE")->load();
    params.limiterOn = (apvts.getRawParameterValue("LIMITER")->load() > 0.5f);

    params.mode = (int)apvts.getRawParameterValue("MODE")->load();

    if (auto* ph = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (ph->getCurrentPosition(info))
            params.bpm = info.bpm;
    }

    reverbProcessor.setParameters(params);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    if (clearTriggered.exchange(false))
    {
        reverbProcessor.reset();
    }

    reverbProcessor.process(context);
}

//==============================================================================
bool VST3OpenValhallaAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* VST3OpenValhallaAudioProcessor::createEditor()
{
    return new VST3OpenValhallaAudioProcessorEditor (*this);
}

//==============================================================================
void VST3OpenValhallaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VST3OpenValhallaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

void VST3OpenValhallaAudioProcessor::savePreset(const juce::File& file)
{
    auto state = apvts.copyState();
    juce::DynamicObject* jsonObject = new juce::DynamicObject();
    juce::DynamicObject* paramsObject = new juce::DynamicObject();

    auto& params = getParameters();
    for (auto* param : params)
    {
        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
        {
            paramsObject->setProperty(p->paramID, apvts.getRawParameterValue(p->paramID)->load());
        }
    }

    jsonObject->setProperty("parameters", paramsObject);
    jsonObject->setProperty("pluginVersion", JucePlugin_VersionString);
    jsonObject->setProperty("pluginName", JucePlugin_Name);

    juce::var jsonVar(jsonObject);
    juce::String jsonString = juce::JSON::toString(jsonVar);

    file.replaceWithText(jsonString);
}

void VST3OpenValhallaAudioProcessor::loadPreset(const juce::File& file)
{
    if (!file.existsAsFile()) return;

    juce::String jsonString = file.loadFileAsString();
    juce::var jsonVar = juce::JSON::parse(jsonString);

    if (juce::JSON::toString(jsonVar) == "null") return;

    if (jsonVar.hasProperty("parameters"))
    {
        auto* paramsObject = jsonVar.getProperty("parameters", juce::var()).getDynamicObject();
        if (paramsObject)
        {
            auto& properties = paramsObject->getProperties();
            for (auto& prop : properties)
            {
                 auto paramID = prop.name.toString();
                 auto value = (float)prop.value;

                 auto* param = apvts.getParameter(paramID);
                 if (param)
                 {
                     if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
                     {
                         float normalized = rangedParam->convertTo0to1(value);
                         rangedParam->setValueNotifyingHost(normalized);
                     }
                 }
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VST3OpenValhallaAudioProcessor();
}

void VST3OpenValhallaAudioProcessor::resetAllParametersToDefault()
{
    auto resetParam = [&](const juce::String& id, float defaultVal) {
        if (auto* p = apvts.getParameter(id))
        {
            if (auto* range = dynamic_cast<juce::RangedAudioParameter*>(p))
                p->setValueNotifyingHost(range->convertTo0to1(defaultVal));
        }
    };

    resetParam("MIX", 50.0f);
    resetParam("WIDTH", 100.0f);
    resetParam("DELAY", 100.0f);
    resetParam("WARP", 0.0f);
    resetParam("FEEDBACK", 50.0f);
    resetParam("DENSITY", 0.0f);
    resetParam("MODRATE", 0.5f);
    resetParam("MODDEPTH", 50.0f);

    resetParam("DYNFREQ", 1000.0f);
    resetParam("DYNQ", 1.0f);
    resetParam("DYNGAIN", 0.0f);
    resetParam("DYNDEPTH", 0.0f);
    resetParam("DYNTHRESH", -20.0f);

    resetParam("DUCKING", 0.0f);
    resetParam("PREDELAY_SYNC", 0.0f); // Free
    resetParam("SATURATION", 0.0f);
    resetParam("DIFFUSION", 100.0f);
    resetParam("GATE_THRESH", -100.0f);

    resetParam("EQ3_LOW", 0.0f);
    resetParam("EQ3_MID", 0.0f);
    resetParam("EQ3_HIGH", 0.0f);

    resetParam("MS_BALANCE", 50.0f);

    if (auto* p = apvts.getParameter("LIMITER")) p->setValueNotifyingHost(1.0f); // On by default
}

void VST3OpenValhallaAudioProcessor::setParametersForMode(int modeIndex)
{
}

void VST3OpenValhallaAudioProcessor::toggleAB()
{
    if (isStateA)
    {
        stateA = apvts.copyState();
        apvts.replaceState(stateB);
        isStateA = false;
    }
    else
    {
        stateB = apvts.copyState();
        apvts.replaceState(stateA);
        isStateA = true;
    }
}
