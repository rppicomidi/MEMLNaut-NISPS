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

// Waldorf microQ MIDI CC options — grouped by synthesis section
static const std::vector<CCOption> kMicroQCCOptions = {
    // Performance
    {  1, "Mod Wheel"       },
    {  5, "Glide Rate"      },
    { 10, "Pan"             },
    { 50, "Pitch Mod"       },
    { 51, "Glide Mode"      },
    { 65, "Glide Active"    },
    // Arpeggiator
    { 12, "Arp Range"       },
    { 13, "Arp Length"      },
    { 14, "Arp Active"      },
    // LFO 1
    { 17, "LFO1 Shape"      },
    { 18, "LFO1 Speed"      },
    { 19, "LFO1 Sync"       },
    { 20, "LFO1 Delay"      },
    // LFO 2
    { 21, "LFO2 Shape"      },
    { 22, "LFO2 Speed"      },
    { 23, "LFO2 Sync"       },
    { 24, "LFO2 Delay"      },
    // LFO 3
    { 25, "LFO3 Shape"      },
    { 26, "LFO3 Speed"      },
    { 27, "LFO3 Sync"       },
    { 28, "LFO3 Delay"      },
    // Oscillator 1
    { 29, "OSC1 Octave"     },
    { 30, "OSC1 Semi"       },
    { 31, "OSC1 Detune"     },
    { 32, "OSC1 FM"         },
    { 33, "OSC1 Shape"      },
    { 34, "OSC1 PW"         },
    { 35, "OSC1 PWM"        },
    // Oscillator 2
    { 36, "OSC2 Octave"     },
    { 37, "OSC2 Semi"       },
    { 38, "OSC2 Detune"     },
    { 39, "OSC2 FM"         },
    { 40, "OSC2 Shape"      },
    { 41, "OSC2 PW"         },
    { 42, "OSC2 PWM"        },
    // Oscillator 3
    { 43, "OSC3 Octave"     },
    { 44, "OSC3 Semi"       },
    { 45, "OSC3 Detune"     },
    { 46, "OSC3 FM"         },
    { 47, "OSC3 Shape"      },
    { 48, "OSC3 PW"         },
    { 49, "OSC3 PWM"        },
    // Mix
    { 52, "OSC1 Level"      },
    { 53, "OSC1 Balance"    },
    { 54, "RingMod Level"   },
    { 55, "RingMod Balance" },
    { 56, "OSC2 Level"      },
    { 57, "OSC2 Balance"    },
    { 58, "OSC3 Level"      },
    { 59, "OSC3 Balance"    },
    { 60, "Noise Level"     },
    { 61, "Noise Balance"   },
    // Filter 1
    { 68, "F1 Type"         },
    { 70, "F1 Cutoff"       },
    { 71, "F1 Resonance"    },
    { 72, "F1 Drive"        },
    { 73, "F1 Keytrack"     },
    { 74, "F1 Env Amt"      },
    { 76, "F1 CutoffMod"    },
    { 77, "F1 FM"           },
    { 78, "F1 Pan"          },
    { 79, "F1 PanMod"       },
    // Filter 2
    { 80, "F2 Type"         },
    { 81, "F2 Cutoff"       },
    { 82, "F2 Resonance"    },
    { 83, "F2 Drive"        },
    { 84, "F2 Keytrack"     },
    { 85, "F2 Env Amt"      },
    { 87, "F2 CutoffMod"    },
    { 88, "F2 FM"           },
    { 89, "F2 Pan"          },
    { 90, "F2 PanMod"       },
    // Amp & FX
    { 93, "Amp Mod"         },
    { 94, "FX1 Mix"         },
    { 95, "FX2 Mix"         },
    // Filter Envelope
    { 96, "FE Attack"       },
    { 97, "FE Decay"        },
    { 98, "FE Sustain"      },
    { 99, "FE Decay2"       },
    {100, "FE Sustain2"     },
    {101, "FE Release"      },
    // Amp Envelope
    {102, "AE Attack"       },
    {103, "AE Decay"        },
    {104, "AE Sustain"      },
    {105, "AE Decay2"       },
    {106, "AE Sustain2"     },
    {107, "AE Release"      },
    // Envelope 3
    {108, "E3 Attack"       },
    {109, "E3 Decay"        },
    {110, "E3 Sustain"      },
    {111, "E3 Decay2"       },
    {112, "E3 Sustain2"     },
    {113, "E3 Release"      },
    // Envelope 4
    {114, "E4 Attack"       },
    {115, "E4 Decay"        },
    {116, "E4 Sustain"      },
    {117, "E4 Decay2"       },
    {118, "E4 Sustain2"     },
    {119, "E4 Release"      },
};

