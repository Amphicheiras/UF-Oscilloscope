#include "UF-Oscilloscope/PluginProcessor.h"
#include "UF-Oscilloscope/PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("MainInput", juce::AudioChannelSet::stereo())
              .withInput("SidechainInput1", juce::AudioChannelSet::stereo())
              .withInput("SidechainInput2", juce::AudioChannelSet::stereo())
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo())
#endif
      )
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    mainInputBufferHistory.clear();
    sidechainBuffer0History.clear();
    sidechainBuffer1History.clear();
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mainInputBufferHistory.clear();
    sidechainBuffer0History.clear();
    sidechainBuffer1History.clear();
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
    sidechainBuffer0 = getBusBuffer(buffer, true, 1);
    sidechainBuffer1 = getBusBuffer(buffer, true, 2);
    auto output = getBusBuffer(buffer, false, 0);
    // **************************************************
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);
    // **************************************************

    addToAudioHistory(0, mainInputBuffer);
    output.copyFrom(0, 0, mainInputBuffer, 0, 0, numSamples);
    output.copyFrom(1, 0, mainInputBuffer, 1, 0, numSamples);

    if (sidechainBuffer0.getNumChannels() > 1)
    {
        addToAudioHistory(1, sidechainBuffer0);
        output.addFrom(0, 0, sidechainBuffer0, 0, 0, numSamples);
        output.addFrom(1, 0, sidechainBuffer0, 1, 0, numSamples);
    }

    if (sidechainBuffer1.getNumChannels() > 1)
    {
        addToAudioHistory(2, sidechainBuffer1);
        output.addFrom(0, 0, sidechainBuffer1, 0, 0, numSamples);
        output.addFrom(1, 0, sidechainBuffer1, 1, 0, numSamples);
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

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor *JUCE_CALLTYPE
createPluginFilter()
{
    return new PluginProcessor();
}

// Add audio buffer to history
void PluginProcessor::addToAudioHistory(int channel, const juce::AudioBuffer<float> &buffer)
{
    std::deque<juce::AudioBuffer<float>> *targetHistory = nullptr;

    // Select the appropriate history buffer
    if (channel == 0)
        targetHistory = &mainInputBufferHistory;
    else if (channel == 1)
        targetHistory = &sidechainBuffer0History;
    else if (channel == 2)
        targetHistory = &sidechainBuffer1History;

    if (targetHistory)
    {
        // Push a copy of the buffer into the history
        targetHistory->emplace_back(buffer);

        // Trim history if it exceeds the maximum size
        while (targetHistory->size() > maxHistorySize)
            targetHistory->pop_front();
    }
}

// Get audio history
const std::deque<juce::AudioBuffer<float>> &PluginProcessor::getAudioHistory(int channel) const
{
    if (channel == 0)
        return mainInputBufferHistory;
    else if (channel == 1)
        return sidechainBuffer0History;
    else if (channel == 2)
        return sidechainBuffer1History;

    return mainInputBufferHistory;
}