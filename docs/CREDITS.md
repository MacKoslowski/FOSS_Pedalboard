# Credits

Every algorithm in FOSS Pedalboard comes from an openly published source. This
file lists them. The same text appears in each pedal's "(i)" tooltip and in the
header comment of its source file.

## Frameworks and libraries

| Component | Author / project | License | Used for |
|---|---|---|---|
| JUCE 8.0.15 | Raw Material Software / JUCE | AGPLv3 (open-source option) | Plugin framework, `juce::dsp` building blocks, UI |
| VST3 SDK (via JUCE) | Steinberg Media Technologies GmbH | GPLv3 | VST3 plugin format. VST is a trademark of Steinberg Media Technologies GmbH. |
| Catch2 v3.8.1 | Catch2 contributors | Boost Software License 1.0 | Unit tests only, not shipped in the plugin |

## Pedals

### Delay (`src/pedals/DelayPedal.*`)

- **Algorithm:** feedback delay line with a one-pole low-pass "tone" filter in the feedback path, optionally tempo-synced.
- **Source:** `juce::dsp::DelayLine` (linear interpolation) and `juce::dsp::FirstOrderTPTFilter`, JUCE framework, AGPLv3.
- **Reference:** U. Zölzer (ed.), *DAFX: Digital Audio Effects*, 2nd ed., Wiley 2011, chapter 2 (delay-based effects). Textbook topology, original implementation.

### Reverb (`src/pedals/ReverbPedal.*`)

- **Algorithm:** Freeverb, a Schroeder/Moorer reverb with 8 parallel comb filters into 4 series all-pass filters per channel.
- **Original author:** "Jezar at Dreampoint", c. 2000, released into the public domain.
- **Source:** `juce::dsp::Reverb`, the JUCE framework's Freeverb implementation, AGPLv3.
- **Reference:** J. O. Smith, *Physical Audio Signal Processing*, "Freeverb" (https://ccrma.stanford.edu/~jos/pasp/Freeverb.html).

## Planned (not yet in the tree)

Listed so the licensing check happens before any code is written.

| Pedal | Intended source | License |
|---|---|---|
| Tuner | YIN pitch detection: A. de Cheveigné and H. Kawahara, "YIN, a fundamental frequency estimator for speech and music", JASA 111(4), 2002. Original implementation. | Published method |
| Noise Gate | `juce::dsp::NoiseGate` or textbook envelope follower (Zölzer, DAFX ch. 4) | AGPLv3 / published method |
| Plate reverb | J. Dattorro, "Effect Design, Part 1: Reverberator and Other Filters", JAES 45(9), 1997. Original implementation. | Published paper |
| Overdrive | Wave digital filter diode clipper, e.g. `chowdsp_wdf` | BSD-3-Clause |
