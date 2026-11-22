#include "HPSSplitter.h"
#include <algorithm>
#include <cmath>

HPSSplitter::HPSSplitter()
{
    window.resize(fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[(size_t) i] = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * (float) i / (float) fftSize);

    // Hann^2 at 4x overlap sums to 1.5. The JUCE complex inverse is unnormalised,
    // so divide by fftSize as well.
    outScale = (1.0f / 1.5f) / (float) fftSize;

    inRing.assign(fftSize, 0.0f);
    pend.assign(hop, 0.0f);
    readyT.assign(hop, 0.0f);
    readyS.assign(hop, 0.0f);
    olaT.assign(fftSize, 0.0f);
    olaS.assign(fftSize, 0.0f);

    spec.assign(fftSize, Complex {});
    specT.assign(fftSize, Complex {});
    specS.assign(fftSize, Complex {});
    outT.assign(fftSize, Complex {});
    outS.assign(fftSize, Complex {});

    magHist.assign((size_t) timeMedianLen * bins, 0.0f);
    maskH.assign(bins, 0.0f);
    tmpT.assign(timeMedianLen, 0.0f);
    tmpF.assign(freqMedianLen, 0.0f);
}

void HPSSplitter::reset()
{
    std::fill(inRing.begin(), inRing.end(), 0.0f);
    std::fill(pend.begin(), pend.end(), 0.0f);
    std::fill(readyT.begin(), readyT.end(), 0.0f);
    std::fill(readyS.begin(), readyS.end(), 0.0f);
    std::fill(olaT.begin(), olaT.end(), 0.0f);
    std::fill(olaS.begin(), olaS.end(), 0.0f);
    std::fill(magHist.begin(), magHist.end(), 0.0f);
    histPos = 0;
    pendCount = 0;
    readIdx = 0;
}

void HPSSplitter::processSample(float in, float sensitivity, float bias,
                                float& transientOut, float& harmonicOut)
{
    transientOut = readyT[(size_t) readIdx];
    harmonicOut  = readyS[(size_t) readIdx];
    ++readIdx;

    pend[(size_t) pendCount++] = in;
    if (pendCount == hop)
    {
        // Slide the analysis window: drop the oldest hop, append the new one.
        std::copy(inRing.begin() + hop, inRing.end(), inRing.begin());
        std::copy(pend.begin(), pend.end(), inRing.begin() + (fftSize - hop));

        processFrame(sensitivity, bias);
        pendCount = 0;
        readIdx = 0;
    }
}

void HPSSplitter::processFrame(float sensitivity, float bias)
{
    // 1. Windowed input (staged in outS, which is overwritten below), then one forward FFT.
    for (int i = 0; i < fftSize; ++i)
        outS[(size_t) i] = Complex(inRing[(size_t) i] * window[(size_t) i], 0.0f);

    fft.perform(outS.data(), spec.data(), false);

    // 2. Magnitude history for the time median.
    float* mag = &magHist[(size_t) histPos * bins];
    for (int k = 0; k < bins; ++k)
        mag[k] = std::abs(spec[(size_t) k]);
    histPos = (histPos + 1) % timeMedianLen;

    // 3. Harmonic (time median) vs. percussive (frequency median) soft mask.
    for (int k = 0; k < bins; ++k)
    {
        for (int t = 0; t < timeMedianLen; ++t)
            tmpT[(size_t) t] = magHist[(size_t) t * bins + k];
        std::nth_element(tmpT.begin(), tmpT.begin() + timeMedianLen / 2, tmpT.end());
        const float H = tmpT[(size_t) (timeMedianLen / 2)];

        for (int j = 0; j < freqMedianLen; ++j)
        {
            const int idx = juce::jlimit(0, bins - 1, k - freqMedianLen / 2 + j);
            tmpF[(size_t) j] = mag[idx];
        }
        std::nth_element(tmpF.begin(), tmpF.begin() + freqMedianLen / 2, tmpF.end());
        const float P = tmpF[(size_t) (freqMedianLen / 2)];

        const float H2 = H * H, P2 = P * P;
        float m = H2 / (H2 + P2 + 1.0e-12f);                // harmonic soft mask
        m = (m - 0.5f) * sensitivity + 0.5f + 0.5f * bias;  // sensitivity + tonal/noise bias
        maskH[(size_t) k] = juce::jlimit(0.0f, 1.0f, m);
    }

    // 4. Split the spectrum and mirror the positive bins so the inverse is real.
    for (int k = 0; k < bins; ++k)
    {
        const float m = maskH[(size_t) k];
        const Complex X = spec[(size_t) k];
        specS[(size_t) k] = X * m;
        specT[(size_t) k] = X * (1.0f - m);

        if (k > 0 && k < fftSize / 2)
        {
            specS[(size_t) (fftSize - k)] = std::conj(specS[(size_t) k]);
            specT[(size_t) (fftSize - k)] = std::conj(specT[(size_t) k]);
        }
    }

    // 5. Inverse FFTs.
    fft.perform(specT.data(), outT.data(), true);
    fft.perform(specS.data(), outS.data(), true);

    // 6. Synthesis window and overlap-add.
    for (int i = 0; i < fftSize; ++i)
    {
        const float w = window[(size_t) i] * outScale;
        olaT[(size_t) i] += outT[(size_t) i].real() * w;
        olaS[(size_t) i] += outS[(size_t) i].real() * w;
    }

    // 7. Publish one hop and shift the accumulators.
    std::copy(olaT.begin(), olaT.begin() + hop, readyT.begin());
    std::copy(olaS.begin(), olaS.begin() + hop, readyS.begin());
    std::copy(olaT.begin() + hop, olaT.end(), olaT.begin());
    std::copy(olaS.begin() + hop, olaS.end(), olaS.begin());
    std::fill(olaT.end() - hop, olaT.end(), 0.0f);
    std::fill(olaS.end() - hop, olaS.end(), 0.0f);
}
