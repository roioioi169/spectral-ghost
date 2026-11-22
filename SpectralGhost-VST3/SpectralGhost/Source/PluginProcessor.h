#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Parameters.h"
#include "dsp/HPSSplitter.h"
#include "dsp/StreamEngines.h"
#include "dsp/Smoothed.h"

class SpectralGhostProcessor : public juce::AudioProcessor
{
public:
    SpectralGhostProcessor();
    ~SpectralGhostProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Spectral Ghost"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static constexpr int maxChannels = 2;

    HPSSplitter     splitters[maxChannels];
    StreamEngines   engines[maxChannels];

    Smoothed sSensitivity, sBias, sBite, sDrift, sCrossMod, sMix;

    std::atomic<float>* pSensitivity = nullptr;
    std::atomic<float>* pBias        = nullptr;
    std::atomic<float>* pSwap        = nullptr;
    std::atomic<float>* pBite        = nullptr;
    std::atomic<float>* pDrift       = nullptr;
    std::atomic<float>* pCrossMod    = nullptr;
    std::atomic<float>* pMix         = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralGhostProcessor)
};
