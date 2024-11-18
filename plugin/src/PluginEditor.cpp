#include "UF-Oscilloscope/PluginEditor.h"
#include "UF-Oscilloscope/PluginProcessor.h"

PluginEditor::PluginEditor(
    PluginProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), bufferSlider()
{
    setupSliders();

    loadLogo();

    setSize(400, 500);
    startTimerHz(60);
}

PluginEditor::~PluginEditor()
{
    gainSlider.setLookAndFeel(nullptr);
    bufferSlider.setLookAndFeel(nullptr);
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::blueviolet);

    g.drawImage(oscillatorLogo, getWidth() / 2 - oscillatorLogo.getWidth() / 2, 20, 500, 100, 0, 0, 500, 100);

    // Calculate the bounds for the rectangle
    float left = 20;
    float right = (float)getWidth() - 20;
    float top = 60;
    float bottom = (float)getHeight() - 150;
    juce::Path rectanglePath;
    rectanglePath.addRectangle(left, top, right - left, bottom - top);
    g.strokePath(rectanglePath, juce::PathStrokeType(5.0f));
    g.setColour(juce::Colours::wheat);
    drawWaveform(g);
}

void PluginEditor::resized()
{
    auto gainSliderWidth = 70;
    auto gainSliderHeight = 100;
    gainSlider.setBounds(1 * getWidth() / 6 - gainSliderWidth / 2 + 40, 385, gainSliderWidth, gainSliderHeight);

    auto bufferSliderWidth = 70;
    auto bufferSliderHeight = 100;
    bufferSlider.setBounds(5 * getWidth() / 6 - bufferSliderWidth / 2 - 40, 385, bufferSliderWidth, bufferSliderHeight);

    auto syncButtonWidth = 40;
    auto syncButtonHeight = 100;
    syncButton.setBounds(3 * getWidth() / 6 - syncButtonWidth / 4, 400, syncButtonWidth, syncButtonHeight);
}

void PluginEditor::timerCallback()
{
    repaint();
}

// ******************************************

