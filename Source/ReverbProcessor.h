#pragma once
#include <juce_dsp/juce_dsp.h>

class ReverbProcessor
{
public:
    ReverbProcessor();
    ~ReverbProcessor();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::ProcessContextReplacing<float>& context);
    void reset();

    void setParameters(float mix, float width, float delay, float warp,
                       float feedback, float density, float modRate,
                       float modDepth, float dynFreq, float dynQ, float dynGain, float dynDepth, float dynThresh, float ducking, int mode);

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 192000 }; // Max 2s roughly
    juce::dsp::Chorus<float> chorus; // For modulation

    juce::dsp::StateVariableTPTFilter<float> dynamicEqFilter;
    juce::dsp::StateVariableTPTFilter<float> detectorFilter;

    double sampleRate = 44100.0;

    // Internal State Parameters
    float currentMix = 50.0f;
    float currentWidth = 100.0f;
    float currentDelay = 300.0f;
    float currentWarp = 0.0f;
    float currentFeedback = 50.0f;
    float currentDensity = 0.0f;
    float currentModRate = 0.5f;
    float currentModDepth = 50.0f;

    // Dynamic EQ Params
    float currentDynFreq = 1000.0f;
    float currentDynQ = 1.0f;
    float currentDynGain = 0.0f;
    float currentDynDepth = 0.0f;
    float currentDynThresh = -20.0f;

    float currentDucking = 0.0f;
    int currentMode = 0;

    // Envelope follower state
    float envelope = 0.0f;
    float dynEqEnvelope = 0.0f;

    // Pre-allocated buffer for processing
    juce::AudioBuffer<float> wetBuffer;

    void updateInternalParameters();
};
