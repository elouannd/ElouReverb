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
    // Load the images from binary resources instead of files
    knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Set up all sliders
    setupSlider(roomSizeSlider, 0.1f, 25.0f, 0.01f, " s");
    roomSizeSlider.setName("roomSize");
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
            roomSizeSlider.setTextValueSuffix("++ s");
        } else if (value > 15.0f) {
            roomSizeSlider.setTextValueSuffix("+ s");
        } else {
            roomSizeSlider.setTextValueSuffix(" s");
        }
    };
    
    // Setup color selection buttons
    const std::pair<juce::String, juce::Colour> colors[] = {
        {"Orange", juce::Colour(0xFFE67E22)},
        {"Blue", juce::Colour(0xFF3498DB)},
        {"Green", juce::Colour(0xFF2ECC71)},
        {"Purple", juce::Colour(0xFF9B59B6)},
        {"Red", juce::Colour(0xFFE74C3C)}
    };
    
    colorLabel.setText("Couleur:", juce::dontSendNotification);
    colorLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    colorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    colorLabel.setColour(juce::Label::outlineColourId, juce::Colours::black);
    colorLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.3f));
    addAndMakeVisible(colorLabel);
    
    for (const auto& [name, color] : colors)
    {
        auto button = std::make_unique<ColorButton>(name, color);
        button->addListener(this);
        addAndMakeVisible(*button);
        colorButtons.push_back(std::move(button));
    }

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
    
    setResizable(true, true);
    setResizeLimits(600, 400, 1200, 800);
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
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, 
                              juce::MathConstants<float>::pi * 2.8f,
                              true);
    
    if (suffix && suffix[0] != '\0')
        slider.setTextValueSuffix(suffix);
        
    // Add text outline effect
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.6f));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);
    slider.setColour(juce::Slider::textBoxHighlightColourId, juce::Colours::white.withAlpha(0.2f));
        
    slider.setLookAndFeel(&knobLookAndFeel);
    addAndMakeVisible(slider);
}

void ElouReverbAudioProcessorEditor::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(16.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    // Add a dark outline effect
    label.setColour(juce::Label::textWhenEditingColourId, juce::Colours::white);
    label.setColour(juce::Label::outlineWhenEditingColourId, juce::Colours::black);
    addAndMakeVisible(label);
}

//==============================================================================
// Update the paint method for frosted glass effect:

void ElouReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Get the theme color first
    juce::Colour mainThemeColor = knobLookAndFeel.getMainColour();
    
    if (easterEggMode && backgroundImage.isValid()) {
        // Draw the custom background image
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    } else {
        // Original background drawing
        juce::Colour darkThemeColor = mainThemeColor.withBrightness(0.2f);
        
        juce::ColourGradient backgroundGradient(
            darkThemeColor, 0, 0,  
            darkThemeColor.darker(0.7f), getWidth(), getHeight(),  
            true
        );
        g.setGradientFill(backgroundGradient);
        g.fillAll();
    }

    // Function to draw outlined text
    auto drawOutlinedText = [&](const juce::String& text, int x, int y, int width, int height, 
                               juce::Justification justification, const juce::Colour& textColor) {
        // Draw text outline/shadow
        g.setColour(juce::Colours::black);
        float outlineThickness = 1.5f;
        for (float xOffset = -outlineThickness; xOffset <= outlineThickness; xOffset += outlineThickness) {
            for (float yOffset = -outlineThickness; yOffset <= outlineThickness; yOffset += outlineThickness) {
                g.drawText(text, x + xOffset, y + yOffset, width, height, justification, true);
            }
        }
        // Draw main text
        g.setColour(textColor);
        g.drawText(text, x, y, width, height, justification, true);
    };

    // Plugin title
    g.setFont(juce::Font(36.0f, juce::Font::bold));
    if (easterEggMode) {
        drawOutlinedText("Ryan", 20, 20, 100, 40, juce::Justification::left, mainThemeColor);
        drawOutlinedText("Gosling Reverb", 120, 20, 250, 40, juce::Justification::left, mainThemeColor.darker(0.3f));
    } else {
        drawOutlinedText("Elou", 20, 20, 100, 40, juce::Justification::left, mainThemeColor);
        drawOutlinedText("Reverb", 120, 20, 150, 40, juce::Justification::left, mainThemeColor.darker(0.3f));
    }
    
    // Version and credit with outline
    g.setFont(juce::Font(12.0f));
    drawOutlinedText("V3 - Elouann 2025", 
                     getWidth() - 200, 25, 
                     180, 20, 
                     juce::Justification::right, 
                     juce::Colours::white.withAlpha(0.6f));
              
    // Draw sections with borders using theme color
    auto drawSection = [&](juce::Rectangle<int> bounds, const juce::String& title) {
        g.setColour(mainThemeColor.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 2.0f);
        
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        drawOutlinedText(title, 
                        bounds.getX(), bounds.getY() - 25, 
                        bounds.getWidth(), 20, 
                        juce::Justification::centred, 
                        juce::Colours::white);
    };
    
    // Main sections
    juce::Rectangle<int> mainSection(50, 100, getWidth() - 100, getHeight() - 200);
    drawSection(mainSection, "MAIN CONTROLS");
}

void ElouReverbAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Header space
    bounds.removeFromTop(80);
    
    // Space for color buttons at the bottom
    auto colorSection = bounds.removeFromBottom(40);
    
    // Position color label and buttons
    colorLabel.setBounds(colorSection.removeFromLeft(80));
    const int buttonWidth = 40;
    const int buttonSpacing = 10;
    const int buttonsStartX = colorSection.getX() + 10;
    
    for (size_t i = 0; i < colorButtons.size(); ++i)
    {
        colorButtons[i]->setBounds(buttonsStartX + (buttonWidth + buttonSpacing) * i,
                                 colorSection.getY() + 10,
                                 buttonWidth,
                                 20);
    }
    
    // Calculate dimensions
    const int padding = 30;
    bounds.reduce(padding, padding);
    
    // Main section
    auto mainSection = bounds;
    mainSection.reduce(20, 20);
    
    // Standard knob size
    const int knobSize = juce::jmin(mainSection.getWidth() / 6, mainSection.getHeight() / 3);
    // Decay knob size (25% larger)
    const int decayKnobSize = static_cast<int>(knobSize * 1.25f);
    
    // Main controls layout
    auto topRow = mainSection.removeFromTop(mainSection.getHeight() / 2);
    
    // First row: Room Size, Damping, Mix
    auto roomArea = topRow.removeFromLeft(topRow.getWidth() / 3);
    auto dampArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    auto mixArea = topRow;
    
    // Labels
    const int labelHeight = 25;
    roomSizeLabel.setBounds(roomArea.removeFromTop(labelHeight));
    dampingLabel.setBounds(dampArea.removeFromTop(labelHeight));
    mixLabel.setBounds(mixArea.removeFromTop(labelHeight));
    
    // First row knobs
    roomSizeSlider.setBounds(roomArea.withSizeKeepingCentre(decayKnobSize, decayKnobSize));
    dampingSlider.setBounds(dampArea.withSizeKeepingCentre(knobSize, knobSize));
    mixSlider.setBounds(mixArea.withSizeKeepingCentre(knobSize, knobSize));
    
    // Second row: Saturation and Pan
    auto bottomRow = mainSection;
    auto saturationArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto panArea = bottomRow;
    
    // Second row labels
    saturationLabel.setBounds(saturationArea.removeFromTop(labelHeight));
    panLabel.setBounds(panArea.removeFromTop(labelHeight));
    
    // Second row knobs
    saturationSlider.setBounds(saturationArea.withSizeKeepingCentre(knobSize, knobSize));
    panSlider.setBounds(panArea.withSizeKeepingCentre(knobSize, knobSize));
}

void ElouReverbAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (auto colorBtn = dynamic_cast<ColorButton*>(button))
    {
        knobLookAndFeel.setMainColour(colorBtn->getColour());
        repaint();
    }
}

void ElouReverbAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Check if click is in title area
    if (event.y < 80 && event.x < 270)  // Title area dimensions
    {
        if (easterEggMode) {
            // If already in easter egg mode, switch back to normal mode
            easterEggMode = false;
            titleClickCount = 0;
            // Disable easter egg mode for knobs
            knobLookAndFeel.setEasterEggMode(false, knobImage);
            // Force a repaint to show the changes
            repaint();
        } else {
            // Normal easter egg activation logic
            titleClickCount++;
            if (titleClickCount >= 10)
            {
                easterEggMode = true;
                // Enable easter egg mode for knobs
                knobLookAndFeel.setEasterEggMode(true, knobImage);
                // Force a repaint to show the changes
                repaint();
            }
        }
    }
}
