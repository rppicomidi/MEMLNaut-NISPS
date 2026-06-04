# MEMLNaut MIDI Control Modes — Implementation Notes

## Erica Synths Resonant Filterbank (`MEMLNautModeEricaFBank`)

**File:** `modes/MEMLNautModeEricaFBank.hpp`  
**Enable:** `#define MODE_ERICA_FBANK` in `MEMLNaut-NISPS.ino`

### Device Overview
10-band analog stereo filterbank. Each band is a bandpass filter at a fixed frequency. Bands can self-oscillate at high resonance (no-input mixer style). Physical MIDI channel is configured on the device via CONFIG → MIDI.

Filter frequencies: 29 Hz, 61 Hz, 115 Hz, 218 Hz, 411 Hz, 777 Hz, 1.5 kHz, 2.8 kHz, 5.2 kHz, 11 kHz

### MIDI Implementation
Source: official V1 manual PDF (downloaded from product page, file_id 650).

CCs **70–83** are **active-mode dependent** — the same CC number controls a different parameter depending on which mode the Erica device is currently in.

| CC | Filterbank (FB) mode | Filter mode | CLK MOD mode | DYN EQ mode |
|----|----------------------|-------------|--------------|-------------|
| 70 | Band 1 (29 Hz)       | Cutoff      | Mod Gain     | Threshold   |
| 71 | Band 2 (61 Hz)       | Slope       | Waveshape    | Ctrl Decay  |
| 72 | Band 3 (115 Hz)      | Width       | Frequency    | Ctrl Attack |
| 73 | Band 4 (218 Hz)      | Type        | Step Dir     | Env Delay   |
| 74 | Band 5 (411 Hz)      | Gain        | BPM/Scale    | Filter Freq |
| 75 | Band 6 (777 Hz)      | Offset      | Band Offset  | Audio Src   |
| 76 | Band 7 (1.5 kHz)     | —           | R Ch Invert  | Band Offset |
| 77 | Band 8 (2.8 kHz)     | —           | —            | —           |
| 78 | Band 9 (5.2 kHz)     | —           | —            | —           |
| 79 | Band 10 (11 kHz)     | —           | —            | —           |
| 80 | Band Low Gain        | —           | —            | —           |
| 81 | Band Mid Gain        | —           | —            | —           |
| 82 | Band High Gain       | —           | —            | —           |
| 83 | Band All Gain        | —           | —            | —           |

**Universal CCs (always active, both stereo channels):**

| CC | Parameter      |
|----|----------------|
| 84 | Resonance Level |
| 85 | Feedback CH 1  |
| 86 | Feedback CH 2  |
| 87 | Feedback CH 3  |
| 88 | Feedback CH 4  |
| 89 | Feedback CH 5  |
| 90 | Feedback CH 6  |
| 91 | Feedback CH 7  |
| 92 | Feedback CH 8  |
| 93 | Feedback CH 9  |
| 94 | Feedback CH 10 |
| 95 | Feedback CH All |
| 96 | Mix (Dry/Wet)  |
| 97 | Spread         |

The device also supports separate MIDI channels for left and right stereo channels (MIDI IN L CH / MIDI IN R CH), configurable in the device menu.

### Mode Implementation
- **Params:** 16 (`TRxSAudioApp<16>`)
- **Default CCs:** 70–79 (10 individual bands), 80–82 (Band Low/Mid/High grouped), 84 (Resonance), 96–97 (Mix/Spread)
- **MIDI channel:** 1 (default; change on device via CONFIG → MIDI)
- **Notes:** not passed through (FX unit, not a pitch instrument)
- **CC throttle:** 30 ms
- **Flash save file:** `/erica_fbank_cc.bin`

### Focus Groups
| Group | CCs | Frequency range |
|-------|-----|-----------------|
| LOW   | 70–72 | 29–115 Hz |
| MID   | 73–76 | 218 Hz – 1.5 kHz |
| HIGH  | 77–79 | 2.8–11 kHz |
| GRP   | 80–83 | Grouped band gains |
| RESO  | 84–95 | Resonance + feedback channels |
| GLOB  | 96–97 | Mix + spread |

Plus **HOLD** button (freezes CC output).

### Usage Notes
- The CC options list includes all four modes (FB / Filter / CLK MOD / DYN EQ) with mode prefix labels (e.g. `"FB B1 29Hz"`, `"Flt: Cutoff"`, `"CLK: Mod Gain"`, `"DYN: Threshold"`). Since CCs 70–83 are mode-dependent on the device, pick the labels matching the mode the Erica unit is currently in.
- The device MIDI OUT port can be set to MIDI THRU or MIDI OUT (sends CC values as you tweak hardware controls). Useful for recording automation into a DAW.

