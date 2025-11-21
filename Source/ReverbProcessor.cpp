#include "ReverbProcessor.h"
#include "PluginProcessor.h" // For Mode enum if needed, but we passed int

ReverbProcessor::ReverbProcessor()
{
    // Initialize defaults
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.33f;
    reverbParams.dryLevel = 0.4f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;

    reverb.setParameters(reverbParams);

    // Initialize Chorus for modulation
    chorus.setRate(0.5f);
    chorus.setDepth(0.5f);
    chorus.setCentreDelay(10.0f);
    chorus.setFeedback(0.0f);
    chorus.setMix(1.0f); // We use chorus purely as a modulator block

    lowPassFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    highPassFilter.setType(juce::dsp::FirstOrderTPTFilterType::highpass);
}

ReverbProcessor::~ReverbProcessor()
{
}

void ReverbProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    reverb.prepare(spec);
    delayLine.prepare(spec);
    chorus.prepare(spec);
    lowPassFilter.prepare(spec);
    highPassFilter.prepare(spec);

    delayLine.setMaximumDelayInSamples(1.5 * sampleRate);

    // Pre-allocate wet buffer
    wetBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}

void ReverbProcessor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    // Apply parameters and mode logic first

    // Base values from knobs
    float roomSize = currentFeedback / 100.0f; // 0 to 1
    float damping = 1.0f - (currentDensity / 100.0f);

    float baseRoomSize = roomSize;
    float baseDamping = damping;
    float baseWidth = currentWidth / 100.0f;

    float appliedDelayMs = currentDelay;
    float appliedChorusRate = currentModRate;
    float appliedChorusDepth = currentModDepth / 100.0f;
    float appliedChorusFeedback = (currentWarp / 100.0f) * 0.8f - 0.4f;
    float appliedChorusMix = 0.5f;

    reverbParams.wetLevel = 1.0f;
    reverbParams.dryLevel = 0.0f;

    // Mode Implementation Logic
    switch (currentMode) {
        case 0: // Twin Star (Gemini): Fast attack, short decay
             reverbParams.roomSize = baseRoomSize * 0.7f;
             reverbParams.damping = baseDamping * 0.5f;
             break;

        case 1: // Sea Serpent (Hydra): Fast-ish attack
             reverbParams.roomSize = baseRoomSize * 0.8f;
             appliedChorusDepth = 0.8f; // More modulation
             break;

        case 2: // Horse Man (Centaurus): Medium attack
             reverbParams.roomSize = baseRoomSize;
             break;

        case 3: // Archer (Sagittarius): Slow attack
             reverbParams.roomSize = baseRoomSize;
             reverbParams.damping = baseDamping * 1.2f;
             break;

        case 4: // Void Maker (Great Annihilator): Very long decay
             reverbParams.roomSize = 0.95f + (baseRoomSize * 0.04f);
             reverbParams.damping = 0.1f;
             break;

        case 5: // Galaxy Spiral (Andromeda)
             reverbParams.roomSize = 0.98f;
             appliedChorusRate = 0.2f; // Slow modulation
             break;

        case 6: // Harp String (Lyra)
             reverbParams.roomSize = baseRoomSize * 0.6f;
             reverbParams.damping = baseDamping * 0.6f;
             break;

        case 7: // Goat Horn (Capricorn)
             reverbParams.roomSize = baseRoomSize * 0.75f;
             baseWidth = baseWidth * 0.8f;
             break;

        case 8: // Nebula Cloud (LMC)
             reverbParams.roomSize = 0.9f;
             appliedDelayMs = currentDelay * 1.5f;
             break;

        case 9: // Triangle (Triangulum)
             reverbParams.roomSize = 0.92f;
             appliedDelayMs = currentDelay * 2.0f;
             break;

        case 10: // Cloud Major
             reverbParams.roomSize = baseRoomSize * 0.8f;
             reverbParams.damping = 0.2f;
             appliedChorusFeedback = 0.7f;
             break;

        case 11: // Cloud Minor
             reverbParams.roomSize = baseRoomSize * 0.6f;
             reverbParams.damping = 0.2f;
             appliedChorusFeedback = 0.6f;
             break;

        case 12: // Queen Chair (Cassiopeia)
             reverbParams.roomSize = 0.95f;
             reverbParams.damping = 0.8f;
             break;

        case 13: // Hunter Belt (Orion)
             reverbParams.roomSize = 0.99f;
             appliedChorusDepth = 0.9f;
             break;

        case 14: // Water Bearer (Aquarius): EchoVerb
             reverbParams.roomSize = baseRoomSize * 0.4f;
             reverbParams.wetLevel = 0.5f;
             break;

        case 15: // Two Fish (Pisces)
             reverbParams.roomSize = baseRoomSize * 0.6f;
             reverbParams.wetLevel = 0.6f;
             break;

        case 16: // Scorpion Tail (Scorpio)
             reverbParams.roomSize = baseRoomSize * 0.7f;
             highPassFilter.setCutoffFrequency(currentEqLow * 2.0f); // Tweak filter directly
             break;

        case 17: // Balance Scale (Libra)
             reverbParams.roomSize = 0.9f;
             appliedChorusRate = currentModRate * 1.5f;
             appliedChorusDepth = 0.7f;
             break;

        case 18: // Lion Heart (Leo)
             reverbParams.roomSize = 0.99f;
             reverbParams.damping = 0.05f;
             break;

        case 19: // Maiden (Virgo)
             reverbParams.roomSize = baseRoomSize * 0.5f;
             appliedDelayMs = currentDelay * 0.8f;
             break;

        case 20: // Seven Sisters (Pleiades)
             reverbParams.roomSize = baseRoomSize * 0.8f;
             baseWidth = 1.0f;
             reverbParams.damping = 0.5f;
             appliedChorusMix = 0.2f;
             break;

        default:
             reverbParams.roomSize = baseRoomSize;
             reverbParams.damping = baseDamping;
             break;
    }

    reverbParams.width = baseWidth;

    // Apply to Modules
    delayLine.setDelay(appliedDelayMs * sampleRate / 1000.0f);

    chorus.setRate(appliedChorusRate);
    chorus.setDepth(appliedChorusDepth);
    chorus.setFeedback(appliedChorusFeedback);
    chorus.setMix(appliedChorusMix);

    reverb.setParameters(reverbParams);

    lowPassFilter.setCutoffFrequency(currentEqHigh);
    if (currentMode != 16) // Already set for Scorpio
        highPassFilter.setCutoffFrequency(currentEqLow);

    // PROCESS AUDIO

    // Get Audio Block
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    // Copy input to wet buffer
    for(size_t i=0; i<inputBlock.getNumChannels(); ++i)
    {
         wetBuffer.copyFrom(i, 0, inputBlock.getChannelPointer(i), (int)inputBlock.getNumSamples());
    }

    // Create a sub-block for the current process size
    juce::dsp::AudioBlock<float> wetBlock(wetBuffer.getArrayOfWritePointers(), inputBlock.getNumChannels(), inputBlock.getNumSamples());
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);

    // 1. Delay
    delayLine.process(wetContext);

    // 2. Modulation (Chorus)
    chorus.process(wetContext);

    // 3. Reverb
    reverb.process(wetContext);

    // 4. EQ
    lowPassFilter.process(wetContext);
    highPassFilter.process(wetContext);

    // 5. Mix
    // dry is inputBlock
    // wet is wetBlock

    float wetAmount = currentMix / 100.0f;
    float dryAmount = 1.0f - wetAmount;

    // Sum
    outputBlock.multiplyBy(dryAmount);
    // Add wet
    // There is no simple add for blocks, iterate channels
    for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch)
    {
        outputBlock.getChannelPointer(ch);
        juce::FloatVectorOperations::addWithMultiply(
            outputBlock.getChannelPointer(ch),
            wetBlock.getChannelPointer(ch),
            wetAmount,
            outputBlock.getNumSamples());
    }
}

void ReverbProcessor::reset()
{
    reverb.reset();
    delayLine.reset();
    chorus.reset();
    lowPassFilter.reset();
    highPassFilter.reset();
}

void ReverbProcessor::setParameters(float mix, float width, float delay, float warp,
                                    float feedback, float density, float modRate,
                                    float modDepth, float eqHigh, float eqLow, int mode)
{
    currentMix = mix;
    currentWidth = width;
    currentDelay = delay;
    currentWarp = warp;
    currentFeedback = feedback;
    currentDensity = density;
    currentModRate = modRate;
    currentModDepth = modDepth;
    currentEqHigh = eqHigh;
    currentEqLow = eqLow;
    currentMode = mode;
}

void ReverbProcessor::updateInternalParameters()
{
    // Logic moved to process or setParameters to avoid atomic issues,
    // but ideally should be here.
}
