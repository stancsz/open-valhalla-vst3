#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FDNRAudioProcessor::FDNRAudioProcessor()
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

FDNRAudioProcessor::~FDNRAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout FDNRAudioProcessor::createParameterLayout()
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

const juce::String FDNRAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FDNRAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FDNRAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FDNRAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FDNRAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FDNRAudioProcessor::getNumPrograms()
{
    return 1;
}

int FDNRAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FDNRAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FDNRAudioProcessor::getProgramName (int index)
{
    return {};
}

void FDNRAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FDNRAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    reverbProcessor.prepare(spec);
}

void FDNRAudioProcessor::releaseResources()
{
    reverbProcessor.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FDNRAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FDNRAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
bool FDNRAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FDNRAudioProcessor::createEditor()
{
    return new FDNRAudioProcessorEditor (*this);
}

//==============================================================================
void FDNRAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FDNRAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

void FDNRAudioProcessor::savePreset(const juce::File& file)
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

void FDNRAudioProcessor::loadPreset(const juce::File& file)
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

                 // Use getParameterAsValue to ensure the ValueTree is updated, 
                 // which triggers the listeners and updates the UI reliably.
                 auto paramValue = apvts.getParameterAsValue(paramID);
                 if (paramValue.refersToSameSourceAs(juce::Value()))
                 {
                     // Fallback if parameter not found in APVTS (shouldn't happen if IDs match)
                     continue;
                 }
                 paramValue.setValue(value);
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FDNRAudioProcessor();
}