---

## Erica Synths × 112dB Steampipe (`MEMLNautModeSteampipe`)

**File:** `modes/MEMLNautModeSteampipe.hpp`  
**Enable:** `#define MODE_STEAMPIPE` in `MEMLNaut-NISPS.ino`

### Device Overview
8-voice polyphonic physical modelling synthesizer (Karplus–Strong variant). Models wind and string instruments. 32 CC-controllable parameters organised in four sections: Envelope, Steam (generator), Pipe (resonator), Reverberator.

### MIDI Implementation
Source: V3 manual PDF + firmware UF2 binary analysis.

Factory default CC assignments are **sequential from CC 70**. The Envelope section (CCs 70–74) is **confirmed** from a device OLED screenshot in the manual. All remaining CCs are **inferred** from the sequential pattern and the parameter name list extracted from the firmware string table.

CCs can be remapped on the device via Settings → MIDI → MIDI CC MAP.

#### Envelope section (CCs 70–74) — CONFIRMED
| CC | Parameter |
|----|-----------|
| 70 | Attack *(inferred — row 0 was selected showing "Learn" instead of its number)* |
| 71 | Decay |
| 72 | Sustain |
| 73 | Release |
| 74 | Scaling (note keytrack for envelope times) |

#### Steam / generator section (CCs 75–81) — inferred
| CC | Parameter |
|----|-----------|
| 75 | Velocity (envelope velocity sensitivity) |
| 76 | DC/Noise mix |
| 77 | LPF Cutoff |
| 78 | LPF Resonance |
| 79 | LPF Keytrack |
| 80 | LPF Envelope Amount |
| 81 | Output Gain |

#### Pipe / resonator section (CCs 82–90) — inferred
| CC | Parameter |
|----|-----------|
| 82 | Push (push-pull feedback mechanism) |
| 83 | SoftHard (saturation type) |
| 84 | Symmetry (saturation even/odd harmonics) |
| 85 | Fine Tune (cents) |
| 86 | Harmonics (stiffness / harmonic spread) |
| 87 | Split Point |
| 88 | Feedback Decay |
| 89 | Feedback Release |
| 90 | Patch Volume |

#### Reverberator section (CCs 91–97+) — inferred
| CC | Parameter |
|----|-----------|
| 91 | Dry/Wet |
| 92 | Master Volume |
| 93 | Spread |
| 94 | Room Size |
| 95 | Spin (reverb detuning/richness) |
| 96 | HF Damping |
| 97 | LF Damping |
| 98–101 | Additional params (device has 32 total; these 4 are unconfirmed) |

### Mode Implementation
- **Params:** 16 (`TRxSAudioApp<16>`)
- **Default CCs:** 70, 71, 73 (Env), 77, 78, 80, 81 (Steam), 82, 86, 88, 89 (Pipe), 90, 91, 93, 94, 95 (Pipe Vol + Reverb)
- **MIDI channel:** 1 (default; configurable on device via Settings → MIDI → MIDI IN CH)
- **Notes:** passed through (polyphonic synth — notes sent to device)
- **CC throttle:** 30 ms
- **Flash save file:** `/steampipe_cc.bin`

### Focus Groups
| Group | CCs |
|-------|-----|
| ENV   | 70–74 |
| STEAM | 75–81 |
| PIPE  | 82–90 |
| REVERB| 91–101 |

Plus **HOLD** button (freezes CC output).

### Usage Notes
- **Verify CC numbers on the device:** scroll the Steampipe's MIDI CC MAP menu (Settings → MIDI → MIDI CC MAP) to confirm all 32 assignments. The confirmed range is CCs 70–74 for the Envelope section. Update `kSteampipeCCOptions` if the remaining CCs differ from the sequential pattern.
- **Wind controller MPE setup (from official FAQ):** in the Steampipe MIDI DEVICE menu, set Source A to CC 74, Source B to poly. Then route Source A in the MOD MATRIX to desired destinations (e.g. LPF Modulation, Generator Amount). This is separate from the CC MAP.
- **Note passthrough** is active — the MEMLNaut passes note-on/note-off messages to the Steampipe so it can be played from the joystick-triggered note source.
- **Preset backup:** device supports USB drag-and-drop preset backup (`.spp` files). 32 factory presets included; 192 user slots available.
