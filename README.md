# FOSS Pedalboard

A free and open source guitar pedalboard plugin built with [JUCE](https://juce.com).
Drop up to six effect pedals onto a board, set their order, stomp them on and off,
and twist their knobs. Every effect is built from an openly published algorithm or
circuit model, and every pedal tells you exactly where its sound came from.

Licensed under the **GNU Affero General Public License v3.0** (AGPLv3).

> **Status: early development.** There is nothing to download yet. The first
> milestone is a Windows VST3 with a Delay and a Reverb pedal.

## What it looks like

*(screenshot coming once there is something to screenshot)*

The interface is deliberately hand-drawn and a little scrappy, in the spirit of
the IOF Drummer plugin: a scribbled title, pen-drawn knobs and footswitches,
handwritten uppercase labels, and a small "UI SCALING" box in the corner. It is
meant to feel like a sketch of a pedalboard on the back of a setlist, not a
photo-real rack of boutique gear.

## Features (planned for v1)

- **Six-slot pedal chain.** Signal flows left to right through slots 1 to 6.
- **One of each.** Each pedal type can appear once on the board. This keeps the
  parameter list the DAW sees fixed and predictable, and keeps CPU use sane.
- **Board controls per slot.** Swap with the pedal to the left or right, replace
  the pedal with a different one, or remove it. An empty slot has an "add pedal"
  button. Drag-and-drop may come later.
- **Footswitch bypass** on every pedal, with click-free switching.
- **"(i)" on every pedal.** Hover the little info icon to see which algorithm the
  pedal uses, who published it, and under what license.
- **Per-pedal presets.** Save and recall a single pedal's knob positions by name.
- **Global input and output gain**, and a **UI scaling** control.

## Pedals

| Pedal | Status | Algorithm basis | Source license |
|---|---|---|---|
| Delay | Planned for v0.1 | Digital delay line with feedback, a one-pole low-pass "tone" filter in the feedback path, and tempo sync. Built on `juce::dsp::DelayLine` with interpolation. | JUCE modules (AGPLv3 / ISC) |
| Reverb | Planned for v0.1 | Freeverb, the public-domain Schroeder/Moorer reverb by Jezar at Dreampoint, as implemented in `juce::dsp::Reverb`. A Dattorro plate reverb (J. Dattorro, "Effect Design Part 1", JAES 1997) is planned as a second algorithm. | Public domain / JUCE modules |
| Tuner | Planned | A pedal that mutes or passes audio and shows pitch on a hand-drawn needle. Detection by autocorrelation / YIN (de Cheveigné & Kawahara, 2002). | Published method, original implementation |
| Noise Gate | Planned | Threshold, attack, release and hold with an envelope follower, after `juce::dsp::NoiseGate` or the textbook design in Zölzer's *DAFX*. | JUCE modules / published method |
| Overdrive / Distortion | Idea | Wave digital filter or published diode-clipper circuit models (for example via `chowdsp_wdf`). | BSD-3 / published papers |
| Chorus / Flanger / Phaser | Idea | `juce::dsp::Chorus` and `juce::dsp::Phaser`, or classic LFO-modulated delay designs. | JUCE modules |
| Tremolo, Compressor, EQ | Idea | Textbook designs. | Published methods |
| Cabinet simulator | Idea | Impulse response loader using `juce::dsp::Convolution`, shipping with openly licensed IRs only. | JUCE modules / CC-licensed IRs |

### Where the algorithms come from

Every pedal in this project is built from sources that are free to use, study and
redistribute:

- Public-domain algorithms, academic papers and textbooks, and code under
  permissive (MIT, BSD, ISC, zlib) or GPL-compatible licenses.
- **Never** code reverse-engineered or decompiled from proprietary plugins, and
  never a "port" of a commercial product's DSP.
- No trademarked pedal names in the UI. A pedal inspired by a classic circuit gets
  a descriptive nickname of its own, not the original brand's name.
- Every DSP source file cites its origin in a header comment, and the same
  citation appears in the pedal's "(i)" tooltip and in `docs/CREDITS.md`.

## Building from source

You will need Git, CMake 3.22 or newer, and a C++20 compiler:

- **Windows:** Visual Studio 2022 with the "Desktop development with C++" workload.
- **macOS:** Xcode 14 or newer.
- **Linux:** GCC 11+ or Clang 14+, plus the usual JUCE dependencies
  (`libasound2-dev`, `libfreetype6-dev`, `libx11-dev`, `libxrandr-dev`,
  `libxinerama-dev`, `libxcursor-dev`, `libcurl4-openssl-dev`, `libgl1-mesa-dev`).

JUCE is pulled in as a git submodule, so clone with submodules:

```sh
git clone --recurse-submodules https://github.com/MacKoslowski/FOSS_Pedalboard.git
cd FOSS_Pedalboard
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release
```

If you already cloned without submodules, run `git submodule update --init --recursive`.
On Windows, pass `-G "Visual Studio 17 2022" -A x64` to the first `cmake` call.
The first configure builds JUCE's helper tool and downloads Catch2 for the tests,
so it takes a minute or two.

The VST3 bundle ends up under `build/FOSSPedalboard_artefacts/Release/VST3/`.
Copy `FOSS Pedalboard.vst3` into your system VST3 folder
(`C:\Program Files\Common Files\VST3` on Windows, `~/Library/Audio/Plug-Ins/VST3`
on macOS, `~/.vst3` on Linux) and rescan plugins in your DAW. A Standalone build is
also produced under `build/FOSSPedalboard_artefacts/Release/Standalone/` for quick
testing without a DAW.

## Architecture in one picture

The code is object-oriented so that the signal path reads like the board itself,
and so that adding a pedal means adding one class and one registry line.

```
PluginProcessor
  └── PedalChain                      six fixed slots, processed in order
        ├── Slot 1  → DelayPedal      : Pedal
        ├── Slot 2  → ReverbPedal     : Pedal
        ├── Slot 3  → (empty)
        └── ...
PedalRegistry                          type id → factory, name, algorithm credit

PluginEditor
  └── BoardComponent                  hand-drawn board background
        └── SlotComponent × 6         swap / replace / remove buttons
              └── PedalComponent      knobs, footswitch, "(i)" icon
                    (drawn by HandDrawnLookAndFeel from supplied artwork)
```

- `Pedal` is an abstract base class: declare parameters, prepare, process, reset,
  build an editor, and report an algorithm credit.
- `PedalChain` owns the six slots, applies click-free bypass, and lets the UI swap,
  replace and remove pedals without disturbing the audio thread.
- `PedalRegistry` is the single list of available pedal types.

See [CLAUDE.md](CLAUDE.md) for the full set of conventions and the
"adding a pedal" checklist.

## Roadmap

- **v0.1** Six-slot chain, Delay, Reverb, bypass, slot controls, "(i)" credits,
  Windows VST3 build.
- **v0.2** Per-pedal presets, Tuner, Noise Gate, first drive pedal, GitHub Actions
  builds for Windows, macOS and Linux, pluginval in CI.
- **Later** CLAP, AU and LV2 formats, MIDI control, drag-and-drop reordering,
  more pedals.

## Quality-of-life ideas under consideration

These are ideas, not promises. Open an issue if one of them matters to you.

- **Per-pedal presets** (planned): save a pedal's knob settings by name as a small
  XML file in your user folder, with a few factory presets per pedal.
- **Whole-board presets** (later, maybe): your DAW already saves the complete board
  inside the project, so board presets only matter for carrying a whole rig
  between projects or into the standalone app. If added, the minimum would be
  "Save board as..." / "Load board" and a handful of factory boards such as
  "Clean + Ambience", "Crunch Rhythm" and "Lead".
- **Drag-and-drop reordering** as an upgrade to the swap buttons, and keyboard
  shortcuts for bypass.
- **Simple meters** next to the global input and output gain.
- **Per-pedal wet/dry mix and output level** so every pedal can sit in parallel.
- **Tempo sync and tap tempo** for delay and modulation pedals.
- **Oversampling toggle** on drive pedals to trade CPU for less aliasing.
- **MIDI CC learn** so a foot controller can stomp pedals during a live set.
- **Snapshots / scenes**: store the on/off state of every pedal and switch
  between them instantly.
- **Undo / redo** for board edits.
- **HiDPI-safe artwork** behind the UI scaling control.
- **Latency reporting** to the host and clean behaviour on sample-rate changes.
- **Cabinet simulator / IR loader** in the last slot for direct recording.
- **Stereo width** on reverb and delay for mono-in, stereo-out use.
- **Accessibility**: every control exposes a name and a readable value to screen
  readers, which JUCE supports out of the box.
- **A "random board" button**, because it fits the vibe.

## Contributing

This is a single-maintainer project.

- **Issues are welcome.** Bug reports, feature requests and suggestions for
  pedals or algorithms are all appreciated. If you suggest an algorithm, please
  link to its original source and state its license.
- **Pull requests are not accepted** and will be closed without review. This is a
  security decision: no third-party code enters the build. The AGPLv3 of course
  lets you fork and build your own version.

## License

FOSS Pedalboard is released under the GNU Affero General Public License v3.0.
See [LICENSE](LICENSE).

- JUCE is used under its AGPLv3 option.
- The VST3 SDK is used under the GPLv3.
- VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe
  and other countries.
