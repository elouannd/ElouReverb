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
        // Text box style
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x11ffffff));
        mainColour = juce::Colour(0xFFE67E22); // Default orange color
    }
    
    void setMainColour(juce::Colour newColour) {
        mainColour = newColour;
    }
    
    juce::Colour getMainColour() const {
        return mainColour;
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        // Dimensions de base
        float radius;
        if (slider.getName() == "roomSize") {
            radius = juce::jmin(width, height) * 0.45f; // Plus grand pour le decay
        } else {
            radius = juce::jmin(width, height) * 0.38f;
        }
        
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;
        
        // Calculate proper angle based on start and end angles
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Corps principal du knob avec la couleur personnalisée
        g.setColour(mainColour);
        g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // Indicateur de position
        const float indicatorLength = radius * 0.7f;
        const float indicatorThickness = 2.5f;
        
        juce::Path indicator;
        indicator.addRectangle(-indicatorThickness * 0.5f, -indicatorLength, 
                              indicatorThickness, indicatorLength);
        
        // Indicateur en marron foncé
        g.setColour(juce::Colour(0xFF2D1810));
        g.fillPath(indicator, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    }

private:
    juce::Colour mainColour;
};

class ColorButton : public juce::TextButton
{
public:
    ColorButton(const juce::String& name, const juce::Colour& color) 
        : juce::TextButton(name), buttonColor(color)
    {
        setSize(30, 20);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(buttonColor);
        g.fillRoundedRectangle(bounds, 4.0f);
        
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, 4.0f);
        }
    }

    juce::Colour getColour() const { return buttonColor; }

private:
    juce::Colour buttonColor;
};

//==============================================================================
/**
*/
class ElouReverbAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::Button::Listener
{
public:
    ElouReverbAudioProcessorEditor (ElouReverbAudioProcessor&);
    ~ElouReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;

private:
    void setupSlider(juce::Slider& slider, float min, float max, float step, const char* suffix = "");
    void setupLabel(juce::Label& label, const juce::String& text);
    void createAttachments();

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
    
    std::vector<std::unique_ptr<ColorButton>> colorButtons;
    juce::Label colorLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElouReverbAudioProcessorEditor)
};
