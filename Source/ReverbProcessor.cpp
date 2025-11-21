#include "ReverbProcessor.h"
#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>

ReverbProcessor::ReverbProcessor()
{
    dynEqFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    detectorFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);

    saturator.functionToUse = [](float x) {
        return std::tanh(x);
    };

    limiter.setThreshold(0.0f);
    limiter.setRelease(100.0f);
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

    dynEqFilter.prepare(spec);
    detectorFilter.prepare(spec);

    eq3Chain.prepare(spec);

    limiter.prepare(spec);
    saturator.prepare(spec);

    delayLine.setMaximumDelayInSamples(2.0 * sampleRate);
    wetBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}

void ReverbProcessor::reset()
{
    reverb.reset();
    delayLine.reset();
    chorus.reset();
    dynEqFilter.reset();
    detectorFilter.reset();
    eq3Chain.reset();
    limiter.reset();
    saturator.reset();

    gateEnv = 0.0f;
    duckEnv = 0.0f;
    dynEqEnv = 0.0f;
}

void ReverbProcessor::setParameters(const ReverbParameters& params)
{
    currentParams = params;
}

void ReverbProcessor::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    // 1. Update DSP Parameters

    juce::dsp::Reverb::Parameters rParams;
    rParams.roomSize = currentParams.feedback / 100.0f;
    rParams.damping = 1.0f - (currentParams.density / 100.0f);
    rParams.width = currentParams.width / 100.0f;
    rParams.wetLevel = 1.0f;
    rParams.dryLevel = 0.0f;
    rParams.freezeMode = 0.0f;

    float baseSize = rParams.roomSize;

    switch (currentParams.mode) {
        case 0: rParams.roomSize *= 0.7f; break; // TwinStar
        case 4: rParams.roomSize = 0.95f + (baseSize * 0.04f); rParams.damping = 0.1f; break; // VoidMaker
        default: break;
    }

    reverb.setParameters(rParams);

    // Pre-Delay
    float delayMs = currentParams.delay;
    if (currentParams.preDelaySync > 0 && currentParams.bpm > 0)
    {
        float beatMs = 60000.0f / (float)currentParams.bpm;
        if (currentParams.preDelaySync == 1) delayMs = beatMs; // 1/4
        else if (currentParams.preDelaySync == 2) delayMs = beatMs * 0.5f; // 1/8
        else if (currentParams.preDelaySync == 3) delayMs = beatMs * 0.25f; // 1/16
    }
    delayLine.setDelay(delayMs * sampleRate / 1000.0f);

    // Warp
    chorus.setRate(currentParams.modRate);
    chorus.setDepth(currentParams.modDepth / 100.0f);
    chorus.setFeedback((currentParams.warp / 100.0f) * 0.5f);
    chorus.setMix(0.5f);

    // Dynamic EQ
    dynEqFilter.setCutoffFrequency(currentParams.dynFreq);
    dynEqFilter.setResonance(currentParams.dynQ);
    detectorFilter.setCutoffFrequency(currentParams.dynFreq);
    detectorFilter.setResonance(currentParams.dynQ);

    // 3-Band EQ
    auto& lowShelf = eq3Chain.get<0>();
    lowShelf.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 200.0f, 0.71f, juce::Decibels::decibelsToGain(currentParams.eq3Low));

    auto& midPeak = eq3Chain.get<1>();
    midPeak.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 1.0f, juce::Decibels::decibelsToGain(currentParams.eq3Mid));

    auto& highShelf = eq3Chain.get<2>();
    highShelf.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6000.0f, 0.71f, juce::Decibels::decibelsToGain(currentParams.eq3High));

    // Limiter
    limiter.setThreshold(currentParams.limiterOn ? -0.1f : 10.0f);

    // 2. Process Audio
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    wetBuffer.copyFrom(0, 0, inputBlock.getChannelPointer(0), (int)inputBlock.getNumSamples());
    if (inputBlock.getNumChannels() > 1)
        wetBuffer.copyFrom(1, 0, inputBlock.getChannelPointer(1), (int)inputBlock.getNumSamples());

    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);

    // 2.1 Saturation (Pre)
    float drive = 1.0f + (currentParams.saturation / 20.0f);
    wetBlock.multiplyBy(drive);
    saturator.process(wetContext);
    wetBlock.multiplyBy(1.0f / drive);

    // 2.2 Pre-Delay
    delayLine.process(wetContext);

    // 2.3 Warp
    chorus.process(wetContext);

    // 2.4 Reverb
    reverb.process(wetContext);

    // 2.5 Gate, DynEQ, Ducking Loop
    size_t nSamples = wetBlock.getNumSamples();
    size_t nChannels = wetBlock.getNumChannels();

    float gateThreshLin = juce::Decibels::decibelsToGain(currentParams.gateThresh);
    float dynThreshLin = std::pow(10.0f, currentParams.dynThresh / 20.0f);

    // Coefficients
    float gateRel = 1.0f - std::exp(-1.0f / (0.1f * sampleRate));
    float dynAtt = 1.0f - std::exp(-1.0f / (0.005f * sampleRate));
    float dynRel = 1.0f - std::exp(-1.0f / (0.1f * sampleRate));
    float duckAtt = 1.0f - std::exp(-1.0f / (0.01f * sampleRate));
    float duckRel = 1.0f - std::exp(-1.0f / (0.1f * sampleRate));

    float duckIntensity = currentParams.ducking / 100.0f;

    for (size_t s = 0; s < nSamples; ++s)
    {
        // Gate Level
        float maxLevel = 0.0f;
        for (size_t ch=0; ch<nChannels; ++ch) maxLevel = std::max(maxLevel, std::abs(wetBlock.getChannelPointer(ch)[s]));

        if (maxLevel > gateThreshLin) gateEnv = 1.0f;
        else gateEnv += (0.0f - gateEnv) * gateRel;

        // DynEQ Detector
        float detOut = detectorFilter.processSample(0, maxLevel);
        float envIn = std::abs(detOut);
        if (envIn > dynEqEnv) dynEqEnv += (envIn - dynEqEnv) * dynAtt;
        else dynEqEnv += (envIn - dynEqEnv) * dynRel;

        float dynGain = 0.0f;
        if (dynEqEnv > dynThreshLin)
        {
             float excessDb = juce::Decibels::gainToDecibels(dynEqEnv + 0.00001f) - currentParams.dynThresh;
             if (excessDb > 0.0f) dynGain = currentParams.dynDepth * std::min(1.0f, excessDb/20.0f);
        }
        float totalDynGain = juce::Decibels::decibelsToGain(currentParams.dynGain + dynGain);

        // Ducking Envelope (Dry Input)
        float dryL = std::abs(inputBlock.getChannelPointer(0)[s]);
        if (dryL > duckEnv) duckEnv += (dryL - duckEnv) * duckAtt;
        else duckEnv += (dryL - duckEnv) * duckRel;

        float duckGain = std::max(0.0f, 1.0f - (duckEnv * duckIntensity * 4.0f));

        // Apply Processes per channel
        for (size_t ch=0; ch<nChannels; ++ch)
        {
            float samp = wetBlock.getChannelPointer(ch)[s];

            // Gate
            samp *= gateEnv;

            // DynEQ (Peak Approx)
            float bp = dynEqFilter.processSample((int)ch, samp);
            samp = samp + (totalDynGain - 1.0f) * bp;

            // Ducking
            samp *= duckGain;

            wetBlock.getChannelPointer(ch)[s] = samp;
        }
    }

    // 2.6 3-Band EQ
    eq3Chain.process(wetContext);

    // 2.7 M/S Balance
    if (nChannels == 2)
    {
        float balance = currentParams.msBalance / 100.0f;
        for (size_t s=0; s<nSamples; ++s)
        {
            float l = wetBlock.getChannelPointer(0)[s];
            float r = wetBlock.getChannelPointer(1)[s];
            float m = (l + r) * 0.5f;
            float side = (l - r) * 0.5f;

            float mGain = (balance < 0.5f) ? 1.0f : 2.0f * (1.0f - balance);
            float sGain = (balance > 0.5f) ? 1.0f : balance * 2.0f;

            wetBlock.getChannelPointer(0)[s] = m * mGain + side * sGain;
            wetBlock.getChannelPointer(1)[s] = m * mGain - side * sGain;
        }
    }

    // 2.9 Mix
    float wetAmt = currentParams.mix / 100.0f;
    float dryAmt = 1.0f - wetAmt;

    outputBlock.multiplyBy(dryAmt);
    for (size_t ch=0; ch<nChannels; ++ch)
        juce::FloatVectorOperations::addWithMultiply(outputBlock.getChannelPointer(ch), wetBlock.getChannelPointer(ch), wetAmt, nSamples);

    // 2.10 Limiter
    if (currentParams.limiterOn)
        limiter.process(context);
}
