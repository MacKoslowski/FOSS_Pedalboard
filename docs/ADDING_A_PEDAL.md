# Adding a pedal

A pedal is one class deriving from `fpb::Pedal` plus one line in the registry.
Everything else (parameters in the host, the board slot, the "(i)" credit, the
default knob panel, state save/restore) comes for free.

## Before writing code

1. Pick an algorithm with a clear open license: public domain, a published paper
   or textbook, or code under MIT/BSD/ISC/zlib or a GPL-compatible license.
   Nothing reverse-engineered from a commercial product.
2. Add a row to `docs/CREDITS.md` with the source and license.
3. Choose a descriptive display name. No trademarked pedal names.

## Steps

1. **Create `src/pedals/FooPedal.h` and `.cpp`.** Start the header with a comment
   block citing the algorithm's origin (author, paper/project, URL, license).
   Derive from `Pedal` and give the class:
   - `kTypeId` (lowercase, stable forever, used in parameter ids and saved state)
   - `kDisplayName` (uppercase, what is written on the enclosure)
   - `pXxx` constants for each parameter name
2. **Implement the interface.**
   - `addParameters()` appends `AudioParameter*` objects with ids built from `paramId (pXxx)`.
   - `attachParameters()` caches `getRawParameterValue()` pointers.
   - `prepare()` allocates everything; `reset()` clears state; `process()` is realtime-safe.
   - `getAlgorithmCredit()` returns the tooltip text: algorithm, source, license.
   - `createEditor()` can simply return a `ParameterPanel` listing the parameter ids and short labels.
3. **Register it.** Add one line to the list in `src/core/PedalRegistry.cpp`.
4. **Add the sources to `CMakeLists.txt`** (`FOSSPEDALBOARD_SOURCES` and `FOSSPEDALBOARD_HEADERS`).
5. **Write a test** in `tests/test_Pedals.cpp`: silence in gives silence out, no NaNs, and one behavioural check (an echo lands where expected, a gate closes, a filter attenuates).
6. **Update the pedal table in `README.md`.**
7. **Artwork.** The procedural look-and-feel draws placeholder knobs and switches. Ask the maintainer for drawings of any new control before inventing one.

## Rules of thumb

- Parameters exist whether or not the pedal is on the board, so defaults must be sensible and the pedal must be silent-safe when idle.
- `reset()` is called on the audio thread the first time the pedal lands in a slot. Keep it allocation-free.
- Smooth anything audible (`juce::SmoothedValue`) to avoid zipper noise.
- Keep the file under about 400 lines. If the DSP is reusable, put the building block in `src/dsp/` with its own citation.
