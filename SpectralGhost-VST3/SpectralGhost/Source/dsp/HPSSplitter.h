#pragma once
#include <juce_dsp/juce_dsp.h>
#include <complex>
#include <vector>

// Causal real-time harmonic/percussive split for ONE channel.
// All buffers are allocated in the constructor; processing never allocates.
// Latency is fftSize samples; report it to the host.
class HPSSplitter
{
public:
    static constexpr int fftOrder      = 10;               // N = 1024
    static constexpr int fftSize       = 1 << fftOrder;
    static constexpr int hop           = fftSize / 4;      // 4x overlap
    static constexpr int bins          = fftSize / 2 + 1;
    static constexpr int timeMedianLen = 9;
    static constexpr int freqMedianLen = 9;

    HPSSplitter();

    void reset();

    // One input sample in, one transient and one harmonic sample out.
    void processSample(float in, float sensitivity, float bias,
                       float& transientOut, float& harmonicOut);

    static constexpr int latency() { return fftSize; }

private:
    void processFrame(float sensitivity, float bias);

    using Complex = std::complex<float>;

    juce::dsp::FFT fft { fftOrder };

    std::vector<float>   window;
    std::vector<float>   inRing;     // last fftSize input samples
    std::vector<float>   pend;       // hop-sized input accumulator
    std::vector<float>   readyT, readyS;
    std::vector<float>   olaT, olaS;

    std::vector<Complex> spec;       // forward spectrum
    std::vector<Complex> specT, specS;
    std::vector<Complex> outT, outS;

    std::vector<float>   magHist;    // timeMedianLen x bins ring buffer
    std::vector<float>   maskH;      // per-bin harmonic mask
    std::vector<float>   tmpT, tmpF; // median scratch

    int histPos = 0;
    int pendCount = 0;
    int readIdx = 0;
    float outScale = 1.0f;
};
