/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ElouReverbAudioProcessorEditor::ElouReverbAudioProcessorEditor (ElouReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up all sliders
    setupSlider(roomSizeSlider, 0.1f, 25.0f, 0.01f, " s");
    setupLabel(roomSizeLabel, "Temps");
    
    setupSlider(dampingSlider, 0.0f, 1.0f, 0.01f);
    setupLabel(dampingLabel, "Etouffement");
    
    setupSlider(mixSlider, 0.0f, 1.0f, 0.01f);
    setupLabel(mixLabel, "Mix (Sec/Mouille)");
    
    setupSlider(saturationSlider, 0.0f, 0.5f, 0.01f);
    setupLabel(saturationLabel, "Warmth");
    
    setupSlider(panSlider, -1.0f, 1.0f, 0.01f);
    setupLabel(panLabel, "Pan");
    
    // Custom text display for very long decay times
    roomSizeSlider.onValueChange = [this]() {
        float value = roomSizeSlider.getValue();
        if (value >= 20.0f) {
            roomSizeSlider.setTextValueSuffix("++ s"); // Use ASCII only
        } else if (value > 15.0f) {
            roomSizeSlider.setTextValueSuffix("+ s");
        } else {
            roomSizeSlider.setTextValueSuffix(" s");
        }
    };
    
    // Create attachments
    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "roomSize", roomSizeSlider);
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "damping", dampingSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mix", mixSlider);
    saturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "saturation", saturationSlider);
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "pan", panSlider);
    
    // Make window resizable
    setResizable(true, true);
    setResizeLimits(600, 400, 1200, 800);
    
    // Set the editor size
    setSize(800, 500);
}

ElouReverbAudioProcessorEditor::~ElouReverbAudioProcessorEditor()
{
    roomSizeSlider.setLookAndFeel(nullptr);
    dampingSlider.setLookAndFeel(nullptr);
    mixSlider.setLookAndFeel(nullptr);
    saturationSlider.setLookAndFeel(nullptr);
    panSlider.setLookAndFeel(nullptr);
}

// Add these implementations after the constructor but before other functions

void ElouReverbAudioProcessorEditor::setupSlider(juce::Slider& slider, float min, float max, float step, const char* suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    slider.setRange(min, max, step);
    
    if (suffix && suffix[0] != '\0')
        slider.setTextValueSuffix(suffix);
        
    slider.setLookAndFeel(&knobLookAndFeel);
    addAndMakeVisible(slider);
}

void ElouReverbAudioProcessorEditor::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(16.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(label);
}

//==============================================================================
// Update the paint method for frosted glass effect:

void ElouReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Simple dark gradient background
    juce::ColourGradient backgroundGradient(
        juce::Colour(0xFF202020), 0, 0,
        juce::Colour(0xFF101010), 0, getHeight(),
        false
    );
    g.setGradientFill(backgroundGradient);
    g.fillAll();
    
    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("ElouReverb", getLocalBounds().removeFromTop(60), juce::Justification::centred, true);
    
    // Version info - updated to V2
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(12.0f));
    g.drawText("V2 - Elouann 2025", 
              getWidth() - 200, getHeight() - 25, 
              190, 20, juce::Justification::right, true);
}

void ElouReverbAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve space for the title header
    auto headerArea = bounds.removeFromTop(60);
    
    // Calculate proportional padding and sizes
    const int padding = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 20;
    
    bounds.reduce(padding, padding);
    
    // Adjust vertical position for controls
    bounds.removeFromTop(padding);
    
    // Split into rows
    auto topRow = bounds.removeFromTop(bounds.getHeight() / 2);
    auto bottomRow = bounds;
    
    const int knobSize = juce::jmin(
        (topRow.getWidth() - padding * 2) / 3,  // Width-based size
        topRow.getHeight() - 30                // Height-based constraint
    );
    
    // Top row for original controls
    auto topLabelsRow = topRow.removeFromTop(25);
    
    // Divide horizontally for the three original controls
    auto roomArea = topRow.removeFromLeft(topRow.getWidth() / 3);
    auto dampArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    auto mixArea = topRow;
    
    // Labels for top row
    roomSizeLabel.setBounds(topLabelsRow.removeFromLeft(topLabelsRow.getWidth() / 3));
    dampingLabel.setBounds(topLabelsRow.removeFromLeft(topLabelsRow.getWidth() / 2));
    mixLabel.setBounds(topLabelsRow);
    
    // Center the knobs in their areas
    roomSizeSlider.setBounds(
        roomArea.getCentreX() - knobSize / 2, 
        roomArea.getCentreY() - knobSize / 2,
        knobSize, 
        knobSize
    );
    
    dampingSlider.setBounds(
        dampArea.getCentreX() - knobSize / 2, 
        dampArea.getCentreY() - knobSize / 2,
        knobSize, 
        knobSize
    );
    
    mixSlider.setBounds(
        mixArea.getCentreX() - knobSize / 2, 
        mixArea.getCentreY() - knobSize / 2,
        knobSize,
        knobSize
    );
    
    // Bottom row for new controls
    auto bottomLabelsRow = bottomRow.removeFromTop(25);
    
    // Divide horizontally for the new controls
    auto saturationArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto panArea = bottomRow;
    
    // Labels for bottom row
    saturationLabel.setBounds(bottomLabelsRow.removeFromLeft(bottomLabelsRow.getWidth() / 2));
    panLabel.setBounds(bottomLabelsRow);
    
    // Center the new knobs in their areas
    saturationSlider.setBounds(
        saturationArea.getCentreX() - knobSize / 2, 
        saturationArea.getCentreY() - knobSize / 2,
        knobSize, 
        knobSize
    );
    
    panSlider.setBounds(
        panArea.getCentreX() - knobSize / 2, 
        panArea.getCentreY() - knobSize / 2,
        knobSize,
        knobSize
    );
}
