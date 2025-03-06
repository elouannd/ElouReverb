/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class ElouReverbAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ElouReverbAudioProcessor();
    ~ElouReverbAudioProcessor() override;

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
    juce::AudioProcessorValueTreeState apvts;
    
    // Add this method to reset the reverb state
    void clearReverbState() {
        reverb.reset();
    }
    
    static void logMessage(const juce::String& message);
    
private:
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    
    // Parameter pointers
    std::atomic<float>* roomSizeParameter = nullptr;
    std::atomic<float>* dampingParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;  // Single mix parameter
    std::atomic<float>* saturationParameter = nullptr; // New saturation parameter
    std::atomic<float>* panParameter = nullptr;        // New pan parameter
    std::atomic<float>* predelayParameter = nullptr;    // New
    std::atomic<float>* lowCutParameter = nullptr;      // New
    std::atomic<float>* highCutParameter = nullptr;     // New
    
    // Filters
    juce::IIRFilter lowCutFilter[2];  // Stereo low cut filter (high pass)
    juce::IIRFilter highCutFilter[2]; // Stereo high cut filter (low pass)
    
    // Delay buffer for predelay
    juce::AudioBuffer<float> predelayBuffer;
    int predelayBufferWritePosition = 0;
    int predelayBufferSize = 0;
    int maxDelayInSamples = 0;
    double sampleRate = 44100.0;
    
    // Helper functions
    float applySaturation(float sample, float amount);
    void applyPredelay(juce::AudioBuffer<float>& buffer, float delayTimeMs);
    
    // Parameter creation function
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElouReverbAudioProcessor)
};
