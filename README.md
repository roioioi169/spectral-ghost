# Spectral Ghost

Experimental transient-vs-sustain spectral split and sound-design processor.
Current stage: browser (Web Audio) prototype. JUCE VST3 port comes after the sound is locked.

## Run the prototype

    python3 -m http.server 8000

Open http://localhost:8000, click "Start audio", then load an audio file or use the mic.

## Layout

    index.html                      UI and Web Audio glue
    src/spectral-ghost-worklet.js   STFT split, Bite / Drift engines, cross-mod, mix

## Known limitations

- Drift is a delay-modulation approximation, not true pitch shifting.
- Pitch-Snapper, Micro-Scatter, Spectral Freeze, Stereo Bloom not implemented yet.
- Mono core. Output is duplicated to both channels.
- Latency is about 1024 samples (~21 ms at 48 kHz).
- HPSS is causal with a 9-frame median; tune KT / KF for quality vs. latency.

## Roadmap

1. Tune split quality on drums, vocals, synths.
2. Add Pitch-Snapper, Micro-Scatter, Freeze, Bloom.
3. Port DSP to JUCE (juce::dsp::FFT, setLatencySamples(1024), APVTS with matching param names).
