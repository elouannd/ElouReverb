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
    // Set up Room Size slider
    roomSizeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    roomSizeSlider.setRange(0.1f, 25.0f, 0.01f); // Changed from 90.0f to 25.0f
    roomSizeSlider.setSkewFactorFromMidPoint(8.0f); // Make it logarithmic after 8 seconds
    roomSizeSlider.setTextValueSuffix(" s");
    roomSizeSlider.setLookAndFeel(&knobLookAndFeel);
    addAndMakeVisible(roomSizeSlider);
    
    roomSizeLabel.setText("Decay Time", juce::dontSendNotification);
    roomSizeLabel.setJustificationType(juce::Justification::centred);
    roomSizeLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(roomSizeLabel);
    
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
    
    // Apply look and feel to all sliders
    dampingSlider.setLookAndFeel(&knobLookAndFeel);
    mixSlider.setLookAndFeel(&knobLookAndFeel);
    saturationSlider.setLookAndFeel(&knobLookAndFeel);
    panSlider.setLookAndFeel(&knobLookAndFeel);
    
    // Make window resizable
    setResizable(true, true);
    setResizeLimits(600, 400, 1200, 800);
    
    // Set the editor size
    setSize (800, 500);
    
    // Damping Slider
    dampingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    dampingSlider.setRange(0.0f, 1.0f, 0.01f);
    dampingSlider.setValue(0.5f);
    addAndMakeVisible(dampingSlider);
    
    dampingLabel.setText("Damping", juce::dontSendNotification);
    dampingLabel.setJustificationType(juce::Justification::centred);
    dampingLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(dampingLabel);
    
    // Mix Slider (replacing Wet/Dry)
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    mixSlider.setRange(0.0f, 1.0f, 0.01f);
    mixSlider.setValue(0.33f);
    addAndMakeVisible(mixSlider);
    
    mixLabel.setText("Mix (Wet/Dry)", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(mixLabel);

    // Warmth Slider - now with range 0-0.5
    saturationSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    saturationSlider.setRange(0.0f, 0.5f, 0.01f);
    saturationSlider.setValue(0.2f);
    addAndMakeVisible(saturationSlider);
    
    saturationLabel.setText("Warmth", juce::dontSendNotification);
    saturationLabel.setJustificationType(juce::Justification::centred);
    saturationLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(saturationLabel);
    
    // Pan Slider
    panSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    panSlider.setRange(-1.0f, 1.0f, 0.01f);
    panSlider.setValue(0.0f);
    addAndMakeVisible(panSlider);
    
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.setJustificationType(juce::Justification::centred);
    panLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(panLabel);

    // Add value tree attachments
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
}

ElouReverbAudioProcessorEditor::~ElouReverbAudioProcessorEditor()
{
    roomSizeSlider.setLookAndFeel(nullptr);
    dampingSlider.setLookAndFeel(nullptr);
    mixSlider.setLookAndFeel(nullptr);
    saturationSlider.setLookAndFeel(nullptr);
    panSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void ElouReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background with subtle gradient
    juce::ColourGradient backgroundGradient(
        juce::Colour(30, 30, 35), 0, 0,  // Dark at top
        juce::Colour(20, 20, 25), 0, getHeight(),  // Darker at bottom
        false
    );
    g.setGradientFill(backgroundGradient);
    g.fillAll();
    
    // Title with gradient fill and shadow
    juce::Rectangle<float> titleArea(0.0f, 10.0f, static_cast<float>(getWidth()), 40.0f);
    
    // Title shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("ElouReverb", titleArea.translated(2.0f, 2.0f), juce::Justification::centred, false);
    
    // Title with gradient
    juce::ColourGradient titleGradient(
        juce::Colour(255, 165, 0),       // Orange
        titleArea.getX(), titleArea.getY(),
        juce::Colour(255, 140, 0),       // Darker orange
        titleArea.getX(), titleArea.getBottom(),
        false
    );
    g.setGradientFill(titleGradient);
    g.setFont(juce::Font(28.0f, juce::Font::bold));
    g.drawText("ElouReverb", titleArea, juce::Justification::centred, true);
    
    // Version and copyright with subtle styling
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(12.0f));
    g.drawText("V1 - Elouann 2025", 
              getWidth() - 200, getHeight() - 20, 190, 20, 
              juce::Justification::right, true);
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
