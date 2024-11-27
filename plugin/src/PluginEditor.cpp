#include "UF-Oscilloscope/PluginEditor.h"
#include "UF-Oscilloscope/PluginProcessor.h"

PluginEditor::PluginEditor(
    PluginProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), bufferSlider()
{
    setupSliders();

    loadLogo();

    setSize(550, 500);
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
    gainSlider.setBounds(1 * getWidth() / 8 - gainSliderWidth / 2 + 40, 385, gainSliderWidth, gainSliderHeight);

    auto bufferSliderWidth = 70;
    auto bufferSliderHeight = 100;
    bufferSlider.setBounds(7 * getWidth() / 8 - bufferSliderWidth / 2 - 40, 385, bufferSliderWidth, bufferSliderHeight);

    auto syncButtonWidth = 40;
    auto syncButtonHeight = 50;
    syncButton.setBounds(3 * getWidth() / 8 - syncButtonWidth / 4, 400, syncButtonWidth, syncButtonHeight);
    syncLabel.setBounds(syncButton.getX() - 5,
                        syncButton.getY() - 20, // Position it above the button
                        syncButtonWidth,        // Match the width of the button
                        20);

    auto inputComboBoxWidth = 50;
    auto inputComboBoxHeight = 50;
    inputComboBox.setBounds(5 * getWidth() / 8 - inputComboBoxWidth / 2 - 40, 400, inputComboBoxWidth, inputComboBoxHeight);
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

    // Check if histories are empty
    // if (mainHistory.empty() && sidechain0History.empty() && sidechain1History.empty())
    //     return;

    auto drawWaveformFromHistory = [&](const juce::AudioBuffer<float> &bufferHistory, juce::Colour color)
    {
        const int numSamples = bufferHistory.getNumSamples();
        const int numChannels = bufferHistory.getNumChannels();

        if (numSamples == 0 || numChannels == 0)
            return;

        const int pixels = 400;                            // Width of the drawing window
        const int step = std::max(1, numSamples / pixels); // Number of samples per pixel
        const int reducedSamples = numSamples / step;

        float prevX = left;                        // Initialize the previous x position
        float prevY = top + adjustedHeight / 2.0f; // Initialize the previous y position

        for (int i = 0; i < reducedSamples; ++i)
        {
            // Calculate start index for this step
            const int startIdx = i * step;

            // Select one representative sample (e.g., the first sample of the step)
            float sampleValue = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                sampleValue += bufferHistory.getSample(ch, startIdx); // Aggregate across channels
            }
            sampleValue /= numChannels; // Average across channels

            // Calculate current x and y positions
            float x = left + (i * adjustedWidth / reducedSamples) * xScale;
            float y = top + adjustedHeight / 2.0f + sampleValue * yScale * (adjustedHeight / 2.0f);

            // Clamp y values to stay within bounds
            y = juce::jlimit(top, bottom, y);

            if (i > 0) // Draw only if there's a previous point
            {
                g.setColour(color);
                g.drawLine(prevX, prevY, x, y, strokeSize);
            }

            // Update previous x and y for the next line segment
            prevX = x;
            prevY = y;
        }
    };

    // ! TEST V
    if (numOfInputs > 0)
    {
        const auto mainInputBufferHistory = audioProcessor.getHistoryBuffer(0);
        drawWaveformFromHistory(mainInputBufferHistory, juce::Colours::green);
    }
    if (numOfInputs > 1)
    {
        const auto sidechainBuffer1History = audioProcessor.getHistoryBuffer(1);
        drawWaveformFromHistory(sidechainBuffer1History, juce::Colours::red);
    }
    if (numOfInputs > 2)
    {
        const auto sidechainBuffer2History = audioProcessor.getHistoryBuffer(2);
        drawWaveformFromHistory(sidechainBuffer2History, juce::Colours::blue);
    }
    if (numOfInputs > 3)
    {
        const auto sidechainBuffer3History = audioProcessor.getHistoryBuffer(3);
        drawWaveformFromHistory(sidechainBuffer3History, juce::Colours::wheat);
    }
    if (numOfInputs > 4)
    {
        const auto sidechainBuffer4History = audioProcessor.getHistoryBuffer(4);
        drawWaveformFromHistory(sidechainBuffer4History, juce::Colours::yellow);
    }
    // ! TEST Λ
}

void PluginEditor::setXScale(int newXScale)
{
    audioProcessor.timeAxisChanged(newXScale);
}

void PluginEditor::setYScale(float newYScale)
{
    yScale = newYScale;
}

void PluginEditor::setupSliders()
{
    customLookAndFeel = std::make_unique<CustomLookAndFeel>();

    bufferSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::wheat);
    bufferSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    bufferSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    bufferSlider.setLookAndFeel(customLookAndFeel.get());
    bufferSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    bufferSlider.setRange(32, 75000, 1);
    bufferSlider.setValue(75000);
    bufferSlider.addListener(this);
    bufferSlider.setTextValueSuffix("");
    bufferSlider.onValueChange = [this]()
    {
        setXScale(static_cast<int>(bufferSlider.getValue()));
    };
    // bufferSlider.onDoubleClick = [this]() {

    // };
    addAndMakeVisible(bufferSlider);
    bufferLabel.setName("bufferLabel");
    bufferLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
    bufferLabel.setText("TIME", juce::NotificationType::dontSendNotification);
    bufferLabel.setJustificationType(juce::Justification::centred);
    bufferLabel.attachToComponent(&bufferSlider, false);
    addAndMakeVisible(bufferLabel);

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
    addAndMakeVisible(gainSlider);
    gainLabel.setName("gainLabel");
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
    gainLabel.setText("GAIN", juce::NotificationType::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.attachToComponent(&gainSlider, false);
    addAndMakeVisible(gainLabel);

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
    addAndMakeVisible(syncButton);
    syncLabel.setName("syncLabel");
    syncLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
    syncLabel.setText("Sync", juce::NotificationType::dontSendNotification);
    syncLabel.setJustificationType(juce::Justification::centred);
    syncLabel.attachToComponent(&syncButton, false);
    addAndMakeVisible(syncLabel);

    inputComboBox.addItem("0", 1);
    inputComboBox.addItem("1", 2);
    inputComboBox.addItem("2", 3);
    inputComboBox.addItem("3", 4);
    inputComboBox.addItem("4", 5);
    inputComboBox.setSelectedId(1);
    inputComboBox.onChange = [this]()
    { inputComboBoxChanged(); };
    addAndMakeVisible(inputComboBox);
    inputComboBoxLabel.setName("inputComboBoxLabel");
    inputComboBoxLabel.setColour(juce::Label::textColourId, juce::Colours::wheat);
    inputComboBoxLabel.setText("#Ins", juce::NotificationType::dontSendNotification);
    inputComboBoxLabel.setJustificationType(juce::Justification::centred);
    inputComboBoxLabel.attachToComponent(&inputComboBox, false);
    addAndMakeVisible(inputComboBoxLabel);
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
        setXScale((int)bufferSlider.getValue());
    }
    else if (slider == &gainSlider)
    {
        setYScale((float)gainSlider.getValue());
    }
}

void PluginEditor::inputComboBoxChanged()
{
    numOfInputs = inputComboBox.getSelectedId();
}