void PluginEditor::drawWaveform(juce::Graphics &g)
{
    // Define your rectangle bounds and stroke size
    float left = 20 + strokeSize + 0.8f;                                      // Adjust left bound
    float right = static_cast<float>(getWidth()) - 20 - strokeSize - 0.8f;    // Adjust right bound
    float top = 60 + strokeSize + 0.8f;                                       // Adjust top bound
    float bottom = static_cast<float>(getHeight()) - 150 - strokeSize - 0.8f; // Adjust bottom bound
    const float adjustedWidth = right - left;                                 // Width of the drawing area
    const float adjustedHeight = bottom - top;                                // Height of the drawing area

    // Fetch histories from the processor
    const auto &mainHistory = audioProcessor.getAudioHistory(0);
    const auto &sidechain0History = audioProcessor.getAudioHistory(1);
    const auto &sidechain1History = audioProcessor.getAudioHistory(2);

    // Check if histories are empty
    // if (mainHistory.empty() && sidechain0History.empty() && sidechain1History.empty())
    //     return;

    // Helper function to draw a waveform from a history of audio buffers
    auto drawWaveformFromHistory = [&](const std::deque<juce::AudioBuffer<float>> &bufferHistory, juce::Colour color)
    {
        // Loop through each buffer in the history
        for (const auto &buffer : bufferHistory)
        {
            const int numSamples = buffer.getNumSamples();
            if (numSamples == 0)
                continue; // Skip empty buffers

            for (int i = 1; i < numSamples; ++i)
            {
                // Adjust x positions based on the zoom scale
                float x1 = left + ((i - 1) * adjustedWidth / numSamples) * xScale;
                float x2 = left + (i * adjustedWidth / numSamples) * xScale;

                // Ensure x1 and x2 are within bounds
                x1 = std::min(x1, right);
                x2 = std::min(x2, right);

                // Get mono sample from stereo input (average of left and right channels)
                float monoSample1 = 0.5f * (buffer.getSample(0, i - 1) + buffer.getSample(1, i - 1));
                float monoSample2 = 0.5f * (buffer.getSample(0, i) + buffer.getSample(1, i));

                // Calculate y positions, scaling to fit inside the rectangle's adjusted height
                float y1 = top + adjustedHeight / 2.0f + monoSample1 * yScale * (adjustedHeight / 2.0f);
                float y2 = top + adjustedHeight / 2.0f + monoSample2 * yScale * (adjustedHeight / 2.0f);

                // Clamp y values to the rectangle's adjusted height
                y1 = juce::jlimit(top, bottom, y1);
                y2 = juce::jlimit(top, bottom, y2);

                // Draw the waveform line
                g.setColour(color);
                g.drawLine(x1, y1, x2, y2, 2.5f);
            }
        }
    };

    drawWaveformFromHistory(mainHistory, juce::Colours::green);
    drawWaveformFromHistory(sidechain0History, juce::Colours::red);
    drawWaveformFromHistory(sidechain1History, juce::Colours::blue);
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
    customLookAndFeel = std::make_unique<CustomLookAndFeel>();
    addAndMakeVisible(bufferSlider);
    bufferSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::wheat);
    bufferSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    bufferSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    bufferSlider.setLookAndFeel(customLookAndFeel.get());
    bufferSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    bufferSlider.setRange(0.01, 10, 0.01);
    bufferSlider.setValue(1.0);
    bufferSlider.addListener(this);
    bufferSlider.setTextValueSuffix("");
    bufferSlider.onValueChange = [this]()
    {
        setXScale((float)bufferSlider.getValue());
    };
    // bufferSlider.onDoubleClick = [this]() {

    // };
    addAndMakeVisible(bufferLabel);
    bufferLabel.setName("bufferLabel");
    bufferLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
    bufferLabel.setText("TIME", juce::NotificationType::dontSendNotification);
    bufferLabel.setJustificationType(juce::Justification::centred);
    bufferLabel.attachToComponent(&bufferSlider, false);

    addAndMakeVisible(gainSlider);
    gainSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::wheat);
    gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainSlider.setLookAndFeel(customLookAndFeel.get());
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    gainSlider.setRange(0.01, 6, 0.01);
    gainSlider.setValue(1.0);
    gainSlider.onValueChange = [this]()
    {
        setYScale((float)gainSlider.getValue());
    };
    addAndMakeVisible(gainLabel);
    gainLabel.setName("gainLabel");
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
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
    syncLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
    syncLabel.setText("Sync", juce::NotificationType::dontSendNotification);
    syncLabel.setJustificationType(juce::Justification::centred);
    syncLabel.attachToComponent(&syncButton, false);
}

void PluginEditor::loadLogo()
{
    juce::File speakerImageFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("UF0/UF00/resources/images/oscillatorLogo.png");
    if (speakerImageFile.existsAsFile())
    {
        oscillatorLogo = juce::ImageFileFormat::loadFrom(speakerImageFile);
        DBG("UF-0scillator logo found @: " + speakerImageFile.getFullPathName());
        return;
    }
    DBG("UF-0scillator logo not found @: " + speakerImageFile.getFullPathName());
    jassertfalse;
    oscillatorLogo = juce::Image();
}

void PluginEditor::mouseDoubleClick(const juce::MouseEvent &event)
{
    if (event.eventComponent == &bufferSlider)
    {
        bufferSlider.setValue(-60.0f, juce::sendNotification);
    }
    else if (event.eventComponent == &gainSlider)
    {
        gainSlider.setValue(0.0f, juce::sendNotification);
    }
}

void PluginEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &bufferSlider)
    {
        setXScale((float)bufferSlider.getValue());
    }
    else if (slider == &gainSlider)
    {
        setYScale((float)gainSlider.getValue());
    }
}