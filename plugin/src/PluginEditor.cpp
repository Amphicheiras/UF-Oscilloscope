#include "UF-Oscilloscope/PluginEditor.h"
#include "UF-Oscilloscope/PluginProcessor.h"

PluginEditor::PluginEditor(
    PluginProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), bufferSlider()
{
    juce::ignoreUnused(audioProcessor);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(audioProcessor.oscilloscope);
    audioProcessor.oscilloscope.setColours(juce::Colours::black, juce::Colours::blueviolet);

    addAndMakeVisible(bufferSlider);
    bufferSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    bufferSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
    bufferSlider.setRange(32, 1024, 1);
    bufferSlider.onValueChange = [this]()
    {
        audioProcessor.oscilloscope.setBufferSize((int)bufferSlider.getValue());
    };

    setSize(400, 400);
}

PluginEditor::~PluginEditor() {}

void PluginEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a
    // solid colour)
    g.fillAll(juce::Colours::black.brighter(0.1f));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Your plugin says hi!", getLocalBounds(),
                     juce::Justification::centred, 1);
}

void PluginEditor::resized()
{
    audioProcessor.oscilloscope.setBounds(getLocalBounds().withSizeKeepingCentre(static_cast<int>((float)getWidth() * 0.5), static_cast<int>((float)getHeight() * 0.5)));
    bufferSlider.setBounds(audioProcessor.oscilloscope.getX() + audioProcessor.oscilloscope.getWidth(), audioProcessor.oscilloscope.getY(), 128, audioProcessor.oscilloscope.getHeight());
}