#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "../src/memllib/hardware/memlnaut/display/XYPadView.hpp"
#include "../src/memllib/hardware/memlnaut/display/CCSelectView.hpp"
#include "../src/memllib/hardware/memlnaut/display/BlockSelectView.hpp"
#include "../src/memllib/hardware/memlnaut/MEMLNaut.hpp"
#include "../src/memllib/audio/FocusManager.hpp"
#include "../TRxSAudioApp.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>
#include <cstdio>

// DSI Mopho MIDI CC options — grouped by synthesis section
// CC numbers per the Mopho MIDI Implementation Chart; verify against manual if uncertain
static const std::vector<CCOption> kMophoCCOptions = {
    // Performance
    {  1, "Mod Wheel"      },
    {  7, "Volume"         },
    { 10, "Pan"            },
    // OSC 1
    { 15, "OSC1 Freq"      },
    { 16, "OSC1 Fine"      },
    { 17, "OSC1 Glide"     },
    { 18, "OSC1 Shape"     },
    { 19, "OSC1 PW"        },
    // OSC 2
    { 20, "OSC2 Freq"      },
    { 21, "OSC2 Fine"      },
    { 22, "OSC2 Glide"     },
    { 23, "OSC2 Shape"     },
    { 24, "OSC2 PW"        },
    { 25, "OSC Sync"       },
    // Mixer
    { 26, "OSC1 Level"     },
    { 27, "OSC2 Level"     },
    { 28, "Sub Osc Lvl"    },
    { 29, "Sub Osc Shape"  },
    { 30, "Noise Level"    },
    { 31, "Feedback"       },
    // Filter
    { 74, "Cutoff"         },
    { 71, "Resonance"      },
    { 35, "Filt Env Amt"   },
    { 36, "Filt Env Vel"   },
    { 37, "Key Track"      },
    { 38, "Audio Mod"      },
    { 39, "4-Pole"         },
    // Filter Envelope
    { 40, "FE Attack"      },
    { 41, "FE Decay"       },
    { 42, "FE Sustain"     },
    { 43, "FE Release"     },
    { 44, "FE Delay"       },
    // Amp
    { 45, "VCA Level"      },
    { 46, "VCA Velocity"   },
    // Amp Envelope
    { 47, "AE Attack"      },
    { 48, "AE Decay"       },
    { 49, "AE Sustain"     },
    { 50, "AE Release"     },
    { 51, "AE Delay"       },
    // LFO 1
    { 52, "LFO1 Rate"      },
    { 53, "LFO1 Shape"     },
    { 54, "LFO1 Amount"    },
    { 55, "LFO1 Dest"      },
    { 56, "LFO1 Key Sync"  },
    // LFO 2
    { 57, "LFO2 Rate"      },
    { 58, "LFO2 Shape"     },
    { 59, "LFO2 Amount"    },
    { 60, "LFO2 Dest"      },
    { 61, "LFO2 Key Sync"  },
    // LFO 3
    { 62, "LFO3 Rate"      },
    { 63, "LFO3 Shape"     },
    { 65, "LFO3 Amount"    },
    { 66, "LFO3 Dest"      },
    { 67, "LFO3 Key Sync"  },
    // LFO 4
    { 68, "LFO4 Rate"      },
    { 69, "LFO4 Shape"     },
    { 70, "LFO4 Amount"    },
    { 72, "LFO4 Dest"      },
    { 73, "LFO4 Key Sync"  },
    // Modulation matrix (slots 1–4: source, dest, amount)
    { 75, "Mod1 Source"    },
    { 76, "Mod1 Dest"      },
    { 77, "Mod1 Amount"    },
    { 78, "Mod2 Source"    },
    { 79, "Mod2 Dest"      },
    { 80, "Mod2 Amount"    },
    { 81, "Mod3 Source"    },
    { 82, "Mod3 Dest"      },
    { 83, "Mod3 Amount"    },
    { 84, "Mod4 Source"    },
    { 85, "Mod4 Dest"      },
    { 86, "Mod4 Amount"    },
    // Misc
    {100, "Pitch Bend Rng" },
    {103, "BPM Tempo"      },
};

// Default 16: filter, both envelopes, LFO rates, OSC tuning, mod wheel, feedback
static const std::vector<uint8_t> kMophoDefaultCCs = {
    74, 71,           // Cutoff, Resonance
    40, 41, 42, 43,   // Filter Env ADSR
    47, 48, 49, 50,   // Amp Env ADSR
    52, 57,           // LFO1 Rate, LFO2 Rate
    15, 20,           // OSC1 Freq, OSC2 Freq
     1, 31            // Mod Wheel, Feedback
};

