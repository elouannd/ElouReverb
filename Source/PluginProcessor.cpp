/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath> // For std::log10

//==============================================================================
ElouReverbAudioProcessor::ElouReverbAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize parameter pointers
    roomSizeParameter = apvts.getRawParameterValue("roomSize");
    dampingParameter = apvts.getRawParameterValue("damping");
    mixParameter = apvts.getRawParameterValue("mix");
    saturationParameter = apvts.getRawParameterValue("saturation"); // New
    panParameter = apvts.getRawParameterValue("pan");               // New

    // Initialize reverb parameters
    reverbParams.roomSize = roomSizeParameter->load();
    reverbParams.damping = dampingParameter->load();
    
    float mix = mixParameter->load();
    reverbParams.wetLevel = mix;
    reverbParams.dryLevel = 1.0f - mix;
    
    reverb.setParameters(reverbParams);
}

ElouReverbAudioProcessor::~ElouReverbAudioProcessor()
{
}

//==============================================================================
const juce::String ElouReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ElouReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ElouReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ElouReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ElouReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ElouReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ElouReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ElouReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ElouReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ElouReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ElouReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverb.reset();
    reverb.setSampleRate(sampleRate);
}

void ElouReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ElouReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ElouReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get the decay time in seconds
    float decayTime = roomSizeParameter->load();
    
    // Convert decay time to roomSize parameter (0-1)
    float roomSize;
    
    // Apply mapping based on decay time range
    if (decayTime <= 8.0f) {
        // Normal range (0.1 to 8.0 seconds)
        roomSize = juce::jmap(decayTime, 0.1f, 8.0f, 0.1f, 0.95f);
    } else {
        // Extended range (8.0 to 30.0 seconds)
        // Logarithmic mapping to approach 0.98 (safer max value)
        float normalizedValue = (decayTime - 8.0f) / (22.0f); // (30-8)
        float logValue = std::log10(normalizedValue * 9.0f + 1.0f) / std::log10(10.0f);
        roomSize = 0.95f + (0.98f - 0.95f) * logValue;
    }
    
    // Update reverb parameters
    reverbParams.roomSize = roomSize;
    reverbParams.damping = dampingParameter->load();
    
    float mix = mixParameter->load();   
    reverbParams.wetLevel = mix;
    reverbParams.dryLevel = 1.0f - mix;
    
    reverb.setParameters(reverbParams);
    
    // Get saturation and pan parameters
    float saturation = saturationParameter->load();
    float pan = panParameter->load();

    // Process reverb
    if (buffer.getNumChannels() == 2) {
        reverb.processStereo(buffer.getWritePointer(0),
                            buffer.getWritePointer(1),
                            buffer.getNumSamples());
                            
        // Apply saturation if needed
        if (saturation > 0.01f) {
            for (int channel = 0; channel < 2; ++channel) {
                float* channelData = buffer.getWritePointer(channel);
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                    channelData[sample] = applySaturation(channelData[sample], saturation);
                }
            }
        }
        
        // Apply panning (simple linear law)
        if (std::abs(pan) > 0.01f) {
            float leftGain = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
            float rightGain = (pan >= 0.0f) ? 1.0f : (1.0f + pan);
            
            float* leftChannel = buffer.getWritePointer(0);
            float* rightChannel = buffer.getWritePointer(1);
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                leftChannel[sample] *= leftGain;
                rightChannel[sample] *= rightGain;
            }
        }
    }
    else {
        reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
        
        // Apply saturation to mono signal if needed
        if (saturation > 0.01f) {
            float* channelData = buffer.getWritePointer(0);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                channelData[sample] = applySaturation(channelData[sample], saturation);
            }
        }
        
        // Panning doesn't apply to mono signals
    }
}

// Add this saturation helper function
float ElouReverbAudioProcessor::applySaturation(float sample, float amount)
{
    // Simple tanh-based soft clipping with drive control
    float drive = 1.0f + 15.0f * amount;
    return std::tanh(sample * drive) / (1.0f + amount * 3.0f);
}

// Add this implementation to your PluginProcessor.cpp file:
void ElouReverbAudioProcessor::logMessage(const juce::String& message)
{
    // Get the desktop folder path
    juce::File logFile(juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                       .getChildFile("ElouReverb_log.txt"));
    
    // Create a timestamp
    juce::Time now = juce::Time::getCurrentTime();
    juce::String timestamp = now.formatted("%H:%M:%S.%ms ");
    
    // Append the message to the log file
    logFile.appendText(timestamp + message + "\n");
}

//==============================================================================
bool ElouReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ElouReverbAudioProcessor::createEditor()
{
    return new ElouReverbAudioProcessorEditor (*this);
}

//==============================================================================
void ElouReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ElouReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ElouReverbAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout ElouReverbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Room Size parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("roomSize", 1),      // parameter ID with version hint
        "Decay Time",                          // parameter name
        0.1f,                                  // minimum value
        25.0f,                                 // maximum value
        8.0f                                   // default value
    ));
    
    // Damping parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("damping", 1),       // parameter ID with version hint
        "Damping",                             // parameter name
        0.0f,                                  // minimum value
        1.0f,                                  // maximum value
        0.5f                                   // default value
    ));
        
    // Mix parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1),           // parameter ID with version hint
        "Mix (Wet/Dry)",                       // parameter name
        0.0f,                                  // minimum value
        1.0f,                                  // maximum value
        0.33f                                  // default value
    ));
        
    // Saturation parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("saturation", 1),    // parameter ID with version hint
        "Warmth",                              // parameter name
        0.0f,                                  // minimum value
        0.5f,                                  // maximum value
        0.2f                                   // default value
    ));
        
    // Pan parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("pan", 1),           // parameter ID with version hint
        "Pan",                                 // parameter name
        -1.0f,                                 // minimum value
        1.0f,                                  // maximum value
        0.0f                                   // default value
    ));
    
    return layout;
}
