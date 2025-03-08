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
    colorLabel.setFont(juce::Font(14.0f));
    colorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
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
    // Get current knob color for theming
    juce::Colour mainThemeColor = knobLookAndFeel.getMainColour();
    juce::Colour darkThemeColor = mainThemeColor.withBrightness(0.2f);
    
    // Fond dégradé principal using the theme color
    juce::ColourGradient backgroundGradient(
        darkThemeColor, 0, 0,  
        darkThemeColor.darker(0.7f), getWidth(), getHeight(),  
        true
    );
    g.setGradientFill(backgroundGradient);
    g.fillAll();
    
    // Titre du plugin avec style Valhalla
    g.setFont(juce::Font(36.0f, juce::Font::bold));
    
    // "Elou" in theme color
    g.setColour(mainThemeColor);
    g.drawText("Elou", 20, 20, 100, 40, juce::Justification::left, true);
    
    // "Reverb" in darker theme color
    g.setColour(mainThemeColor.darker(0.3f));
    g.drawText("Reverb", 120, 20, 150, 40, juce::Justification::left, true);
    
    // Version et crédit
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(12.0f));
    g.drawText("V3 - Elouann 2025", 
              getWidth() - 200, 25, 
              180, 20, juce::Justification::right, true);
              
    // Dessiner les sections avec bordures using theme color
    auto drawSection = [&](juce::Rectangle<int> bounds, const juce::String& title) {
        g.setColour(mainThemeColor.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 2.0f);
        
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText(title, bounds.getX(), bounds.getY() - 25, bounds.getWidth(), 20, 
                  juce::Justification::centred, true);
    };
    
    // Sections principales
    juce::Rectangle<int> mainSection(50, 100, getWidth() - 100, getHeight() - 200);
    drawSection(mainSection, "CONTRÔLES PRINCIPAUX");
}

void ElouReverbAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Espace pour l'en-tête
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
    
    // Calcul des dimensions
    const int padding = 30;
    bounds.reduce(padding, padding);
    
    // Section principale
    auto mainSection = bounds;
    mainSection.reduce(20, 20);
    
    // Taille des knobs standards
    const int knobSize = juce::jmin(mainSection.getWidth() / 6, mainSection.getHeight() / 3);
    // Taille du knob de decay (25% plus grand)
    const int decayKnobSize = static_cast<int>(knobSize * 1.25f);
    
    // Disposition des contrôles principaux
    auto topRow = mainSection.removeFromTop(mainSection.getHeight() / 2);
    
    // Première rangée : Room Size, Damping, Mix
    auto roomArea = topRow.removeFromLeft(topRow.getWidth() / 3);
    auto dampArea = topRow.removeFromLeft(topRow.getWidth() / 2);
    auto mixArea = topRow;
    
    // Labels
    const int labelHeight = 25;
    roomSizeLabel.setBounds(roomArea.removeFromTop(labelHeight));
    dampingLabel.setBounds(dampArea.removeFromTop(labelHeight));
    mixLabel.setBounds(mixArea.removeFromTop(labelHeight));
    
    // Knobs de la première rangée
    roomSizeSlider.setBounds(roomArea.withSizeKeepingCentre(decayKnobSize, decayKnobSize));
    dampingSlider.setBounds(dampArea.withSizeKeepingCentre(knobSize, knobSize));
    mixSlider.setBounds(mixArea.withSizeKeepingCentre(knobSize, knobSize));
    
    // Deuxième rangée : Saturation et Pan
    auto bottomRow = mainSection;
    auto saturationArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2);
    auto panArea = bottomRow;
    
    // Labels de la deuxième rangée
    saturationLabel.setBounds(saturationArea.removeFromTop(labelHeight));
    panLabel.setBounds(panArea.removeFromTop(labelHeight));
    
    // Knobs de la deuxième rangée
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
