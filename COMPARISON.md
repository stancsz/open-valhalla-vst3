# Comparison: Open Valhalla vs. Valhalla Supermassive

This document outlines the key architectural and functional differences between **Open Valhalla** (this project) and the original **Valhalla Supermassive** by Valhalla DSP.

## 1. DSP Architecture

### Valhalla Supermassive
*   **Core**: Uses **Feedback Delay Networks (FDNs)**.
*   **Structure**: A matrix of delay lines where the output of each delay is fed back into the inputs of others via a mixing matrix (Householder, Hadamard, etc.).
*   **Mechanism**:
    *   **Modes**: Each mode (Gemini, Hydra, etc.) defines a specific delay network topology (number of delays, length ratios) and feedback matrix.
    *   **Warp**: Directly stretches or shrinks the *relative* lengths of the delay lines in the FDN. This creates the shifting resonances and transition from "echo" to "reverb".
    *   **Density**: Controls the mixing coefficients in the feedback matrix or adds allpass diffusers, effectively smearing the transients.

### Open Valhalla
*   **Core**: Uses a hybrid chain of **Delay + Chorus + Schroeder Reverb**.
*   **Structure**:
    *   `juce::dsp::DelayLine` (Pre-delay)
    *   `juce::dsp::Chorus` (Modulation)
    *   `juce::dsp::Reverb` (Standard Schroeder/Moorer reverb with comb/allpass filters)
    *   `juce::dsp::Filters` (EQ)
*   **Mechanism**:
    *   **Modes**: Approximated by tweaking standard reverb parameters (Room Size, Damping) and modulation settings (Chorus Rate/Depth).
    *   **Warp**: Currently maps to Chorus feedback or feedback routing, simulating some tonal shift but not the true delay-stretching behavior of an FDN.
    *   **Density**: Maps to Reverb Damping or Room Size limits. It does not alter the diffusion density of the algorithm itself (which is fixed in `juce::dsp::Reverb`).

## 2. Missing Features

### Functional
*   **True FDN Behavior**: The "blooming" effect where echo density increases over time (e.g., in "Great Annihilator") is hard to replicate perfectly without an FDN. Open Valhalla approximates this with high feedback and modulation.
*   **Clear Button**: Supermassive has a "CLEAR" button to instantly kill the reverb buffer. (Implemented in this update).
*   **Interactive Help**: Supermassive shows tooltips explaining each mode when hovered.
*   **Preset Management**: A built-in browser for presets.

### Sonic
*   **Warp Texture**: The "metallic" to "smooth" transition driven by Warp in Supermassive is distinctive. Open Valhalla's Warp (Chorus feedback) creates a different kind of texture (flanging/ringing).
*   **Stereo Image**: Supermassive's FDNs often produce wide, complex stereo fields due to cross-channel feedback. Open Valhalla relies on the stereo width implementation of the standard JUCE Reverb.

## 3. Summary
Open Valhalla is a **functional tribute** that captures the *spirit* and *workflow* of Supermassive using accessible standard DSP blocks. It is excellent for learning and creating lush spaces, but it lacks the specific FDN mathematical models that give Supermassive its exact signature sound.