// Focus group bitmasks
static constexpr uint32_t kMophoFocusOSC  = (1u << 0);
static constexpr uint32_t kMophoFocusMIX  = (1u << 1);
static constexpr uint32_t kMophoFocusFILT = (1u << 2);
static constexpr uint32_t kMophoFocusENV  = (1u << 3);
static constexpr uint32_t kMophoFocusLFO  = (1u << 4);
static constexpr uint32_t kMophoFocusMOD  = (1u << 5);

class MEMLNautModeMopho {
public:
    constexpr static size_t kN_InputParams     = InterfaceRLBase::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;

    inline static TRxSAudioApp<32> audioApp;
    std::array<String, TRxSAudioApp<32>::nVoiceSpaces> voiceSpaceList;

    using InterfaceRL_t = InterfaceRL<TRxSAudioApp<32>::kN_Params>;
    InterfaceRL_t interface;
    std::shared_ptr<InterfaceRL_t> interfacePtr;

    FocusManager<TRxSAudioApp<32>::kN_Params, 6> focusManager;
    uint32_t presentGroupsMask_ = 0;

    void setupInterface() {
        interface.setup(kN_InputParams, TRxSAudioApp<32>::kN_Params);
        interface.bindInterface(InterfaceRLBase::INPUT_MODES::JOYSTICK, true);
        interface.setModeInfo("mopho", "Mopho");
        interfacePtr = make_non_owning(interface);

        focusManager.setGroupName(0, "OSC");
        focusManager.setGroupName(1, "MIX");
        focusManager.setGroupName(2, "FILT");
        focusManager.setGroupName(3, "ENV");
        focusManager.setGroupName(4, "LFO");
        focusManager.setGroupName(5, "MOD");

        interface.paramTransformHook = [this](std::vector<float>& p) {
            focusManager.applyInPlace(p);
        };
    }

    String getHelpTitle() { return "DSI Mopho Mode"; }

    __force_inline stereosample_t process(stereosample_t x) {
        return audioApp.Process(x);
    }

    void setupAudio(float sample_rate) {
        audioApp.Setup(sample_rate, interfacePtr);
        voiceSpaceList = audioApp.getVoiceSpaceNames();
    }

    __force_inline void loop() { audioApp.loop(); }

    std::shared_ptr<MIDIInOut> midi_interf;

    void setupMIDI(std::shared_ptr<MIDIInOut> new_midi_interf) {
        midi_interf = new_midi_interf;
        midi_interf->Setup(TRxSAudioApp<32>::kN_Params);
        midi_interf->SetMIDISendChannel(1);
        midi_interf->SetMIDINoteChannel(1);
        midi_interf->SetParamCCNumbers(kMophoDefaultCCs);
        midi_interf->SetNoteCallback([this](bool note_on, uint8_t note, uint8_t velocity) {
            if (note_on)
                midi_interf->sendNoteOn(note, velocity);
            else
                midi_interf->sendNoteOff(note, velocity);
        });
        interface.bindMIDI(midi_interf);
        interface.paramOutputHook = [this](std::span<const float> params) {
            if (held_) return;
            uint32_t now = millis();
            if (now - lastCCSendMs_ >= 30) {
                midi_interf->SendParamsAsMIDICC(params);
                lastCCSendMs_ = now;
            }
        };
    }

