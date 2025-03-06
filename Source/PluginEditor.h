/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Add this before the ElouReverbAudioProcessorEditor class

class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        // Define colors
        juce::Colour knobColor = juce::Colour(60, 60, 60);
        juce::Colour highlightColor = juce::Colour(200, 100, 20);
        
        // Define bounds
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centerX = bounds.getCentreX();
        auto centerY = bounds.getCentreY();
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Draw knob body
        g.setColour(knobColor);
        g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // Draw metal ring
        g.setColour(juce::Colours::lightgrey);
        float ringThickness = 2.0f;
        g.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2, ringThickness);
        
        // Draw indicator line
        g.setColour(juce::Colours::white);
        
        juce::Path p;
        auto pointerLength = radius * 0.65f;
        auto pointerThickness = 4.0f;
        
        p.addRoundedRectangle(-pointerThickness * 0.5f, -radius * 0.9f, 
                             pointerThickness, pointerLength, 1.0f);
        
        p.applyTransform(juce::AffineTransform::rotation(toAngle)
                         .translated(centerX, centerY));
        
        g.fillPath(p);
        
        // Add a highlight reflection
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillEllipse(centerX - radius * 0.4f, centerY - radius * 0.7f, radius * 0.3f, radius * 0.3f);
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
