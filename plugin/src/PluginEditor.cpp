#include "UF-Oscilloscope/PluginEditor.h"
#include "UF-Oscilloscope/PluginProcessor.h"

PluginEditor::PluginEditor(
    PluginProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), bufferSlider()
{
    juce::ignoreUnused(audioProcessor);

    addAndMakeVisible(bufferSlider);
    bufferSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    bufferSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
    bufferSlider.setRange(32, 1024, 1);
    bufferSlider.onValueChange = [this]()
    {
        // audioProcessor.oscilloscope.setBufferSize((int)bufferSlider.getValue());
    };

    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
    gainSlider.setRange(-60, 12, 1);
    gainSlider.onValueChange = [this]()
    {
        // audioProcessor.setGain((float)gainSlider.getValue());
    };

    setSize(400, 400);
    startTimerHz(30);
}

PluginEditor::~PluginEditor() {}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::blueviolet);
    drawWaveform(g);
}

void PluginEditor::resized()
{
    bufferSlider.setBounds(200, 300, 100, 100);
    gainSlider.setBounds(100, 300, 100, 100);
}

void PluginEditor::timerCallback()
{
    audioBuffer = audioProcessor.getAudioBuffer();
    repaint();
}

void PluginEditor::drawWaveform(juce::Graphics &g)
{
    const int width = getWidth();
    const int height = getHeight();
    g.setColour(juce::Colours::white);
    const int numSamples = audioBuffer.getNumSamples();
    for (int i = 1; i < numSamples; ++i)
    {
        float y1 = height / 2.0f + audioBuffer.getSample(0, i - 1) * height / 2.0f;
        float y2 = height / 2.0f + audioBuffer.getSample(0, i) * height / 2.0f;

        g.drawLine((float)((i - 1) * width / numSamples), y1,
                   (float)(i * width / numSamples), y2);
    }
}