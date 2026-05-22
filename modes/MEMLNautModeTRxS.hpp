#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "../src/memllib/hardware/memlnaut/display/XYPadView.hpp"
#include "../src/memllib/hardware/memlnaut/display/CCSelectView.hpp"
#include "../src/memllib/hardware/memlnaut/MEMLNaut.hpp"
#include "../TRxSAudioApp.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>

// TR-6S MIDI CC targets — all 33 supported CCs, grouped by instrument then effects
static const std::vector<CCOption> kTR6SCCOptions = {
    { 96, "BD Ctrl"        },
    { 97, "SD Ctrl"        },
    { 25, "SD Tune"        },
    { 28, "SD Decay"       },
    { 29, "SD Level"       },
    {102, "LT Ctrl"        },
    { 46, "LT Tune"        },
    { 47, "LT Decay"       },
    { 48, "LT Level"       },
    {106, "HC Ctrl"        },
    { 58, "HC Tune"        },
    { 59, "HC Decay"       },
    { 60, "HC Level"       },
    {107, "CH Ctrl"        },
    { 61, "CH Tune"        },
    { 62, "CH Decay"       },
    { 63, "CH Level"       },
    {108, "OH Ctrl"        },
    { 80, "OH Tune"        },
    { 81, "OH Decay"       },
    { 82, "OH Level"       },
    { 91, "Reverb Level"   },
    { 16, "Delay Level"    },
    { 17, "Delay Time"     },
    { 18, "Delay Feedback" },
    { 19, "Master FX Ctrl" },
    { 15, "Master FX"      },
    { 71, "Accent"         },
    {  9, "Shuffle"        },
    { 70, "Fill IN Trig"   },
};

// Default 8 assignments: the 6 Ctrl knobs + Reverb + Delay Level
static const std::vector<uint8_t> kTR6SDefaultCCs = { 96, 97, 102, 106, 107, 108, 91, 16 };

class MEMLNautModeTRxS {
public:
    constexpr static size_t kN_InputParams    = InterfaceRL::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;

    inline static TRxSAudioApp<> audioApp;
    std::array<String, TRxSAudioApp<>::nVoiceSpaces> voiceSpaceList;

    InterfaceRL interface;
    std::shared_ptr<InterfaceRL> interfacePtr;

    void setupInterface() {
        interface.setup(kN_InputParams, TRxSAudioApp<>::kN_Params);
        interface.bindInterface(InterfaceRL::INPUT_MODES::JOYSTICK, true);
        interface.setModeInfo("trxs", "TR-6S");
        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() { return "TR-6S Mode"; }

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
        midi_interf->Setup(TRxSAudioApp<>::kN_Params);
        midi_interf->SetMIDISendChannel(10);  // TR-6S default channel
        midi_interf->SetParamCCNumbers(kTR6SDefaultCCs);
        // No paramOutputHook — default SendParamsAsMIDICC() path is used
        interface.bindMIDI(midi_interf);
    }

    void addViews() {
        // CC assignment screen
        auto ccView = std::make_shared<CCSelectView>(TRxSAudioApp<>::kN_Params, "CC Assign");
        ccView->setOptions(kTR6SCCOptions);
        ccView->setSelectedCCs(kTR6SDefaultCCs);
        ccView->setOnChangeCallback([this](const std::vector<uint8_t>& ccs) {
            midi_interf->SetParamCCNumbers(ccs);
        });
        MEMLNaut::Instance()->disp->AddView(ccView);
        interface.addInputSourceView(false);
    }

    inline void processAnalysisParams() {}
    void analyse(stereosample_t) {}
    AudioDriver::codec_config_t getCodecConfig() { return audioApp.GetDriverConfig(); }
    void loopCore0() {}
};
