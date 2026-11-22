# Spectral Ghost (VST3)

Real-time transient vs. harmonic spectral split with Swap, Cross-Mod, Bite, Drift, and Mix.
Built with JUCE 8 and CMake. Pitch-Snap, Glide, Micro-Scatter, Blur/Freeze, and Stereo Bloom
are not implemented yet and are intentionally absent from the parameter list.

## Build

Requirements: CMake 3.22+, a C++17 compiler, Git. JUCE is fetched automatically.

macOS / Linux:

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release -j

Windows (Visual Studio 2022):

    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release

Output: build/SpectralGhost_artefacts/Release/VST3/Spectral Ghost.vst3

## Install

- macOS: copy to ~/Library/Audio/Plug-Ins/VST3/
- Windows: copy to C:\Program Files\Common Files\VST3\

Rescan plugins in your DAW.

## Layout

    Source/PluginProcessor.*        AudioProcessor, parameter wiring, per-sample smoothing
    Source/PluginEditor.*           Knob UI bound through APVTS
    Source/Parameters.h             Parameter IDs and layout
    Source/dsp/HPSSplitter.*        STFT harmonic/percussive split (latency = 1024)
    Source/dsp/StreamEngines.*      Swap, Bite, Drift (placeholder), Cross-Mod, Mix
    Source/dsp/Smoothed.h           Parameter ramps
    Source/dsp/SafetyUtils.h        NaN, denormal, and clip guards
    Source/ui/AeroLookAndFeel.*     Glass panels and rotary knob drawing

## Known limitations

- Drift is a delay-modulation placeholder, not pitch shifting.
- Mono-accurate split is run per channel with independent state.
- Latency reported as 1024 samples.
