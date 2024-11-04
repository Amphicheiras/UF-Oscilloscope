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
    bufferSlider.setRange(0.1, 10, 1);
    bufferSlider.setValue(1.0);
    bufferSlider.setTextValueSuffix("ms");
    bufferSlider.onValueChange = [this]()
    {
        setXScale((float)bufferSlider.getValue());
    };
    addAndMakeVisible(bufferLabel);
    gainLabel.setName("bufferLabel");
    bufferLabel.setText("Time", juce::NotificationType::dontSendNotification);
    bufferLabel.setJustificationType(juce::Justification::centred);
    bufferLabel.attachToComponent(&bufferSlider, false);

    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
    gainSlider.setRange(0.1, 6, 0.1);
    gainSlider.setValue(1.0);
    gainSlider.onValueChange = [this]()
    {
        setYScale((float)gainSlider.getValue());
    };
    addAndMakeVisible(gainLabel);
    gainLabel.setName("gainLabel");
    gainLabel.setText("Gain", juce::NotificationType::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.attachToComponent(&gainSlider, false);

    setSize(400, 400);
    startTimerHz(60);
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
    const float width = static_cast<float>(getWidth());
    const float height = static_cast<float>(getHeight());
    g.setColour(juce::Colours::white);

    const int numSamples = audioBuffer.getNumSamples();
    if (numSamples == 0)
        return; // Early return if there's no audio data

    // Iterate through samples and draw lines scaled by xScale and yScale
    for (int i = 1; i < numSamples; ++i)
    {
        float x1 = (float)((i - 1) * width / numSamples) * xScale;
        float x2 = (float)(i * width / numSamples) * xScale;

        // Clamp x values to width
        if (x1 >= width)
            x1 = width - 1;
        if (x2 >= width)
            x2 = width - 1;

        float y1 = height / 2.0f + audioBuffer.getSample(0, i - 1) * yScale * (height / 2.0f);
        float y2 = height / 2.0f + audioBuffer.getSample(0, i) * yScale * (height / 2.0f);

        // Clamp y values to height
        if (y1 < 0)
            y1 = 0;
        if (y1 > height)
            y1 = height;
        if (y2 < 0)
            y2 = 0;
        if (y2 > height)
            y2 = height;

        g.drawLine(x1, y1, x2, y2);
    }
}

void PluginEditor::setXScale(float newXScale)
{
    xScale = newXScale;
}

void PluginEditor::setYScale(float newYScale)
{
    yScale = newYScale;
}