#include "UF-Oscilloscope/PluginProcessor.h"
#include "UF-Oscilloscope/PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("MainInput", juce::AudioChannelSet::stereo())
              .withInput("AuxInput1", juce::AudioChannelSet::stereo())
              .withInput("AuxInput2", juce::AudioChannelSet::stereo())
              .withInput("AuxInput3", juce::AudioChannelSet::stereo())
              .withInput("AuxInput4", juce::AudioChannelSet::stereo())
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

    inputBuffers.resize(numSidechainInputs);
    inputHistories.resize(numSidechainInputs);
    historyBufferIndex.resize(numSidechainInputs, 0);

    for (int bufferID = 0; bufferID < numSidechainInputs; ++bufferID)
    {
        inputBuffers[bufferID].setSize(2, samplesPerBlock);
        inputHistories[bufferID].setSize(2, historyBufferSize);
        inputBuffers[bufferID].clear();
    }
}

void PluginProcessor::releaseResources()
{
    for (auto &buffer : inputHistories)
    {
        buffer.clear();
    }
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

    for (int bufferID = 0; bufferID < numSidechainInputs; ++bufferID)
    {
        auto sidechainBuffer = getBusBuffer(buffer, true, bufferID);

        if (sidechainBuffer.getNumChannels() > 0)
        {
            processBufferHistory(inputHistories[bufferID], sidechainBuffer, 2, numSamples, bufferID);
            output.addFrom(0, 0, sidechainBuffer, 0, 0, numSamples);
            output.addFrom(1, 0, sidechainBuffer, 1, 0, numSamples);
        }
    }

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
    return inputHistories[channel];
}

void PluginProcessor::processBufferHistory(juce::AudioBuffer<float> &historyBuffer, const juce::AudioBuffer<float> &buffer, int numChannels, int numSamples, int bufferID)
{
    if (historyBufferFlag)
    {
        historyBuffer.setSize(numChannels, historyBufferSize);
        historyBuffer.clear();
        historyBufferFlag = false;
    }

    int bufferCopySize = std::min(historyBufferSize, numSamples);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto *historyChannelPointer = historyBuffer.getWritePointer(channel);
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
                historyChannelPointer[(historyBufferIndex[bufferID] + sample) % historyBufferSize] = bufferChannelPointer[sample];
            }
        }
    }

    historyBufferIndex[bufferID] = (historyBufferIndex[bufferID] + numSamples) % historyBufferSize;
}

void PluginProcessor::setHistoryBufferSize(int size)
{
    historyBufferSize = size;
    historyBufferFlag = true;
}

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}