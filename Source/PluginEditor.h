/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Update the KnobLookAndFeel class:

class KnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KnobLookAndFeel()
    {
        // Modern color palette
        setColour(juce::Slider::thumbColourId, juce::Colour(255, 165, 0));       // Orange accent
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(255, 165, 0).withAlpha(0.8f));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(40, 40, 45));
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(50, 50, 55).withAlpha(0.6f));
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        // Calculate angles and sizes
        const float radius = juce::jmin(width, height) * 0.4f;
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Draw outer circle (knob body)
        const float outerRadius = radius * 1.1f;
        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.fillEllipse(centerX - outerRadius, centerY - outerRadius, outerRadius * 2, outerRadius * 2);
        
        // Draw inner gradient (knob face)
        juce::ColourGradient gradient(
            juce::Colour(70, 70, 75), centerX, centerY - radius * 0.4f,
            juce::Colour(30, 30, 35), centerX, centerY + radius * 0.6f,
            true
        );
        g.setGradientFill(gradient);
        g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // Draw indicator line
        g.setColour(findColour(juce::Slider::thumbColourId));
        const float lineThickness = radius * 0.1f;
        const float lineLength = radius * 0.8f;
        
        juce::Path indicator;
        indicator.addRectangle(-lineThickness * 0.5f, -lineLength, lineThickness, lineLength);
        
        g.fillPath(indicator, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
        
        // Draw indicator dot
        const float dotRadius = lineThickness;
        const float dotX = centerX + lineLength * 0.6f * std::cos(angle - juce::MathConstants<float>::halfPi);
        const float dotY = centerY + lineLength * 0.6f * std::sin(angle - juce::MathConstants<float>::halfPi);
        
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);
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
