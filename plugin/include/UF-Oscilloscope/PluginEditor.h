#pragma once

#include "UF-Oscilloscope/PluginProcessor.h"

class PluginEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluginEditor(PluginProcessor &);
    ~PluginEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    void setXScale(float newXScale);
    void setYScale(float newYScale);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor &audioProcessor;

    float xScale = 1.0f;
    float yScale = 1.0f;

    juce::Slider bufferSlider;
    juce::Label bufferLabel;

    juce::Slider gainSlider;
    juce::Label gainLabel;

    juce::ToggleButton syncButton;
    juce::Label syncLabel;

    void setupSliders();

    juce::AudioBuffer<float> audioBuffer;
    void drawWaveform(juce::Graphics &g);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};