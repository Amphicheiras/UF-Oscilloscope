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

    const juce::AudioBuffer<float> &getAudioBuffer(int channel) const
    {
        if (channel == 0)
        {
            return mainInputBuffer;
        }
        else if (channel == 1)
        {
            return sidechainBuffer0;
        }
        else if (channel == 2)
        {
            return sidechainBuffer1;
        }
        return mainInputBuffer;
    }

    void setBPM();
    juce::Optional<double> getBPM();

private:
    juce::AudioBuffer<float> mainInputBuffer;
    juce::AudioBuffer<float> sidechainBuffer0;
    juce::AudioBuffer<float> sidechainBuffer1;

    juce::Optional<double> bpm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};