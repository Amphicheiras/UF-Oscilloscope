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

    const std::deque<juce::AudioBuffer<float>> &getAudioHistory(int channel) const;
    void addToAudioHistory(int channel, const juce::AudioBuffer<float> &buffer);

    void setBPM();
    juce::Optional<double> getBPM();

private:
    // History settings
    const size_t maxHistorySize = 1; // Maximum number of buffers to keep in history
    juce::AudioBuffer<float> mainInputBuffer;
    juce::AudioBuffer<float> sidechainBuffer0;
    juce::AudioBuffer<float> sidechainBuffer1;
    std::deque<juce::AudioBuffer<float>> mainInputBufferHistory;
    std::deque<juce::AudioBuffer<float>> sidechainBuffer0History;
    std::deque<juce::AudioBuffer<float>> sidechainBuffer1History;

    juce::Optional<double> bpm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};