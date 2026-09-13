# HARBINGER OF DEATH — Pedal Description & Architecture
### *By THE EDGE OF FEAR*

---

## 1. Sonic Concept & Vision

**HARBINGER OF DEATH** was created for modern extreme music: down-tuned guitars (Drop A, Drop F, 8-string guitars), crushing bass, and synth textures that need aggressive frequency-cutting presence without turning into unintelligible mud.

Unlike traditional analog fuzzes that flub out when fed sub-bass notes or complex chords, Harbinger of Death uses a multi-stage hybrid DSP engine combining:
1. **Dynamic High-Pass & DC Filtering** to retain tight palm-muted attack transients.
2. **Frequency Follower Upper-Octave Synthesis** (+1, +2, or Dual Octaves) with dynamic envelope tracking and tracking inertia/glide.
3. **Microtonal Detune Voices** to provide hyper-wide stereo imaging.
4. **Hard-Square Digital Waveshaping** with adjustable odd/even harmonic asymmetry.
5. **Rhythmic Amplitude Chopper** with DAW tempo-sync divisions for machine-gun style gating and stutter breakdowns.

---

## 2. DSP Signal Flow Diagram

`
                      ┌──────────────────────┐
                      │      AUDIO INPUT     │
                      └──────────┬───────────┘
                                 │
                      ┌──────────▼───────────┐
                      │ 1. Input Conditioning│ (DC Blocker, High-Pass, Pre-Gain)
                      └──────────┬───────────┘
                                 │
             ┌───────────────────┴───────────────────┐
             │                                       │
    ┌────────▼──────────────┐             ┌──────────▼───────────┐
    │ 2. Dynamic Octave Gen │             │ 3. Multi-Voice Detune│
    │ (+1 / +2 / Dual + Lag)│             │ (Stereo Spread/Shift)│
    └────────┬──────────────┘             └──────────┬───────────┘
             │                                       │
             └───────────────────┬───────────────────┘
                                 │
                      ┌──────────▼───────────┐
                      │ 4. Square Waveshaper │ (Hard Clipping, Asymmetry, Saturation)
                      └──────────┬───────────┘
                                 │
                      ┌──────────▼───────────┐
                      │ 5. Dynamic Tone / EQ │ (Post-Gain Tilt, Mid-Notch, Hi-Cut)
                      └──────────┬───────────┘
                                 │
                      ┌──────────▼───────────┐
                      │ 6. Chop & Stutter    │ (LFO / Host-BPM Sync Gate)
                      └──────────┬───────────┘
                                 │
                      ┌──────────▼───────────┐
                      │ 7. Master Output &   │ (Brickwall Output Limiter)
                      │    Safety Limiter    │
                      └──────────┬───────────┘
                                 │
                      ┌──────────▼───────────┐
                      │     AUDIO OUTPUT     │
                      └──────────────────────┘
`

---

## 3. DSP Engine Breakdown

### Stage 1: Input Conditioning
- Blocks DC offset and low-frequency mud before non-linear stages to avoid intermodulation distortion on low strings.

### Stage 2: Dynamic Octave Engine
- Tracks fundamental pitch frequencies and synthesizes pristine upper-octave overtones (+1 octave up, +2 octaves up, or both blended).
- Features adjustable envelope lag for creating pitch dive-bombs, trailing glides, or instant laser attacks.

### Stage 3: Detune & Stereo Width Engine
- Generates microtonally pitch-shifted secondary voices (cents-level detuning) across the stereo spectrum.

### Stage 4: Square Waveshaping & Asymmetric Saturation
- Pushes the hybridized signal through hard-knee digital waveshaping with variable bias asymmetry.

### Stage 5: Post-Shaper Tone Shaping
- Active tilt EQ balancing aggressive high-mid presence against heavy low-end chunk, supplemented with high-cut filtering to eliminate unwanted aliasing sizzle.

### Stage 6: Chop / Stutter Modulator
- Fast amplitude modulation gate capable of running in free frequency mode (0.1 Hz – 50 Hz) or locking directly to the DAW host tempo grid.

### Stage 7: Master Safety Limiter
- Ultra-low latency peak limiter to ensure clean output without digital clipping overs.
