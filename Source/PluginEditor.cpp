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
    setSize(365, 325);
    Timer::startTimerHz(20);
    
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

    setSliderParametersDial(feedbackSlider, true);
    setSliderParametersDial(offsetSlider, true);
    setSliderParametersDial(mixSlider, true);
    setSliderParametersDial(modRateSlider, true);
    setSliderParametersDial(modDepthSlider, true);
    
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
    auto sliderPosFeedback = feedbackSlider.getValue() * (pi / (pi+.07))/ feedbackSlider.getMaximum();
    auto sliderPosOffset= offsetSlider.getValue() / offsetSlider.getMaximum();
    auto sliderPosMix = mixSlider.getValue() / mixSlider.getMaximum();
    auto sliderPosRate = modRateSlider.getValue() / modRateSlider.getMaximum();
    auto sliderPosDepth = modDepthSlider.getValue() / modDepthSlider.getMaximum();
    auto titleFont = Font("Euphemia UCAS", 60.0f, Font::plain);
    auto fillRect2 = Rectangle<float>(mixSlider.getX()-5, mixSlider.getY() - 5, mixSlider.getWidth()+10, mixSlider.getHeight() + 25);
       
    g.fillAll (Colours::black);
    g.setFont(titleFont);
    g.setColour(Colours::skyblue);
    g.setOpacity(1);
    g.drawFittedText(String("EZ Chorus"), 150, 0, 200, 60, Justification::centredTop, 1);
    g.setColour(Colours::grey);
    g.setOpacity(.50);
    g.fillRoundedRectangle(fillRect2, 10);
    g.setColour(Colours::white);
    g.drawRoundedRectangle(fillRect2, 10, 2);
    
    drawGroupRectangle(offsetSlider, feedbackSlider, String("Stuff"), g);
    drawGroupRectangle(modRateSlider, modDepthSlider, String("Stuff"), g);
    drawRotarySlider(g, row1X, column1Y + 50, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosMix, 4 * pi / 3, 8*pi/3, mixSlider, String("Mix"));
    drawRotarySlider(g, mixSlider.getRight() + horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosOffset, 4 * pi / 3, 8 * pi /3, offsetSlider, String("Phase Offset"));
    drawRotarySlider(g, offsetSlider.getX(), offsetSlider.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight, sliderPosFeedback , 4 * pi / 3, 8 * pi /3, feedbackSlider, String("Feedback"));
    drawRotarySlider(g, offsetSlider.getRight() + horizontalDistance, offsetSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosRate, 4 * pi / 3, 8 * pi /3, modRateSlider, String("Mod Rate"));
    drawRotarySlider(g, feedbackSlider.getRight() + horizontalDistance, feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight, sliderPosDepth, 4 * pi / 3, 8 * pi /3, modDepthSlider, String("Mod Depth"));
    drawParamText(g);
}

void EZChorusAudioProcessorEditor::resized()
{
    mixSlider.setBounds(row1X, column1Y + 50, sliderWidthAndHeight, sliderWidthAndHeight);
    offsetSlider.setBounds(mixSlider.getRight() + horizontalDistance, column1Y, sliderWidthAndHeight, sliderWidthAndHeight);
    feedbackSlider.setBounds(offsetSlider.getX(), offsetSlider.getBottom() + distanceBetweenSlidersVertical, sliderWidthAndHeight, sliderWidthAndHeight);
    modRateSlider.setBounds(offsetSlider.getRight() + horizontalDistance, offsetSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight);
    modDepthSlider.setBounds(feedbackSlider.getRight() + horizontalDistance, feedbackSlider.getY(), sliderWidthAndHeight, sliderWidthAndHeight);
}
void EZChorusAudioProcessorEditor::drawParamText(Graphics &g)
{
    auto text = String("");
    if (feedbackSlider.isMouseOverOrDragging())
    {
        auto& fBack = *audioProcessor.apvts.getRawParameterValue("FEEDBACK");
        text = String("Feedback: " + std::to_string(fBack));
    }
    if (mixSlider.isMouseOverOrDragging())
    {
        auto& mix = *audioProcessor.apvts.getRawParameterValue("MIX");
        text = String("Mix:    " + std::to_string(mix));
    }
    if (modRateSlider.isMouseOverOrDragging())
    {
        auto& rate = *audioProcessor.apvts.getRawParameterValue("CHORUSRATE");
        text = String("Mod Rate: " + std::to_string(rate) + "Hz");
    }
    if (modDepthSlider.isMouseOverOrDragging())
    {
        auto& depth = *audioProcessor.apvts.getRawParameterValue("CHORUSDEPTH");
        text = String("Mod Depth: " + std::to_string(depth));
    }
    if (offsetSlider.isMouseOverOrDragging())
    {
        auto& offset = *audioProcessor.apvts.getRawParameterValue("PHASEOFFSET");
        text = String("Phase Offset: " + std::to_string(offset));
    }
    auto textRect = Rectangle<float>(2, 10, 125, 50);
    g.setColour(Colours::white);
    g.drawFittedText(text, 5, 10, 100, 50, Justification::centredTop, 2);
    g.drawRoundedRectangle(textRect, 10, 2);
    g.setColour(Colours::grey);
    g.setOpacity(.5);
    g.drawRoundedRectangle(textRect, 10, 2);
}
void EZChorusAudioProcessorEditor::timerCallback()
{
    repaint();
}
