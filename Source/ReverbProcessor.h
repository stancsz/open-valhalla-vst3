#pragma once
#include <juce_dsp/juce_dsp.h>

struct ReverbParameters
{
    float mix = 50.0f;
    float width = 100.0f;
    float delay = 100.0f;
    float warp = 0.0f;
    float feedback = 50.0f;
    float density = 0.0f;
    float modRate = 0.5f;
    float modDepth = 50.0f;
    int mode = 0;

    // Dynamic EQ
    float dynFreq = 1000.0f;
    float dynQ = 1.0f;
    float dynGain = 0.0f;
    float dynDepth = 0.0f;
    float dynThresh = -20.0f;

    // New Features
    float ducking = 0.0f;
    int preDelaySync = 0;
    float saturation = 0.0f;
    float diffusion = 100.0f;
    float gateThresh = -100.0f;
    float eq3Low = 0.0f;
    float eq3Mid = 0.0f;
    float eq3High = 0.0f;
    float msBalance = 50.0f;
    bool limiterOn = true;
    double bpm = 120.0;
};

class ReverbProcessor
{
public:
    ReverbProcessor();
    ~ReverbProcessor();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::ProcessContextReplacing<float>& context);
    void reset();

    void setParameters(const ReverbParameters& params);

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 192000 };
    juce::dsp::Chorus<float> chorus;

    // Dynamic EQ
    juce::dsp::StateVariableTPTFilter<float> dynEqFilter; // Bandpass for mixing
    juce::dsp::StateVariableTPTFilter<float> detectorFilter; // Bandpass for detector

    // 3-Band EQ
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>> eq3Chain;

    // Dynamics
    juce::dsp::Limiter<float> limiter;
    // Simple Gate implementation variables
    float gateEnv = 0.0f;

    // Saturation
    juce::dsp::WaveShaper<float> saturator;

    double sampleRate = 44100.0;

    ReverbParameters currentParams;

    // Envelopes
    float duckEnv = 0.0f;
    float dynEqEnv = 0.0f;

    // Pre-allocated buffer for processing
    juce::AudioBuffer<float> wetBuffer;
};
