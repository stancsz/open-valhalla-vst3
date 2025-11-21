#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "ReverbProcessor.h"

class VST3OpenValhallaAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    VST3OpenValhallaAudioProcessor();
    ~VST3OpenValhallaAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Preset Management
    void savePreset(const juce::File& file);
    void loadPreset(const juce::File& file);

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    ReverbProcessor reverbProcessor;

public:
    // Trigger Clear
    std::atomic<bool> clearTriggered { false };

    void resetAllParametersToDefault();
    void setParametersForMode(int modeIndex);

private:

    // Mode Enum
    enum Mode
    {
        TwinStar = 0,        // Gemini
        SeaSerpent,          // Hydra
        HorseMan,            // Centaurus
        Archer,              // Sagittarius
        VoidMaker,           // Great Annihilator
        GalaxySpiral,        // Andromeda
        HarpString,          // Lyra
        GoatHorn,            // Capricorn
        NebulaCloud,         // Large Magellanic Cloud
        Triangle,            // Triangulum
        CloudMajor,          // Cirrus Major
        CloudMinor,          // Cirrus Minor
        QueenChair,          // Cassiopeia
        HunterBelt,          // Orion
        WaterBearer,         // Aquarius
        TwoFish,             // Pisces
        ScorpionTail,        // Scorpio
        BalanceScale,        // Libra
        LionHeart,           // Leo
        Maiden,              // Virgo
        SevenSisters         // Pleiades
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3OpenValhallaAudioProcessor)
};
