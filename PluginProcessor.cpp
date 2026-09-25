#define JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE 1

#include "JuceHeader.h"

#include "PluginProcessor.h"

#include "PluginEditor.h"

#include <string.h>

#include <algorithm>
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::paral()
{
    juce::AudioProcessorValueTreeState::ParameterLayout ley;

    ley.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("P0", 1),
        "P0", 0.0f, 1.0f, 1.0f));
    
    ley.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("P1", 1),
        "P1", 0.0f, 1.0f, 0.3f));

    ley.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("P2", 1),
        "P2", 0.0f, 1.0f, 0.9f));

    ley.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("P3", 1),
        "P3", 0.0f, 1.0f, 0.7f));
        
    ley.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("P4", 1),
        "P4", 0.0f, 1.0f, 0.90f));

    ley.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("P5", 1),
        "P5", 0.1f, 0.5f, 0.1f));
    
    return ley;
}
//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       STATE (*this, nullptr, "NITRA", paral())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return "MoldRoom";
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto getP = [&](std::atomic<float>*& p,const juce::StringRef n){
        p = STATE.getRawParameterValue(n);
    };

    auto zero = [&](auto&& buf){
        std::fill(std::begin(buf), std::end(buf), 0.0f);
    };

    getP(P0s, "P0");
    getP(P1s, "P1");
    getP(P2s, "P2");
    getP(P3s, "P3");
    getP(P4s, "P4");
    getP(P5s, "P5");

    zero(filter);
    zero(filter2);
    zero(combL);
    zero(combR);
    zero(combL2);
    zero(combR2);
    zero(combL3);
    zero(combR3);
    zero(combL4);
    zero(combR4);
    zero(combL5);
    zero(combR5);
    zero(combL6);
    zero(combR6);
    zero(combL7);
    zero(combR7);
    zero(combL8);
    zero(combR8);

    zero(allpasL);
    zero(allpasR);
    zero(allpasL2);
    zero(allpasR2);
    zero(allpasL3);
    zero(allpasR3);
    zero(allpasL4);
    zero(allpasR4);
    zero(allpasL5);
    zero(allpasR5);
    zero(allpasL6);
    zero(allpasR6);

    combindx =0;
    apfindx =0;
   
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float sample = buffer.getNumSamples();

    auto* chL = buffer.getWritePointer (0);
    auto* chR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : chL;

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, sample);

    float wet = P1s->load();
    float dry = P0s->load();
    float RoomSize = P2s->load()*0.983f;
    float apfFB = P3s->load()*0.7f;
    float Hcut = 1.0f - P4s->load();
    float Lcut = 1.0f - (P5s->load()*0.09f);

    for (int i=0; i<sample; ++i){
        
        LFO1 += 0.0000052154f;
        if (LFO1>=1.0f)LFO1-=1.0f; 
        LFO2 += 0.00000839f;
        if (LFO2>=1.0f)LFO2-=1.0f;
        LFO3 += 0.0000115646f;
        if (LFO3>=1.0f)LFO3-=1.0f;
        LFO4 += 0.0000156463f;
        if (LFO4>=1.0f)LFO4-=1.0f;

        int lfo1 = ReverbLFO[((int)LFO1*35)];
        int lfo2 = ReverbLFO[((int)LFO2*35)];
        int lfo3 = ReverbLFO[((int)LFO3*35)];
        int lfo4 = ReverbLFO[((int)LFO4*35)];

        int comb1 = (combindx - (1031+lfo1)+4096)%4096;

        int comb2 = (combindx - (1123+lfo2)+4096)%4096;
            
        int comb3 = (combindx - (1217+lfo3)+4096)%4096;
            
        int comb4 = (combindx - (1321+lfo4)+4096)%4096;
        
        int comb5 = (combindx - (1429+lfo1)+4096)%4096;
          
        int comb6 = (combindx - (1531+lfo2)+4096)%4096;
            
        int comb7 = (combindx - (1621+lfo3)+4096)%4096;
            
        int comb8 = (combindx - (1721+lfo4)+4096)%4096;
           
        int comb1a = (combindx - (1789+lfo1)+4096)%4096;
           
        int comb2a = (combindx - (1901+lfo2)+4096)%4096;
            
        int comb3a = (combindx - (1999+lfo3)+4096)%4096;
            
        int comb4a = (combindx - (2113+lfo4)+4096)%4096;

        int comb5a = (combindx - (2221+lfo1)+4096)%4096;
           
        int comb6a = (combindx - (2333+lfo2)+4096)%4096;
          
        int comb7a = (combindx - (2467+lfo3)+4096)%4096;
            
        int comb8a = (combindx - (2593+lfo4)+4096)%4096;
           
        
        int alp1 = (apfindx - 233 + 1024)%1024;
            
        int alp2 = (apfindx - 293 + 1024)%1024;
            
        int alp3 = (apfindx - 383 + 1024)%1024;
           
        int alp4 = (apfindx - 419 + 1024)%1024;
            
        int alp5 = (apfindx - 547 + 1024)%1024;
           
        int alp6 = (apfindx - 701 + 1024)%1024;

        int alp1a = (apfindx - 131 + 1024)%1024;
           
        int alp2a = (apfindx - 239 + 1024)%1024;
           
        int alp3a = (apfindx - 389 + 1024)%1024;
            
        int alp4a = (apfindx - 479 + 1024)%1024;
        
        int alp5a = (apfindx - 563 + 1024)%1024;
            
        int alp6a = (apfindx - 809 + 1024)%1024;
       

        float ainp1 = chL[i];
        float ainp2 = chR[i];
        float siga1 = allpasL[alp1];
        float siga2 = allpasR[alp1a];
        float aout1 = (ainp1 *-apfFB)+siga1;
        float aout2 = (ainp2 *-apfFB)+siga2;
        allpasL[apfindx]=ainp1+(siga1*apfFB);
        allpasR[apfindx]=ainp2+(siga2*apfFB);

        float siga3 = allpasL2[alp2a];
        float siga4 = allpasR2[alp2];
        float aout3 = (aout1 *-apfFB)+siga3;
        float aout4 = (aout2 *-apfFB)+siga4;
        allpasL2[apfindx]=aout1+(siga3*apfFB);
        allpasR2[apfindx]=aout2+(siga4*apfFB);

        float siga5 = allpasL3[alp3];
        float siga6 = allpasR3[alp3a];
        float aout5 = (aout3 *-apfFB)+siga5;
        float aout6 = (aout4 *-apfFB)+siga6;
        allpasL3[apfindx]=aout3+(siga5*apfFB);
        allpasR3[apfindx]=aout4+(siga6*apfFB);

        float siga7 = allpasL4[alp4a];
        float siga8 = allpasR4[alp4];
        float aout7 = (aout5 *-apfFB)+siga7;
        float aout8 = (aout6 *-apfFB)+siga8;
        allpasL4[apfindx]=aout5+(siga7*apfFB);
        allpasR4[apfindx]=aout6+(siga8*apfFB);

        float siga9 = allpasL5[alp5];
        float siga10 = allpasR5[alp5a];
        float aout9 = (aout7 *-apfFB)+siga9;
        float aout10 = (aout8 *-apfFB)+siga10;
        allpasL5[apfindx]=aout7+(siga9*apfFB);
        allpasR5[apfindx]=aout8+(siga10*apfFB);

        float siga11 = allpasL6[alp6a];
        float siga12 = allpasR6[alp6];
        float aout11 = (aout9 *-apfFB)+siga11;
        float aout12 = (aout10 *-apfFB)+siga12;
        allpasL6[apfindx]=aout9+(siga11*apfFB);
        allpasR6[apfindx]=aout10+(siga12*apfFB);
        
        float fd1 = combL[comb1];
        float fd2 = combL2[comb3];
        float fd3 = combL3[comb5];
        float fd4 = combL4[comb7];
        float fd5 = combL5[comb1a];
        float fd6 = combL6[comb3a];
        float fd7 = combL7[comb5a];
        float fd8 = combL8[comb7a];
        float fd9 = combR[comb2];
        float fd10 = combR2[comb4];
        float fd11 = combR3[comb6];
        float fd12 = combR4[comb8];
        float fd13 = combR5[comb2a];
        float fd14 = combR6[comb4a];
        float fd15 = combR7[comb6a];
        float fd16 = combR8[comb8a];

        float cout1 = (1.0f-Hcut)*fd1+(Hcut*filter2[0]);
        float cout2 = (1.0f-Hcut)*fd9+(Hcut*filter2[1]);

        filter2[0]=cout1;
        filter2[1]=cout2;

        float cout3 = (1.0f-Hcut)*fd2+(Hcut*filter2[2]);
        float cout4 = (1.0f-Hcut)*fd10+(Hcut*filter2[3]);

        filter2[2]=cout3;
        filter2[3]=cout4;

        float cout5 = (1.0f-Hcut)*fd3+(Hcut*filter2[4]);
        float cout6 = (1.0f-Hcut)*fd11+(Hcut*filter2[5]);

        filter2[4]=cout5;
        filter2[5]=cout6;

        float cout7 = (1.0f-Hcut)*fd4+(Hcut*filter2[6]);
        float cout8 = (1.0f-Hcut)*fd12+(Hcut*filter2[7]);

        filter2[6]=cout7;
        filter2[7]=cout8;

        float cout9 = (1.0f-Hcut)*fd5+(Hcut*filter2[8]);
        float cout10 = (1.0f-Hcut)*fd13+(Hcut*filter2[9]);

        filter2[8]=cout9;
        filter2[9]=cout10;

        float cout11 = (1.0f-Hcut)*fd6+(Hcut*filter2[10]);
        float cout12 = (1.0f-Hcut)*fd14+(Hcut*filter2[11]);

        filter2[10]=cout11;
        filter2[11]=cout12;

        float cout13 = (1.0f-Hcut)*fd7+(Hcut*filter2[12]);
        float cout14 = (1.0f-Hcut)*fd15+(Hcut*filter2[13]);

        filter2[12]=cout13;
        filter2[13]=cout14;

        float cout15 = (1.0f-Hcut)*fd8+(Hcut*filter2[14]);
        float cout16 = (1.0f-Hcut)*fd16+(Hcut*filter2[15]);

        filter2[14]=cout15;
        filter2[15]=cout16;

        
        float fcout1 = Lcut*(filter[0]+cout1-filter[1]);
        float fcout2 = Lcut*(filter[2]+cout2-filter[3]);

        filter[0]=fcout1;
        filter[1]=cout1;
        filter[2]=fcout2;
        filter[3]=cout2;

        float fcout3 = Lcut*(filter[4]+cout3-filter[5]);
        float fcout4 = Lcut*(filter[6]+cout4-filter[7]);

        filter[4]=fcout3;
        filter[5]=cout3;
        filter[6]=fcout4;
        filter[7]=cout4;

        float fcout5 = Lcut*(filter[8]+cout5-filter[9]);
        float fcout6 = Lcut*(filter[10]+cout6-filter[11]);

        filter[8]=fcout5;
        filter[9]=cout5;
        filter[10]=fcout6;
        filter[11]=cout6;

        float fcout7 = Lcut*(filter[12]+cout7-filter[13]);
        float fcout8 = Lcut*(filter[14]+cout8-filter[15]);

        filter[12]=fcout7;
        filter[13]=cout7;
        filter[14]=fcout8;
        filter[15]=cout8;

        float fcout9 = Lcut*(filter[16]+cout9-filter[17]);
        float fcout10 = Lcut*(filter[18]+cout10-filter[19]);

        filter[16]=fcout9;
        filter[17]=cout9;
        filter[18]=fcout10;
        filter[19]=cout10;

        float fcout11 = Lcut*(filter[20]+cout11-filter[21]);
        float fcout12 = Lcut*(filter[22]+cout12-filter[23]);

        filter[20]=fcout11;
        filter[21]=cout11;
        filter[22]=fcout12;
        filter[23]=cout12;

        float fcout13 = Lcut*(filter[24]+cout13-filter[25]);
        float fcout14 = Lcut*(filter[26]+cout14-filter[27]);

        filter[24]=fcout13;
        filter[25]=cout13;
        filter[26]=fcout14;
        filter[27]=cout14;

        float fcout15 = Lcut*(filter[28]+cout15-filter[29]);
        float fcout16 = Lcut*(filter[30]+cout16-filter[31]);

        filter[28]=fcout15;
        filter[29]=cout15;
        filter[30]=fcout16;
        filter[31]=cout16;

        float Hadamard[16];

        Hadamard[0]=fcout1;
        Hadamard[1]=fcout2;
        Hadamard[2]=fcout3;
        Hadamard[3]=fcout4;
        Hadamard[4]=fcout5;
        Hadamard[5]=fcout6;
        Hadamard[6]=fcout7;
        Hadamard[7]=fcout8;
        Hadamard[8]=fcout9;
        Hadamard[9]=fcout10;
        Hadamard[10]=fcout11;
        Hadamard[11]=fcout12;
        Hadamard[12]=fcout13;
        Hadamard[13]=fcout14;
        Hadamard[14]=fcout15;
        Hadamard[15]=fcout16;

        Hadamard16(Hadamard);

        combL[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[0]*RoomSize;
        combR[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[1]*RoomSize;
        combL2[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[2]*RoomSize;
        combR2[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[3]*RoomSize;
        combL3[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[4]*RoomSize;
        combR3[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[5]*RoomSize;
        combL4[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[6]*RoomSize;
        combR4[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[7]*RoomSize;
        combL5[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[8]*RoomSize;
        combR5[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[9]*RoomSize;
        combL6[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[10]*RoomSize;
        combR6[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[11]*RoomSize;
        combL7[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[12]*RoomSize;
        combR7[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[13]*RoomSize;
        combL8[combindx] = aout11 * (1.0f-(RoomSize*0.4f)) + Hadamard[14]*RoomSize;
        combR8[combindx] = aout12 * (1.0f-(RoomSize*0.4f)) + Hadamard[15]*RoomSize;

        float LoutFDN = (Hadamard[0]+Hadamard[2]+Hadamard[4]+Hadamard[6]+Hadamard[8]+Hadamard[11]+Hadamard[13]+Hadamard[15])*0.25f;
        float RoutFDN = (Hadamard[1]+Hadamard[3]+Hadamard[5]+Hadamard[7]+Hadamard[9]+Hadamard[10]+Hadamard[12]+Hadamard[14])*0.25f;

        chL[i] = (chL[i]*dry)+ (LoutFDN*wet);
        chR[i] = (chR[i]*dry) +(RoutFDN*wet);

        combindx = (combindx + 1) % 4096;
        
        apfindx = (apfindx + 1) % 1024;

    }

}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

inline void AudioPluginAudioProcessor::Hadamard16 (float* Hamadard)
{
    for (int i = 0; i < 16; i += 2) {
        float a = Hamadard[i], b = Hamadard[i+1];
        Hamadard[i] = a + b; Hamadard[i+1] = a - b;
    }
    for (int i = 0; i < 16; i += 4) {
        for (int j = 0; j < 2; ++j) {
            float a = Hamadard[i+j], b = Hamadard[i+j+2];
            Hamadard[i+j] = a + b; Hamadard[i+j+2] = a - b;
        }
    }
    for (int i = 0; i < 16; i += 8) {
        for (int j = 0; j < 4; ++j) {
            float a = Hamadard[i+j], b = Hamadard[i+j+4];
            Hamadard[i+j] = a + b; Hamadard[i+j+4] = a - b;
        }
    }
    for (int j = 0; j < 8; ++j) {
        float a = Hamadard[j], b = Hamadard[j+8];
        Hamadard[j] = a + b; Hamadard[j+8] = a - b;
    }
    const float Nor = 1.0f / 4.0f; 
    for (int i = 0; i < 16; ++i) {
        Hamadard[i] *= Nor;
    }
}
