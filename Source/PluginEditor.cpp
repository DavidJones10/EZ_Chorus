/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EZChorusAudioProcessorEditor::EZChorusAudioProcessorEditor (EZChorusAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&otherLookAndFeel);
    setSize(400, 400);
        
        
    feedbackSlider.setLookAndFeel(&otherLookAndFeel);
    offsetSlider.setLookAndFeel(&otherLookAndFeel);
    mixSlider.setLookAndFeel(&otherLookAndFeel);
    modRateSlider.setLookAndFeel(&otherLookAndFeel);
    modDepthSlider.setLookAndFeel(&otherLookAndFeel);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    feedbackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FEEDBACK", feedbackSlider);
    offsetAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "PHASEOFFSET", offsetSlider);
    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MIX", mixSlider);
    modDepthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CHORUSDEPTH", modDepthSlider);
    modRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CHORUSRATE", modRateSlider);


    setSliderParametersDial(feedbackSlider);
    setSliderParametersDial(offsetSlider);
    setSliderParametersDial(mixSlider);
    setSliderParametersDial(modRateSlider);
    setSliderParametersDial(modDepthSlider);


    
    feedbackSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    offsetSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    mixSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    modRateSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
    modDepthSlider.setRotaryParameters(4 * pi / 3, 8 * pi /3, true);
}

EZChorusAudioProcessorEditor::~EZChorusAudioProcessorEditor()
{
    setLookAndFeel((nullptr));
}

//==============================================================================
void EZChorusAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto sliderPosFeedback = feedbackSlider.getValue() * ( pi / 3)/ feedbackSlider.getMaximum();
    auto sliderPosOffset= offsetSlider.getValue() * (pi / 3)/ offsetSlider.getMaximum();
    auto sliderPosMix = mixSlider.getValue() * (pi / 3)/ mixSlider.getMaximum();
    auto sliderPosRate = modRateSlider.getValue() * (pi / 3)/ modRateSlider.getMaximum();
    auto sliderPosDepth = modDepthSlider.getValue() * (pi / 3)/ modDepthSlider.getMaximum();
    
    //g.drawFittedText(<#const String &text#>, <#Rectangle<int> area#>, Justification::centredTop, 1);

       
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        
    drawRotarySlider(g, row1X, column1Y, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosMix, 4 * pi / 3, 8*pi/3, mixSlider, String("Mix"));
    drawRotarySlider(g, mixSlider.getRight() + horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosOffset, 4 * pi / 3, 8 * pi /3, offsetSlider, String("Phase Offset"));
    drawRotarySlider(g, mixSlider.getX(), offsetSlider.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosFeedback , 4 * pi / 3, 8 * pi /3, feedbackSlider, String("Feedback"));
    drawRotarySlider(g, offsetSlider.getRight() + horizontalDistance, offsetSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosRate, 4 * pi / 3, 8 * pi /3, modRateSlider, String("Mod Rate"));
    drawRotarySlider(g, feedbackSlider.getRight() + horizontalDistance, feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosDepth, 4 * pi / 3, 8 * pi /3, modDepthSlider, String("Mod Depth"));
    //g.drawRoundedRectangle(<#float x#>, <#float y#>, <#float width#>, <#float height#>, <#float cornerSize#>, <#float lineThickness#>)
        
}

void EZChorusAudioProcessorEditor::resized()
{
    mixSlider.setBounds(row1X, column1Y, sliderWidthAndHeight, sliderWidthAndHeight);
    offsetSlider.setBounds(mixSlider.getRight() + horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight);
    feedbackSlider.setBounds(mixSlider.getX(), offsetSlider.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight);
    lowpassFreqSlider.setBounds(feedbackSlider.getRight() + horizontalDistance, feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight);
    modRateSlider.setBounds(offsetSlider.getRight() + horizontalDistance, offsetSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight);
    modDepthSlider.setBounds(feedbackSlider.getRight() + horizontalDistance, feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight);
}