void FDNRAudioProcessor::resetAllParametersToDefault()
{
    auto resetParam = [&](const juce::String& id, float defaultVal) {
        // Use getParameterAsValue to ensure bidirectional update
        apvts.getParameterAsValue(id).setValue(defaultVal);
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

    if (auto* p = apvts.getParameter("LIMITER")) 
        apvts.getParameterAsValue("LIMITER").setValue(true); // On by default
}

void FDNRAudioProcessor::setParametersForMode(int modeIndex)
{
    auto setParam = [&](const juce::String& id, float val) {
        auto param = apvts.getParameterAsValue(id);
        if ((float)param.getValue() != val)
            param.setValue(val);
    };

    // Helper to reset common modifiers to a "clean" state before applying specific character
    auto resetModifiers = [&]() {
        setParam("WARP", 0.0f);
        setParam("SATURATION", 0.0f);
        setParam("DUCKING", 0.0f);
        setParam("GATE_THRESH", -100.0f);
        setParam("DYNFREQ", 1000.0f);
        setParam("DYNGAIN", 0.0f);
    };

    resetModifiers();

    switch (modeIndex)
    {
        case TwinStar: // Gemini - Balanced, dual nature, standard hall
            setParam("MIX", 40.0f);
            setParam("DELAY", 350.0f);
            setParam("FEEDBACK", 55.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 60.0f);
            setParam("DIFFUSION", 80.0f);
            setParam("MODRATE", 0.6f);
            setParam("MODDEPTH", 25.0f);
            setParam("EQ3_LOW", 0.0f);
            setParam("EQ3_MID", 0.0f);
            setParam("EQ3_HIGH", 0.0f);
            break;

        case SeaSerpent: // Hydra - Deep, submerged, modulated tail
            setParam("MIX", 55.0f);
            setParam("DELAY", 850.0f);
            setParam("FEEDBACK", 88.0f);
            setParam("WIDTH", 90.0f);
            setParam("DENSITY", 85.0f);
            setParam("DIFFUSION", 50.0f);
            setParam("MODRATE", 0.25f);
            setParam("MODDEPTH", 75.0f);
            setParam("EQ3_LOW", 4.0f);
            setParam("EQ3_HIGH", -6.0f);
            setParam("WARP", 20.0f);
            break;

        case HorseMan: // Centaurus - Strong, stable, room-like, woody
            setParam("MIX", 35.0f);
            setParam("DELAY", 180.0f);
            setParam("FEEDBACK", 40.0f);
            setParam("WIDTH", 75.0f);
            setParam("DENSITY", 95.0f);
            setParam("DIFFUSION", 100.0f);
            setParam("MODRATE", 1.2f);
            setParam("MODDEPTH", 10.0f);
            setParam("EQ3_LOW", -1.0f);
            setParam("EQ3_MID", 2.0f);
            setParam("EQ3_HIGH", -2.0f);
            break;

        case Archer: // Sagittarius - Sharp, distant, bright attacks
            setParam("MIX", 45.0f);
            setParam("DELAY", 550.0f);
            setParam("FEEDBACK", 65.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 30.0f); // Lower density for distinct reflections
            setParam("DIFFUSION", 40.0f);
            setParam("MODRATE", 0.8f);
            setParam("MODDEPTH", 35.0f);
            setParam("EQ3_HIGH", 4.0f);
            setParam("SATURATION", 10.0f);
            break;

        case VoidMaker: // Great Annihilator - Massive, infinite, dark drone
            setParam("MIX", 100.0f); // Drone territory
            setParam("DELAY", 1000.0f);
            setParam("FEEDBACK", 98.0f); // Near freeze
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 100.0f);
            setParam("DIFFUSION", 100.0f);
            setParam("MODRATE", 0.15f);
            setParam("MODDEPTH", 60.0f);
            setParam("EQ3_LOW", 8.0f);
            setParam("EQ3_HIGH", -12.0f);
            setParam("SATURATION", 45.0f);
            break;

        case GalaxySpiral: // Andromeda - Swirling, vast, spacey
            setParam("MIX", 50.0f);
            setParam("DELAY", 600.0f);
            setParam("FEEDBACK", 80.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 50.0f);
            setParam("DIFFUSION", 70.0f);
            setParam("MODRATE", 2.8f); // Fast swirl
            setParam("MODDEPTH", 65.0f);
            setParam("WARP", 30.0f);
            break;

        case HarpString: // Lyra - Resonant, metallic, comb-filtery
            setParam("MIX", 40.0f);
            setParam("DELAY", 60.0f); // Very short for resonance
            setParam("FEEDBACK", 90.0f);
            setParam("WIDTH", 60.0f);
            setParam("DENSITY", 0.0f); // No smoothing
            setParam("DIFFUSION", 0.0f); // Pure delays
            setParam("MODRATE", 0.4f);
            setParam("MODDEPTH", 15.0f);
            setParam("EQ3_HIGH", 6.0f);
            break;

        case GoatHorn: // Capricorn - Earthy, dry, distorted plate
            setParam("MIX", 30.0f);
            setParam("DELAY", 220.0f);
            setParam("FEEDBACK", 45.0f);
            setParam("WIDTH", 80.0f);
            setParam("DENSITY", 80.0f);
            setParam("DIFFUSION", 90.0f);
            setParam("MODRATE", 0.9f);
            setParam("MODDEPTH", 20.0f);
            setParam("SATURATION", 35.0f);
            setParam("EQ3_LOW", 2.0f);
            setParam("EQ3_MID", 3.0f);
            setParam("EQ3_HIGH", -4.0f);
            break;

        case NebulaCloud: // Large Magellanic Cloud - Diffuse, soft, ambient
            setParam("MIX", 65.0f);
            setParam("DELAY", 900.0f);
            setParam("FEEDBACK", 82.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 100.0f);
            setParam("DIFFUSION", 100.0f); // Max diffusion
            setParam("MODRATE", 0.3f);
            setParam("MODDEPTH", 40.0f);
            setParam("EQ3_HIGH", -3.0f);
            break;

        case Triangle: // Triangulum - Simple, geometric, sparse echoes
            setParam("MIX", 40.0f);
            setParam("DELAY", 450.0f);
            setParam("FEEDBACK", 50.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 10.0f);
            setParam("DIFFUSION", 20.0f);
            setParam("MODRATE", 0.0f);
            setParam("MODDEPTH", 0.0f);
            break;

        case CloudMajor: // Cirrus Major - Bright, airy, uplifting
            setParam("MIX", 50.0f);
            setParam("DELAY", 700.0f);
            setParam("FEEDBACK", 75.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 90.0f);
            setParam("DIFFUSION", 95.0f);
            setParam("MODRATE", 0.7f);
            setParam("MODDEPTH", 30.0f);
            setParam("EQ3_LOW", -5.0f);
            setParam("EQ3_HIGH", 6.0f);
            break;

        case CloudMinor: // Cirrus Minor - Dark, moody, mysterious
            setParam("MIX", 55.0f);
            setParam("DELAY", 750.0f);
            setParam("FEEDBACK", 78.0f);
            setParam("WIDTH", 90.0f);
            setParam("DENSITY", 90.0f);
            setParam("DIFFUSION", 95.0f);
            setParam("MODRATE", 0.5f);
            setParam("MODDEPTH", 45.0f);
            setParam("EQ3_LOW", 3.0f);
            setParam("EQ3_HIGH", -8.0f);
            break;

        case QueenChair: // Cassiopeia - Regal, wide, rich, complex
            setParam("MIX", 60.0f);
            setParam("DELAY", 650.0f);
            setParam("FEEDBACK", 72.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 85.0f);
            setParam("DIFFUSION", 85.0f);
            setParam("MODRATE", 1.5f);
            setParam("MODDEPTH", 55.0f); // Rich chorus
            setParam("EQ3_MID", 2.0f);
            break;

        case HunterBelt: // Orion - Focused, punchy, tight
            setParam("MIX", 35.0f);
            setParam("DELAY", 150.0f);
            setParam("FEEDBACK", 25.0f);
            setParam("WIDTH", 60.0f);
            setParam("DENSITY", 100.0f);
            setParam("DIFFUSION", 100.0f);
            setParam("MODRATE", 0.0f);
            setParam("MODDEPTH", 0.0f);
            setParam("GATE_THRESH", -30.0f); // Gated effect
            break;

        case WaterBearer: // Aquarius - Liquid, fluid, flowing
            setParam("MIX", 70.0f);
            setParam("DELAY", 500.0f);
            setParam("FEEDBACK", 65.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 70.0f);
            setParam("DIFFUSION", 60.0f);
            setParam("MODRATE", 3.0f); // Fast liquid modulation
            setParam("MODDEPTH", 85.0f);
            setParam("WARP", 15.0f);
            break;

        case TwoFish: // Pisces - Deep, dual delay lines feel
            setParam("MIX", 50.0f);
            setParam("DELAY", 600.0f);
            setParam("FEEDBACK", 60.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 40.0f);
            setParam("DIFFUSION", 50.0f);
            setParam("MODRATE", 0.4f);
            setParam("MODDEPTH", 60.0f);
            setParam("EQ3_LOW", 5.0f);
            setParam("EQ3_HIGH", -10.0f); // Underwater
            break;

        case ScorpionTail: // Scorpio - Aggressive, stinging, intense
            setParam("MIX", 45.0f);
            setParam("DELAY", 300.0f);
            setParam("FEEDBACK", 55.0f);
            setParam("WIDTH", 80.0f);
            setParam("DENSITY", 80.0f);
            setParam("DIFFUSION", 80.0f);
            setParam("MODRATE", 4.0f); // Intense flutter
            setParam("MODDEPTH", 30.0f);
            setParam("SATURATION", 80.0f); // Heavy saturation
            setParam("EQ3_HIGH", 5.0f);
            break;

        case BalanceScale: // Libra - Perfectly neutral, reference room
            setParam("MIX", 50.0f);
            setParam("DELAY", 400.0f);
            setParam("FEEDBACK", 50.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 50.0f);
            setParam("DIFFUSION", 50.0f);
            setParam("MODRATE", 0.5f);
            setParam("MODDEPTH", 20.0f);
            setParam("EQ3_LOW", 0.0f);
            setParam("EQ3_MID", 0.0f);
            setParam("EQ3_HIGH", 0.0f);
            break;

        case LionHeart: // Leo - Warm, bold, mid-forward
            setParam("MIX", 55.0f);
            setParam("DELAY", 500.0f);
            setParam("FEEDBACK", 65.0f);
            setParam("WIDTH", 90.0f);
            setParam("DENSITY", 75.0f);
            setParam("DIFFUSION", 85.0f);
            setParam("MODRATE", 0.8f);
            setParam("MODDEPTH", 25.0f);
            setParam("SATURATION", 25.0f);
            setParam("EQ3_MID", 4.0f); // Mid boost
            setParam("EQ3_HIGH", -2.0f);
            break;

        case Maiden: // Virgo - Clean, pure, pristine
            setParam("MIX", 40.0f);
            setParam("DELAY", 350.0f);
            setParam("FEEDBACK", 45.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 80.0f);
            setParam("DIFFUSION", 90.0f);
            setParam("MODRATE", 0.3f);
            setParam("MODDEPTH", 10.0f); // Very subtle
            setParam("SATURATION", 0.0f); // No saturation
            setParam("EQ3_LOW", -2.0f); // Clean up mud
            break;

        case SevenSisters: // Pleiades - Shimmering, multi-tap texture
            setParam("MIX", 60.0f);
            setParam("DELAY", 777.0f);
            setParam("FEEDBACK", 77.0f);
            setParam("WIDTH", 100.0f);
            setParam("DENSITY", 30.0f); // Grainy
            setParam("DIFFUSION", 60.0f);
            setParam("MODRATE", 2.0f);
            setParam("MODDEPTH", 50.0f);
            setParam("EQ3_HIGH", 8.0f); // Shimmer brightness
            break;

        default:
            setParam("MIX", 50.0f);
            break;
    }
}

void FDNRAudioProcessor::toggleAB()
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
