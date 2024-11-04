#include "UF-Oscilloscope/PluginEditor.h"
#include "UF-Oscilloscope/PluginProcessor.h"

PluginEditor::PluginEditor(
    PluginProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), bufferSlider()
{
    setupSliders();

    setSize(400, 500);
    startTimerHz(60);
}

PluginEditor::~PluginEditor() {}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::blueviolet);

    // Calculate the bounds for the rectangle
    float left = 20;
    float right = (float)getWidth() - 20;
    float top = 20;
    float bottom = (float)getHeight() - 150;
    juce::Path rectanglePath;
    rectanglePath.addRectangle(left, top, right - left, bottom - top);
    g.strokePath(rectanglePath, juce::PathStrokeType(5.0f));
    drawWaveform(g);
}

void PluginEditor::resized()
{
    auto bufferSliderWidth = 100;
    auto bufferSliderHeight = 100;
    bufferSlider.setBounds(getWidth() / 6 - bufferSliderWidth / 2, 385, bufferSliderWidth, bufferSliderHeight);

    auto gainSliderWidth = 100;
    auto gainSliderHeight = 100;
    gainSlider.setBounds(5 * getWidth() / 6 - gainSliderWidth / 2, 385, gainSliderWidth, gainSliderHeight);

    auto syncButtonWidth = 40;
    auto syncButtonHeight = 100;
    syncButton.setBounds(3 * getWidth() / 6 - syncButtonWidth / 4, 400, syncButtonWidth, syncButtonHeight);
}

void PluginEditor::timerCallback()
{
    audioBuffer = audioProcessor.getAudioBuffer();
    repaint();
}

// ******************************************

void PluginEditor::drawWaveform(juce::Graphics &g)
{
    const float width = static_cast<float>(getWidth() - 40);
    const float height = static_cast<float>(getHeight() - 170);
    g.setColour(juce::Colours::white);

    //

    const int numSamples = audioBuffer.getNumSamples();

    //

    if (numSamples == 0)
        return; // if there's no audio data

    for (int i = 1; i < numSamples; ++i)
    {
        float x1 = (float)((i - 1) * width / numSamples) * xScale + 20; // Adding offset to the right
        float x2 = (float)(i * width / numSamples) * xScale + 20;       // Adding offset to the right

        // Clamp x values to width
        if (x1 >= width + 20) // Adjusting clamp to account for offset
            x1 = width + 19;  // Adjusting clamp to account for offset
        if (x2 >= width + 20) // Adjusting clamp to account for offset
            x2 = width + 19;  // Adjusting clamp to account for offset

        float y1 = height / 2.0f + audioBuffer.getSample(0, i - 1) * yScale * (height / 2.0f) + 20; // Adding offset down
        float y2 = height / 2.0f + audioBuffer.getSample(0, i) * yScale * (height / 2.0f) + 20;     // Adding offset down

        // Clamp y values to height
        if (y1 < 20)          // Adjusting clamp to account for offset
            y1 = 20;          // Adjusting clamp to account for offset
        if (y1 > height + 20) // Adjusting clamp to account for offset
            y1 = height + 20;
        if (y2 < 20)          // Adjusting clamp to account for offset
            y2 = 20;          // Adjusting clamp to account for offset
        if (y2 > height + 20) // Adjusting clamp to account for offset
            y2 = height + 20;

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

void PluginEditor::setupSliders()
{
    addAndMakeVisible(bufferSlider);
    bufferSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    bufferSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    bufferSlider.setRange(0.1, 10, 1);
    bufferSlider.setValue(1.0);
    bufferSlider.setTextValueSuffix("ms");
    bufferSlider.onValueChange = [this]()
    {
        setXScale((float)bufferSlider.getValue());
    };
    addAndMakeVisible(bufferLabel);
    gainLabel.setName("bufferLabel");
    bufferLabel.setText("TIME", juce::NotificationType::dontSendNotification);
    bufferLabel.setJustificationType(juce::Justification::centred);
    bufferLabel.attachToComponent(&bufferSlider, false);

    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gainSlider.setRange(0.1, 6, 0.1);
    gainSlider.setValue(1.0);
    gainSlider.onValueChange = [this]()
    {
        setYScale((float)gainSlider.getValue());
    };
    addAndMakeVisible(gainLabel);
    gainLabel.setName("gainLabel");
    gainLabel.setText("GAIN", juce::NotificationType::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.attachToComponent(&gainSlider, false);

    addAndMakeVisible(syncButton);
    syncButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    syncButton.onClick = [this]()
    {
        stopTimer();
        if (syncButton.getToggleState())
        {
            const double bpm = (double)*audioProcessor.getBPM();
            if (bpm > 0)
            {
                double intervalHz = bpm / 60.0;
                startTimerHz(static_cast<int>(intervalHz));
                return;
            }
        }
        startTimerHz(60);
    };
    addAndMakeVisible(syncLabel);
    syncLabel.setName("syncLabel");
    syncLabel.setText("Sync", juce::NotificationType::dontSendNotification);
    syncLabel.setJustificationType(juce::Justification::centred);
    syncLabel.attachToComponent(&syncButton, false);
}