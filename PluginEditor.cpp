#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "JuceHeader.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    imagefile0 = file.getChildFile("Contents")
                    .getChildFile("Resources")
                    .getChildFile("images")
                    .getChildFile("knob1.png");
    imagefile1 = file.getChildFile("Contents")
                    .getChildFile("Resources")
                    .getChildFile("images")
                    .getChildFile("knob2.png");
    imagefile2 = file.getChildFile("Contents")
                    .getChildFile("Resources")
                    .getChildFile("images")
                    .getChildFile("back.png");
    if (imagefile0.existsAsFile()){
        Image[0] = juce::ImageFileFormat::loadFrom(imagefile0);
    }
    if (imagefile1.existsAsFile()){
        Image[1] = juce::ImageFileFormat::loadFrom(imagefile1);
    }
    if (imagefile2.existsAsFile()){
        Image[2] = juce::ImageFileFormat::loadFrom(imagefile2);
    }
    std::vector<std::pair<juce::String, juce::Slider*>> slidermap{
        {"P0", &Slider[0]},
        {"P1", &Slider[1]},
        {"P2", &Slider[2]},
        {"P3", &Slider[3]},
        {"P4", &Slider[4]},
        {"P5", &Slider[5]}
    };
    for (const auto& [paramID, sliderPtr] : slidermap){
        addAndMakeVisible(sliderPtr);
        sliderPtr->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        sliderPtr->setAlpha(0.0);
        sliderPtr->setSliderStyle (juce::Slider::RotaryVerticalDrag);
        sliderPtr->onValueChange = [this](){repaint();};
        attach.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.STATE, paramID, *sliderPtr));
        sliders.push_back(sliderPtr);
    }
    setSize(400, 200);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
    auto autodraw = [&](const juce::Image& i,const juce::Slider& s, int x, int y, int w, int h){
        auto b = s.getBounds();
        g.drawImage(i, b.getX(), b.getY(), b.getWidth(), b.getHeight(), x, y, w, h, false);
    };
    auto getindex = [&](juce::Slider& s, int f){
        int num = juce::jlimit(0, f, (int)(s.valueToProportionOfLength(s.getValue())*f));
        return num;
    };
    if (Image[2].isValid()){
        int num = getindex(Slider[2], 9)*200;
        g.drawImage(Image[2], 0, 0, 400, 200, 0, num, 400, 200, false);
    }
    if (Image[0].isValid()){
        for (auto* s : sliders){
            if (s != nullptr){
            int num = getindex(*s, 100);
            int numY = ((num+4) % 6)*60;
            int numX = ((num+4) / 6)*60;
            autodraw(Image[0], *s, numX, numY, 60, 60);
            }
        }
    }
}

void AudioPluginAudioProcessorEditor::resized()
{

    Slider[2].setBounds(78, 24, 86, 86);
    Slider[0].setBounds(20, 112, 70, 70);
    Slider[1].setBounds(135, 112, 70, 70);
    Slider[4].setBounds(192, 22, 45, 45);
    Slider[5].setBounds(220, 54, 44, 44);
    Slider[3].setBounds(310, 60, 70, 70);

}
