/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Update the KnobLookAndFeel class with analog console knob design:

class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KnobLookAndFeel()
    {
        // Simple text box styling
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x11ffffff));
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        // Basic dimensions
        const float radius = juce::jmin(width, height) * 0.38f;
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Draw simple white circle with subtle shadow for depth
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillEllipse(centerX - radius + 2, centerY - radius + 2, radius * 2, radius * 2);
        
        // Draw white circle
        g.setColour(juce::Colours::white);
        g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // Draw thin black marker line
        g.setColour(juce::Colours::black);
        const float lineThickness = radius * 0.06f; // Thin line
        const float lineLength = radius * 0.9f;     // Almost to the edge
        
        juce::Path indicator;
        indicator.addRectangle(-lineThickness * 0.5f, -lineLength, lineThickness, lineLength);
        
        g.fillPath(indicator, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    }
};

//==============================================================================
/**
*/
class ElouReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ElouReverbAudioProcessorEditor (ElouReverbAudioProcessor&);
    ~ElouReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Add these method declarations
    void setupSlider(juce::Slider& slider, float min, float max, float step, const char* suffix = "");
    // Update method declaration to accept juce::String
    void setupLabel(juce::Label& label, const juce::String& text);
    void createAttachments();

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ElouReverbAudioProcessor& audioProcessor;

    KnobLookAndFeel knobLookAndFeel;
    
    juce::Slider roomSizeSlider;
    juce::Slider dampingSlider;
    juce::Slider mixSlider;  
    juce::Slider saturationSlider;
    juce::Slider panSlider;
    
    juce::Label roomSizeLabel;
    juce::Label dampingLabel;
    juce::Label mixLabel;    
    juce::Label saturationLabel;
    juce::Label panLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElouReverbAudioProcessorEditor)
};