// Default 16: filter cutoffs/resonances, LFO speeds, filter envelope, OSC detunes, FX
static const std::vector<uint8_t> kMicroQDefaultCCs = {
    70, 71,      // F1 Cutoff, F1 Resonance
    81, 82,      // F2 Cutoff, F2 Resonance
    18, 22,      // LFO1 Speed, LFO2 Speed
    96, 97, 98, 101,  // FE Attack, Decay, Sustain, Release
    31, 38,      // OSC1 Detune, OSC2 Detune
    93, 94,      // Amp Mod, FX1 Mix
     1, 10       // Mod Wheel, Pan
};

// Focus group bitmasks
static constexpr uint32_t kMQFocusOSC    = (1u << 0);
static constexpr uint32_t kMQFocusMIX    = (1u << 1);
static constexpr uint32_t kMQFocusFILTER = (1u << 2);
static constexpr uint32_t kMQFocusENV    = (1u << 3);
static constexpr uint32_t kMQFocusLFO    = (1u << 4);
static constexpr uint32_t kMQFocusFX     = (1u << 5);

class MEMLNautModeMicroQ {
public:
    constexpr static size_t kN_InputParams     = InterfaceRL::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;

    inline static TRxSAudioApp<32> audioApp;
    std::array<String, TRxSAudioApp<32>::nVoiceSpaces> voiceSpaceList;

    InterfaceRL interface;
    std::shared_ptr<InterfaceRL> interfacePtr;

    FocusManager<TRxSAudioApp<32>::kN_Params, 6> focusManager;
    uint32_t presentGroupsMask_ = 0;

    void setupInterface() {
        interface.setup(kN_InputParams, TRxSAudioApp<32>::kN_Params);
        interface.bindInterface(InterfaceRL::INPUT_MODES::JOYSTICK, true);
        interface.setModeInfo("microq", "microQ");
        interfacePtr = make_non_owning(interface);

        focusManager.setGroupName(0, "OSC");
        focusManager.setGroupName(1, "MIX");
        focusManager.setGroupName(2, "FILT");
        focusManager.setGroupName(3, "ENV");
        focusManager.setGroupName(4, "LFO");
        focusManager.setGroupName(5, "FX");

        interface.paramTransformHook = [this](std::vector<float>& p) {
            focusManager.applyInPlace(p);
        };
    }

    String getHelpTitle() { return "Waldorf microQ Mode"; }

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
        midi_interf->SetMIDISendChannel(1);  // microQ default channel
        midi_interf->SetMIDINoteChannel(1);
        midi_interf->SetParamCCNumbers(kMicroQDefaultCCs);
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
            std::vector<String>{"OSC", "MIX", "FILT", "ENV", "LFO", "FX", "HOLD"},
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
        ccView->setOptions(kMicroQCCOptions);

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
    static constexpr const char* kCCFile = "/microq_cc.bin";
    uint32_t lastCCSendMs_ = 0;
    bool held_ = false;

    static uint32_t ccToGroupMask(uint8_t cc) {
        if (cc >= 17 && cc <= 28)   return kMQFocusLFO;
        if (cc >= 29 && cc <= 51)   return kMQFocusOSC;
        if (cc == 50 || cc == 51)   return kMQFocusOSC;
        if (cc >= 52 && cc <= 61)   return kMQFocusMIX;
        if (cc >= 68 && cc <= 90)   return kMQFocusFILTER;
        if (cc >= 91 && cc <= 95)   return kMQFocusFX;
        if (cc >= 96 && cc <= 119)  return kMQFocusENV;
        return 0;  // performance CCs (1, 5, 7, 10, etc.) — always active
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
            Serial.printf("microQ: loaded %u CC assignments from flash\n", (unsigned)ccs.size());
            if (!ccs.empty()) return ccs;
        } else {
            Serial.println("microQ: no saved CC assignments, using defaults");
        }
        return std::vector<uint8_t>(kMicroQDefaultCCs.begin(), kMicroQDefaultCCs.end());
    }

    void saveCCAssignments(const std::vector<uint8_t>& ccs) {
        FILE* f = fopen(kCCFile, "wb");
        if (f) {
            fwrite(ccs.data(), 1, ccs.size(), f);
            fclose(f);
            Serial.printf("microQ: saved %u CC assignments to flash\n", (unsigned)ccs.size());
        } else {
            Serial.println("microQ: failed to open flash for writing");
        }
    }
};
