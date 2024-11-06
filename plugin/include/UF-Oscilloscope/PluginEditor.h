#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UF-Oscilloscope/PluginProcessor.h"
#include "UF-Oscilloscope/CustomLookAndFeel.h"

class PluginEditor final : public juce::AudioProcessorEditor,
                           private juce::Slider::Listener,
                           private juce::Timer
{
public:
    explicit PluginEditor(PluginProcessor &);
    ~PluginEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;
    void timerCallback() override;

    void setXScale(float newXScale);
    void setYScale(float newYScale);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor &audioProcessor;

    float xScale = 1.0f;
    float yScale = 1.0f;

    std::unique_ptr<CustomLookAndFeel> customLookAndFeel;

    juce::Slider bufferSlider;
    juce::Label bufferLabel;

    juce::Slider gainSlider;
    juce::Label gainLabel;

    juce::ToggleButton syncButton;
    juce::Label syncLabel;

    void setupSliders();

    juce::AudioBuffer<float> audioBuffer;
    void drawWaveform(juce::Graphics &g);
    juce::Image oscillatorLogo;

    void mouseDoubleClick(const juce::MouseEvent &event) override;
    void sliderValueChanged(juce::Slider *slider) override;

    void loadLogo();
    float left;
    float right;
    float top;
    float bottom;
    float strokeSize = 2.5f; // Stroke width for the rectangle

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};