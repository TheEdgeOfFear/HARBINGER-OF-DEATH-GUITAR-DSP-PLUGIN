#pragma once

#include <string>
#include <vector>

namespace HarbingerPresets
{

struct Preset
{
    std::string category;
    std::string name;
    std::string description;
    float dissonance = 0.0f;
    float squareWave = 0.5f;
    float octaveLevel = 0.5f;
    int octaveMode = 0;       // 0: +1, 1: +2, 2: Both
    float speedHz = 8.0f;
    bool speedBpmSync = false;
    int syncDivision = 6;     // 1/16
    int trashMode = 0;        // 0: Latching, 1: Momentary, 2: Tap+Hold
    bool trashActive = true;
    float mix = 1.0f;
    float outputGain = 1.0f;
    bool isFactory = true;
};

inline std::vector<Preset> getBuiltInPresets()
{
    return {
        // === SLAUGHTER & DEATHCORE ===
        {
            "Slaughter & Deathcore",
            "Kostolom Stutter",
            "Crushing Jack Simmons rhythm fuzz with 1/16 tempo-synced hard stutter and dual screaming octaves.",
            0.15f, 0.92f, 0.65f, 2, 8.0f, true, 6, 0, true, 1.0f, 1.0f, true
        },
        {
            "Slaughter & Deathcore",
            "Demolisher Fuzz",
            "Saturated, abrasive square-wave wall with slight upper octave tracking to slice through ultra-dense 8-string mixes.",
            0.08f, 0.95f, 0.45f, 0, 6.0f, false, 6, 0, true, 1.0f, 1.05f, true
        },
        {
            "Slaughter & Deathcore",
            "1984 Breakdown",
            "Gated velcro sputter fuzz dialed for brutal palm-muted stops and instant violent transitions.",
            0.22f, 0.88f, 0.50f, 0, 10.0f, true, 6, 2, true, 1.0f, 0.98f, true
        },
        {
            "Slaughter & Deathcore",
            "Bonebreaker Lead",
            "+2 Octave screeching banshee wail with high dissonance detuning for disorienting chaotic solos.",
            0.55f, 0.78f, 0.85f, 1, 12.0f, false, 3, 0, true, 0.95f, 1.0f, true
        },
        {
            "Slaughter & Deathcore",
            "Viking Death Wail",
            "Sub-heavy attack with +1 and +2 octaves trailing slightly behind pick transients.",
            0.18f, 0.85f, 0.70f, 2, 4.0f, false, 6, 0, true, 1.0f, 1.0f, true
        },

        // === GATED GLITCH & STUTTER ===
        {
            "Gated Glitch & Stutter",
            "Machine Gun Chop",
            "High-speed 1/32 note hard laser gating with abrasive digital square clipping.",
            0.10f, 0.90f, 0.35f, 0, 16.0f, true, 9, 1, true, 1.0f, 1.0f, true
        },
        {
            "Gated Glitch & Stutter",
            "Cybernetic Tremolo",
            "Smooth tempo-locked 1/8 note rhythmic pulse with subtle dissonance modulation.",
            0.25f, 0.60f, 0.40f, 0, 5.0f, true, 3, 0, true, 1.0f, 1.0f, true
        },
        {
            "Gated Glitch & Stutter",
            "Tripwire Gate",
            "Momentary-oriented trash circuit designed to inject instantaneous glitch bursts during djent chugs.",
            0.30f, 0.95f, 0.60f, 2, 12.0f, true, 6, 1, false, 1.0f, 1.0f, true
        },
        {
            "Gated Glitch & Stutter",
            "Helicopter Rotor",
            "Heavy low-frequency stutter slicing across 7-string drop-tuned chordal riffs.",
            0.12f, 0.80f, 0.30f, 0, 4.5f, false, 3, 0, true, 1.0f, 0.95f, true
        },
        {
            "Gated Glitch & Stutter",
            "1/16 Triplet Slicer",
            "Polyrhythmic 1/16 Triplet chop locked to host BPM for mathcore and technical metal rhythms.",
            0.15f, 0.85f, 0.50f, 0, 8.0f, true, 7, 0, true, 1.0f, 1.0f, true
        },

        // === DISSONANT DOOM & SLUDGE ===
        {
            "Dissonant Doom & Sludge",
            "Ring of Agony",
            "Maximized dissonance detuning producing extreme intermodulation discordance and ring-mod artifacts.",
            0.92f, 0.75f, 0.55f, 0, 2.0f, false, 3, 0, true, 0.90f, 0.95f, true
        },
        {
            "Dissonant Doom & Sludge",
            "Subharmonic Void",
            "Murky, thick industrial fuzz with heavy detuned unisons and decayed crossover sputter.",
            0.65f, 0.90f, 0.20f, 0, 1.5f, false, 3, 0, true, 1.0f, 1.0f, true
        },
        {
            "Dissonant Doom & Sludge",
            "Discordant Drone",
            "Slow, brooding pitch-shifted decay that clashes against lingering open strings.",
            0.78f, 0.70f, 0.45f, 2, 0.8f, false, 3, 0, true, 0.85f, 1.0f, true
        },
        {
            "Dissonant Doom & Sludge",
            "Industrial Corrosion",
            "Crushed sample decimation and harsh square waves evoking Nine Inch Nails and Godflesh.",
            0.45f, 0.98f, 0.25f, 0, 3.2f, false, 6, 0, true, 1.0f, 1.02f, true
        },

        // === SCREAMING LEAD OCTAVES ===
        {
            "Screaming Lead Octaves",
            "Dual Octave Banshee",
            "The iconic Banshee lead sound: both +1 and +2 octaves blooming behind high lead phrases.",
            0.20f, 0.82f, 0.90f, 2, 7.0f, false, 6, 0, true, 1.0f, 1.0f, true
        },
        {
            "Screaming Lead Octaves",
            "Hellfire Solo +1",
            "Tight, biting +1 octave fuzz with high sustain and razor pick definition.",
            0.12f, 0.85f, 0.80f, 0, 6.0f, false, 6, 0, true, 1.0f, 1.0f, true
        },
        {
            "Screaming Lead Octaves",
            "Nerve Shredder +2",
            "Piercing +2 octave shrieks that cut straight over double-kick and blast beats.",
            0.30f, 0.88f, 0.92f, 1, 9.0f, false, 6, 0, true, 0.95f, 1.05f, true
        },
        {
            "Screaming Lead Octaves",
            "Ghostly Bloom",
            "Gentle square wave saturation with heavy octave lag swell trailing after every note.",
            0.15f, 0.40f, 0.85f, 2, 2.5f, false, 3, 0, true, 0.80f, 1.0f, true
        },

        // === MODERN DJENT & DROP-TUNING ===
        {
            "Modern Djent & Drop-Tuning",
            "Drop A Tight Trash",
            "Fast gating and tight 55Hz low-cut tuned specifically for low Drop A & Drop G riffs.",
            0.05f, 0.92f, 0.30f, 0, 10.0f, true, 6, 0, true, 1.0f, 1.0f, true
        },
        {
            "Modern Djent & Drop-Tuning",
            "Double Drop D Fuzz",
            "Ultra-low octave tracking with zero low-end mud, giving bass-like thickness to guitar stabs.",
            0.10f, 0.85f, 0.60f, 0, 6.0f, false, 6, 0, true, 1.0f, 0.98f, true
        },
        {
            "Modern Djent & Drop-Tuning",
            "8-String Clean Octave Blend",
            "50% Dry blend with +1 octave bloom and light square wave for modern progressive metal.",
            0.15f, 0.35f, 0.70f, 0, 5.0f, false, 6, 0, true, 0.55f, 1.0f, true
        }
    };
}

} // namespace HarbingerPresets
