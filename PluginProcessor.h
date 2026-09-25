#define JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE 1

#pragma once

#include "JuceHeader.h"
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

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

    inline void Hadamard16 (float* Hadamard);

    float filter[32];
    float filter2[24];
    float LFO1=0.0f;
    float LFO2=0.0f;
    float LFO3=0.0f;
    float LFO4=0.0f;
    int combindx =0;
    int apfindx =0;

    float combL[4096];
    float combR[4096];
    float combL2[4096];
    float combR2[4096];
    float combL3[4096];
    float combR3[4096];
    float combL4[4096];
    float combR4[4096];
    float combL5[4096];
    float combR5[4096];
    float combL6[4096];
    float combR6[4096];
    float combL7[4096];
    float combR7[4096];
    float combL8[4096];
    float combR8[4096];

    float allpasL[1024];
    float allpasR[1024];
    float allpasL2[1024];
    float allpasR2[1024];
    float allpasL3[1024];
    float allpasR3[1024];
    float allpasL4[1024];
    float allpasR4[1024];
    float allpasL5[1024];
    float allpasR5[1024];
    float allpasL6[1024];
    float allpasR6[1024];
    
    static inline const int ReverbLFO[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, -1, -2, -3, -4, -5, -6, -7, -8, -8, -7, -6, -5, -4, -3, -2, -1, 0};
    
    std::atomic<float>* P0s = nullptr;
    std::atomic<float>* P1s = nullptr;
    std::atomic<float>* P2s = nullptr;
    std::atomic<float>* P3s = nullptr;
    std::atomic<float>* P4s = nullptr;
    std::atomic<float>* P5s = nullptr;
    
    juce::AudioProcessorValueTreeState STATE;
    juce::AudioProcessorValueTreeState::ParameterLayout paral();

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
   
};