    void addViews() {
        auto updateActiveDims = [this]() {
            uint32_t mask = focusManager.getSelectedMask();
            constexpr size_t N = TRxSAudioApp<32>::kN_Params;
            std::vector<bool> active(N);
            for (size_t i = 0; i < N; i++)
                active[i] = (mask == 0) || ((focusManager.paramGroupMask[i] & mask) != 0);
            interface.setActiveDims(active);
        };
        updateActiveDims();

        auto focusView = std::make_shared<BlockSelectView>(
            "Focus", TFT_DARKGREY, 7, 60, 60, TFT_WHITE,
            std::vector<String>{"OSC", "MIX", "FILT", "ENV", "LFO", "MOD", "HOLD"},
            TFT_GREENYELLOW, 2);

        focusView->SetOnSelectCallback([this, focusView, updateActiveDims](size_t id) {
            if (id == 7) {
                held_ = !held_;
                focusView->toggleAlt(6);
                return;
            }
            size_t groupIdx = id - 1;
            if (!((presentGroupsMask_ >> groupIdx) & 1u)) return;
            uint32_t newMask = focusManager.getSelectedMask() ^ (1u << groupIdx);
            focusManager.setFocus(newMask, interface.getLastAction());
            focusView->toggleAlt(groupIdx);
            updateActiveDims();
        });
        MEMLNaut::Instance()->disp->InsertViewAfter(interface.nnOutputsGraphView, focusView);

        auto ccView = std::make_shared<CCSelectView>(TRxSAudioApp<32>::kN_Params, "CC Assign");
        ccView->setOptions(kMophoCCOptions);

        auto saved = loadCCAssignments();
        ccView->setSelectedCCs(saved);
        midi_interf->SetParamCCNumbers(saved);
        recomputeFocusGroups(saved, focusView, updateActiveDims);

        ccView->setOnChangeCallback([this, focusView, updateActiveDims](const std::vector<uint8_t>& ccs) {
            midi_interf->SetParamCCNumbers(ccs);
            saveCCAssignments(ccs);
            recomputeFocusGroups(ccs, focusView, updateActiveDims);
        });
        MEMLNaut::Instance()->disp->AddView(ccView);
        interface.addInputSourceView(false);
    }

    inline void processAnalysisParams() {}
    void analyse(stereosample_t) {}
    AudioDriver::codec_config_t getCodecConfig() { return audioApp.GetDriverConfig(); }
    void loopCore0() {}

private:
    static constexpr const char* kCCFile = "/mopho_cc.bin";
    uint32_t lastCCSendMs_ = 0;
    bool held_ = false;

    static uint32_t ccToGroupMask(uint8_t cc) {
        if (cc >= 15 && cc <= 25)  return kMophoFocusOSC;
        if (cc >= 26 && cc <= 31)  return kMophoFocusMIX;
        if ((cc >= 35 && cc <= 39) || cc == 71 || cc == 74)  return kMophoFocusFILT;
        if (cc >= 40 && cc <= 51)  return kMophoFocusENV;
        if (cc >= 52 && cc <= 73)  return kMophoFocusLFO;
        if (cc >= 75 && cc <= 86)  return kMophoFocusMOD;
        return 0;  // performance CCs (1, 7, 10, 100, 103) — always active
    }

    void recomputeFocusGroups(const std::vector<uint8_t>& ccs,
                              std::shared_ptr<BlockSelectView> focusView,
                              std::function<void()> updateActiveDims) {
        std::array<uint32_t, TRxSAudioApp<32>::kN_Params> masks{};
        for (size_t i = 0; i < ccs.size() && i < masks.size(); i++)
            masks[i] = ccToGroupMask(ccs[i]);
        focusManager.setParamGroups(masks);

        presentGroupsMask_ = 0;
        for (auto m : masks) presentGroupsMask_ |= m;

        uint32_t currentSel = focusManager.getSelectedMask();
        uint32_t validSel   = currentSel & presentGroupsMask_;
        if (validSel != currentSel) {
            focusManager.setFocus(validSel, interface.getLastAction());
            for (size_t i = 0; i < 6; i++) {
                if (((currentSel >> i) & 1u) && !((validSel >> i) & 1u))
                    focusView->setAltState(i, false);
            }
        }
        updateActiveDims();
    }

    std::vector<uint8_t> loadCCAssignments() {
        FILE* f = fopen(kCCFile, "rb");
        if (f) {
            std::vector<uint8_t> ccs;
            uint8_t b;
            while (fread(&b, 1, 1, f) == 1) ccs.push_back(b);
            fclose(f);
            Serial.printf("Mopho: loaded %u CC assignments from flash\n", (unsigned)ccs.size());
            if (!ccs.empty()) return ccs;
        } else {
            Serial.println("Mopho: no saved CC assignments, using defaults");
        }
        return std::vector<uint8_t>(kMophoDefaultCCs.begin(), kMophoDefaultCCs.end());
    }

    void saveCCAssignments(const std::vector<uint8_t>& ccs) {
        FILE* f = fopen(kCCFile, "wb");
        if (f) {
            fwrite(ccs.data(), 1, ccs.size(), f);
            fclose(f);
            Serial.printf("Mopho: saved %u CC assignments to flash\n", (unsigned)ccs.size());
        } else {
            Serial.println("Mopho: failed to open flash for writing");
        }
    }
};
