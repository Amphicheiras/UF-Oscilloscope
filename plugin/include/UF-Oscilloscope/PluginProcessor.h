#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_utils/juce_audio_utils.h>

class PluginProcessor final : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    // ***********************************************************

    void processBufferHistory(juce::AudioBuffer<float> &historyBuffer, const juce::AudioBuffer<float> &buffer, int numChannels, int numSamples, int bufferID);
    const juce::AudioBuffer<float> &getHistoryBuffer(int channel) const;

    void setHistoryBufferSize(int size);

    void setBPM();
    juce::Optional<double> getBPM();

private:
    bool historyBufferFlag = false;
    int historyBufferSize = 75000;
    int numSidechainInputs = 5;

    std::vector<juce::AudioBuffer<float>> inputBuffers;
    std::vector<juce::AudioBuffer<float>> inputHistories;
    std::vector<int> historyBufferIndex;

    juce::Optional<double> bpm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};