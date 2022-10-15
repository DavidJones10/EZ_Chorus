/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EZChorusAudioProcessor::EZChorusAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, apvts(*this,
       nullptr,
       "Parameters",
       { std::make_unique<AudioParameterFloat>(ParameterID("FEEDBACK", 1), "Feedback", NormalisableRange<float> { 0.0f, .98f, .001f }, 0.5f),
        std::make_unique<AudioParameterFloat>(ParameterID("PHASEOFFSET",1), "Phase Offset", NormalisableRange<float> { 0.0f, 1.0, .01f }, 0.0f) ,
        std::make_unique<AudioParameterFloat>(ParameterID("MIX",1), "Mix", NormalisableRange<float> { 0.0f, 1.0f, .001f }, 0.5f),
        std::make_unique<AudioParameterFloat>(ParameterID("CHORUSDEPTH",1), "Chorus Depth", NormalisableRange<float> { 0.0f, 1.0f, .001f }, .5f),
        std::make_unique<AudioParameterFloat>(ParameterID("CHORUSRATE",1), "Chorus Rate", NormalisableRange<float> { 0.0f, 5.0f, .001f }, .5f),
       }
       )
#endif
{
    delayBufferRight = nullptr;
    delayBufferLeft = nullptr;

    bufferWriteHead = 0;
    bufferLength = 0;
    bufferLength = 0;

    delayTimeSamples = 0;
    
    feedbackLeft = 0;
    feedbackRight = 0;
    lfoPhase = 0;
}

EZChorusAudioProcessor::~EZChorusAudioProcessor()
{
    if (delayBufferRight != nullptr)
    {
        delete [] delayBufferRight;
        delayBufferRight = nullptr;
    }
    if (delayBufferLeft != nullptr)
    {
        delete [] delayBufferLeft;
        delayBufferLeft = nullptr;
    }
}

//==============================================================================
const juce::String EZChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EZChorusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EZChorusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EZChorusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EZChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EZChorusAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EZChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EZChorusAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EZChorusAudioProcessor::getProgramName (int index)
{
    return {};
}

void EZChorusAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EZChorusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    bufferLength = sampleRate * (MAX_DELAY_TIME);
    lfoPhase = 0;
    if (delayBufferLeft == nullptr)
        {
            delete [] delayBufferLeft;
            delayBufferLeft = new float[bufferLength];
        }
    zeromem(delayBufferLeft, bufferLength * sizeof(float));
    
    if (delayBufferRight == nullptr)
        {
            delete [] delayBufferRight;
            delayBufferRight = new float[bufferLength];
        }

    // Clear the buffers after instantiating!
    zeromem(delayBufferRight, bufferLength * sizeof(float));

    bufferWriteHead = 0;
}

void EZChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EZChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EZChorusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto& fBack = *apvts.getRawParameterValue("FEEDBACK");
    auto& mix = *apvts.getRawParameterValue("MIX");
    auto& modRate = *apvts.getRawParameterValue("CHORUSRATE");
    auto& modDepth = *apvts.getRawParameterValue("CHORUSDEPTH");
    auto& phaseOffset = *apvts.getRawParameterValue("PHASEOFFSET");

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    float* leftBuffer = buffer.getWritePointer(0);
    float* rightBuffer = buffer.getWritePointer(1);
    
    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {
        delayBufferRight[bufferWriteHead] = rightBuffer[sample] + feedbackRight;
        delayBufferLeft[bufferWriteHead] = leftBuffer[sample] + feedbackLeft;
        
        float lfoLeft = sin(2 * M_PI * lfoPhase);
        float lfoPhaseRight = lfoPhase + phaseOffset;
        if (lfoPhaseRight > 1)
            lfoPhase -= 1;
        
        float lfoRight = sin(2 * M_PI * lfoPhaseRight);
        
        lfoPhase += modRate / getSampleRate();
        if (lfoPhase > 1)
            lfoPhase -= 1;
        lfoRight *= modDepth;
        lfoLeft *= modDepth;
        
        float lfoMappedLeft = jmap(lfoLeft, -1.0f, 1.0f, 0.005f, 0.03f);
        float lfoMappedRight = jmap(lfoRight, -1.0f, 1.0f, 0.005f, 0.03f);
        float delayTimeSamplesLeft = getSampleRate() * lfoMappedLeft;
        float delayTimeSamplesRight = getSampleRate() * lfoMappedRight;
        
        float readHeadLeft = bufferWriteHead - delayTimeSamplesLeft;
        
        if (readHeadLeft < 0)
            readHeadLeft += bufferLength;
        
        float readHeadRight = bufferWriteHead - delayTimeSamplesRight;
        if (readHeadRight < 0 )
            readHeadRight += bufferLength;
        
        int rHeadLX = (int) readHeadLeft;
        int rHeadLX1 = rHeadLX + 1;
        float rHeadFloatL = readHeadLeft - rHeadLX;
        if (rHeadLX1 > bufferLength)
            rHeadLX1 -= bufferLength;
        
        int rHeadRX = (int) readHeadRight;
        int rHeadRX1 = rHeadRX + 1;
        float rHeadFloatR = readHeadRight - rHeadRX;
        if (rHeadRX1 > bufferLength)
            rHeadRX1 -= bufferLength;
        
        float delaySampleLeft = lerp(delayBufferLeft[rHeadLX], delayBufferLeft[rHeadLX1], rHeadFloatL);
        float delaySampleRight = lerp(delayBufferRight[rHeadRX], delayBufferRight[rHeadRX1], rHeadFloatR);

        feedbackLeft = fBack * delaySampleLeft;
        feedbackRight = fBack * delaySampleRight;
        
        bufferWriteHead++;
        
        buffer.setSample(0, sample, buffer.getSample(0, sample) * (1-mix) + delaySampleLeft * mix);
        buffer.setSample(1, sample, buffer.getSample(1, sample) * (1-mix) + delaySampleRight * mix);

        if (bufferWriteHead >= bufferLength)
            bufferWriteHead = 0;
        }
    
    
}

//==============================================================================
bool EZChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EZChorusAudioProcessor::createEditor()
{
    return new EZChorusAudioProcessorEditor (*this);
}

//==============================================================================
void EZChorusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EZChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EZChorusAudioProcessor();
}

float EZChorusAudioProcessor::lerp(float sample1, float sample2, float inPhase)
{
    return (1 - inPhase) * sample1 + inPhase * sample2;
}


