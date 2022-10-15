/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
struct OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, juce::Colours::darkblue);
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider, const String& label)
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();
 
        // fill
        g.setColour (juce::Colours::skyblue);
        g.fillEllipse (rx, ry, rw, rw);
        
        // outline
        g.setColour (juce::Colours::darkblue);
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));
        
        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        // pointer
        g.setColour (juce::Colours::white);
        g.fillPath (p);
        // textbox
        g.drawFittedText(label , x, slider.getBottom(), width, 25, Justification::centred, 3);
        g.setColour(Colours::whitesmoke);
    }
};
class EZChorusAudioProcessorEditor  : public juce::AudioProcessorEditor, public OtherLookAndFeel
{
public:
    EZChorusAudioProcessorEditor (EZChorusAudioProcessor&);
    ~EZChorusAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
         //============//
        //MY FUNCTIONS//
       //============//
       
    void setSliderParametersHorizontal (juce::Slider& slider, juce::Label& label, const juce::String& labelText, bool hasTextBox = false)
        {
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
            if (hasTextBox)
                slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
            else
                slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 25);
            addAndMakeVisible(slider);
            
            label.setText (labelText, juce::dontSendNotification);
            label.setJustificationType (juce::Justification::centred);
            addAndMakeVisible (label);
        }
    void sliderResizedHorizontal (juce::Slider& slider, juce::Label& label, int x, int y)
    {
        const auto sliderToLabelRatio = 15;
        const auto labelHeight = 17;
        const auto sliderWidth = 175;
        const auto sliderHeight = 15;
        slider.setBounds (x, y + sliderToLabelRatio, sliderWidth, sliderHeight);
        label.setBounds (slider.getX(), y, sliderHeight + 80, labelHeight);
    }
    void setSliderParametersDial (Slider& slider, bool hasTextBox = false)
    {
        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        if (hasTextBox)
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
        else
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 25);
        addAndMakeVisible(slider);
        slider.setAlpha(0);
        slider.setLookAndFeel(&otherLookAndFeel);
    }
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EZChorusAudioProcessor& audioProcessor;
    OtherLookAndFeel otherLookAndFeel;
        
    Slider feedbackSlider;
    Slider offsetSlider;
    Slider lowpassFreqSlider;
    Slider mixSlider;
    Slider modRateSlider;
    Slider modDepthSlider;
    
    
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> offsetAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lowpassAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> modRateAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> modDepthAttachment;

    const float pi = MathConstants<const float>::pi;
    int sliderWidthAndHeight = 100;
    int distanceBetweenSlidersVertical = 20;
    int horizontalDistance = 20;
    int row1X = 10;
    int column1Y = 10;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EZChorusAudioProcessorEditor)
};
