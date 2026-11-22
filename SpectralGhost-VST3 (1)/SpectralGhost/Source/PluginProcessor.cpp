#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectralGhostProcessor::SpectralGhostProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input",   juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", makeLayout())
{
    pSensitivity = apvts.getRawParameterValue(ParamIDs::sensitivity);
    pBias        = apvts.getRawParameterValue(ParamIDs::bias);
    pSwap        = apvts.getRawParameterValue(ParamIDs::swap);
    pBite        = apvts.getRawParameterValue(ParamIDs::bite);
    pDrift       = apvts.getRawParameterValue(ParamIDs::drift);
    pCrossMod    = apvts.getRawParameterValue(ParamIDs::crossMod);
    pMix         = apvts.getRawParameterValue(ParamIDs::mix);
}

bool SpectralGhostProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SpectralGhostProcessor::prepareToPlay(double sampleRate, int)
{
    for (auto& s : splitters) s.reset();
    for (auto& e : engines)   e.prepare(sampleRate);

    sSensitivity.reset(sampleRate, *pSensitivity);
    sBias.reset(sampleRate, *pBias);
    sBite.reset(sampleRate, *pBite);
    sDrift.reset(sampleRate, *pDrift);
    sCrossMod.reset(sampleRate, *pCrossMod);
    sMix.reset(sampleRate, *pMix);

    setLatencySamples(HPSSplitter::latency());
}

void SpectralGhostProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = juce::jmin(buffer.getNumChannels(), maxChannels);
    const int numSamples  = buffer.getNumSamples();

    // Targets are set once per block; the smoothers ramp them per sample.
    sSensitivity.set(*pSensitivity);
    sBias.set(*pBias);
    sBite.set(*pBite);
    sDrift.set(*pDrift);
    sCrossMod.set(*pCrossMod);
    sMix.set(*pMix);

    const bool swap = *pSwap > 0.5f;

    float* channels[maxChannels] = { nullptr, nullptr };
    for (int ch = 0; ch < numChannels; ++ch)
        channels[ch] = buffer.getWritePointer(ch);

    for (int n = 0; n < numSamples; ++n)
    {
        const float sens  = sSensitivity.next();
        const float bias  = sBias.next();

        StreamEngines::Params p;
        p.swap     = swap;
        p.bite     = sBite.next();
        p.drift    = sDrift.next();
        p.crossMod = sCrossMod.next();
        p.mix      = sMix.next();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float t = 0.0f, h = 0.0f;
            splitters[ch].processSample(channels[ch][n], sens, bias, t, h);
            channels[ch][n] = engines[ch].process(t, h, p);
        }
    }

    // Channels beyond the stereo bus (if any) are cleared.
    for (int ch = numChannels; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, numSamples);
}

juce::AudioProcessorEditor* SpectralGhostProcessor::createEditor()
{
    return new SpectralGhostEditor(*this);
}

void SpectralGhostProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SpectralGhostProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectralGhostProcessor();
}
