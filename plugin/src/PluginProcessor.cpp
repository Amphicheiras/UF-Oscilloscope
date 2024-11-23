#include "UF-Oscilloscope/PluginProcessor.h"
#include "UF-Oscilloscope/PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("MainInput", juce::AudioChannelSet::stereo())
              .withInput("AuxInput1", juce::AudioChannelSet::stereo())
              //   .withInput("AuxInput2", juce::AudioChannelSet::stereo())
              //   .withInput("AuxInput3", juce::AudioChannelSet::stereo())
              //   .withInput("AuxInput4", juce::AudioChannelSet::stereo())
              .withOutput("Output", juce::AudioChannelSet::stereo()))
{
}

PluginProcessor::~PluginProcessor() {}

const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0
              // programs, so this should be at least 1, even if you're not
              // really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PluginProcessor::changeProgramName(int index,
                                        const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    mainInputBufferHistory.setSize(2, historyBufferSize);
    sidechainBuffer1History.setSize(2, historyBufferSize);
    // sidechainBuffer2History.setSize(2, historyBufferSize);
    // sidechainBuffer3History.setSize(2, historyBufferSize);
    // sidechainBuffer4History.setSize(2, historyBufferSize);
    // clear the buffer with zeroes
    mainInputBufferHistory.clear();
    sidechainBuffer1History.clear();
    // sidechainBuffer2History.clear();
    // sidechainBuffer3History.clear();
    // sidechainBuffer4History.clear();
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mainInputBufferHistory.clear();
    sidechainBuffer1History.clear();
    // sidechainBuffer2History.clear();
    // sidechainBuffer3History.clear();
    // sidechainBuffer4History.clear();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages)
{
    mainInputBuffer = getBusBuffer(buffer, true, 0);
    sidechainBuffer1 = getBusBuffer(buffer, true, 1);
    // sidechainBuffer2 = getBusBuffer(buffer, true, 2);
    // sidechainBuffer3 = getBusBuffer(buffer, true, 3);
    // sidechainBuffer4 = getBusBuffer(buffer, true, 4);
    auto output = getBusBuffer(buffer, false, 0);
    // **************************************************
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);
    // **************************************************

    processBufferHistory(mainInputBufferHistory, mainInputBuffer, 2, numSamples, 0);
    output.addFrom(0, 0, mainInputBuffer, 0, 0, numSamples);
    output.addFrom(1, 0, mainInputBuffer, 1, 0, numSamples);

    if (sidechainBuffer1.getNumChannels() > 1)
    {
        processBufferHistory(sidechainBuffer1History, sidechainBuffer1, 2, numSamples, 1);
        output.addFrom(0, 0, sidechainBuffer1, 0, 0, numSamples);
        output.addFrom(1, 0, sidechainBuffer1, 1, 0, numSamples);
    }

    // if (sidechainBuffer2.getNumChannels() > 1)
    // {
    //     processBufferHistory(sidechainBuffer2History, sidechainBuffer2, 2, numSamples);
    //     output.addFrom(0, 0, sidechainBuffer2, 0, 0, numSamples);
    //     output.addFrom(1, 0, sidechainBuffer2, 1, 0, numSamples);
    // }

    // if (sidechainBuffer3.getNumChannels() > 1)
    // {
    //     processBufferHistory(sidechainBuffer3History, sidechainBuffer3, 2, numSamples);
    //     output.addFrom(0, 0, sidechainBuffer3, 0, 0, numSamples);
    //     output.addFrom(1, 0, sidechainBuffer3, 1, 0, numSamples);
    // }

    // if (sidechainBuffer4.getNumChannels() > 1)
    // {
    //     processBufferHistory(sidechainBuffer4History, sidechainBuffer4, 2, numSamples);
    //     output.addFrom(0, 0, sidechainBuffer4, 0, 0, numSamples);
    //     output.addFrom(1, 0, sidechainBuffer4, 1, 0, numSamples);
    // }

    setBPM();
}

bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the getStateInformation()
    // call.
    juce::ignoreUnused(data, sizeInBytes);
}

void PluginProcessor::setBPM()
{
    if (getPlayHead()->getPosition()->getBpm().hasValue())
        bpm = getPlayHead()->getPosition()->getBpm();
    else
        bpm = 0.0;
}

juce::Optional<double> PluginProcessor::getBPM()
{
    return bpm;
}

// Get audio history
const juce::AudioBuffer<float> &PluginProcessor::getHistoryBuffer(int channel) const
{
    if (channel == 0)
        return mainInputBufferHistory;
    else if (channel == 1)
        return sidechainBuffer1History;
    // else if (channel == 2)
    //     return sidechainBuffer2History;
    // else if (channel == 3)
    //     return sidechainBuffer3History;
    // else if (channel == 4)
    //     return sidechainBuffer4History;
    return mainInputBufferHistory;
}

void PluginProcessor::processBufferHistory(juce::AudioBuffer<float> &historyBuffer, const juce::AudioBuffer<float> &buffer, int numChannels, int numSamples, int bufferID)
{
    if (historyFlag)
    {
        historyBuffer.setSize(numChannels, historyBufferSize);
        historyBuffer.clear();

        historyFlag = false;
    }

    int bufferCopySize = std::min(historyBufferSize, numSamples);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float *historyChannelPointer = historyBuffer.getWritePointer(channel); // Get write pointer for each channel
        auto *bufferChannelPointer = buffer.getReadPointer(channel);
        // If history buffer is smaller than the audio buffer
        if (historyBufferSize < numSamples)
        {
            std::copy(bufferChannelPointer, bufferChannelPointer + bufferCopySize, historyChannelPointer);
        }
        else
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                historyChannelPointer[(currentIndex[bufferID] + sample) % historyBufferSize] = bufferChannelPointer[sample];
            }
        }
    }
    currentIndex[bufferID] = (currentIndex[bufferID] + numSamples) % historyBufferSize;
}

void PluginProcessor::setHistoryBufferSize(int size)
{
    historyBufferSize = size;
    historyFlag = true;
}

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}