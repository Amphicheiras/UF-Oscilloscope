#include "UF-Oscilloscope/PluginProcessor.h"
#include "UF-Oscilloscope/PluginEditor.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("MainInput", juce::AudioChannelSet::stereo())
              .withInput("SidechainInput1", juce::AudioChannelSet::stereo(), true)
              .withInput("SidechainInput2", juce::AudioChannelSet::stereo(), true)
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
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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
    auto mainInput = getBusBuffer(buffer, true, 0);
    auto sidechainInput1 = getTotalNumInputChannels() > 1 ? getBusBuffer(buffer, true, 1) : juce::AudioBuffer<float>(); // Auxiliary input (Channel 1, if available)
    auto sidechainInput2 = getTotalNumInputChannels() > 2 ? getBusBuffer(buffer, true, 2) : juce::AudioBuffer<float>(); // Auxiliary input (Channel 1, if available)
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

    mainInputBuffer.setSize(mainInput.getNumChannels(), numSamples);
    mainInputBuffer.copyFrom(0, 0, mainInput, 0, 0, numSamples);
    mainInputBuffer.copyFrom(1, 0, mainInput, 1, 0, numSamples);
    output.copyFrom(0, 0, mainInput, 0, 0, numSamples);
    output.copyFrom(1, 0, mainInput, 1, 0, numSamples);

    if (sidechainInput1.getNumChannels() > 0) // If sidechain input exists
    {
        sidechainBuffer0.setSize(sidechainInput1.getNumChannels(), numSamples); // Allocate space for sidechain input
        sidechainBuffer0.copyFrom(0, 0, sidechainInput1, 0, 0, numSamples);     // Copy sidechain input data
        sidechainBuffer0.copyFrom(1, 0, sidechainInput1, 1, 0, numSamples);     // Copy sidechain input data
        output.addFrom(0, 0, sidechainInput1, 0, 0, numSamples);                // Optionally mix sidechain input 1 into the output
        output.addFrom(1, 0, sidechainInput1, 1, 0, numSamples);                // Optionally mix sidechain input 1 into the output
    }

    if (sidechainInput2.getNumChannels() > 0) // If sidechain input exists
    {
        sidechainBuffer1.setSize(sidechainInput2.getNumChannels(), numSamples); // Allocate space for sidechain input
        sidechainBuffer1.copyFrom(0, 0, sidechainInput2, 0, 0, numSamples);     // Copy sidechain input data
        sidechainBuffer1.copyFrom(1, 0, sidechainInput2, 1, 0, numSamples);     // Copy sidechain input data
        output.addFrom(0, 0, sidechainInput2, 0, 0, numSamples);                // Optionally mix sidechain input 2 into the output
        output.addFrom(1, 0, sidechainInput2, 1, 0, numSamples);                // Optionally mix sidechain input 2 into the output
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