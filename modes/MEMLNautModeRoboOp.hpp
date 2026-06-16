#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "../src/memllib/hardware/memlnaut/MEMLNaut.hpp"
#include "../TRxSAudioApp.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "MEMLNautMode.hpp"
#include "../MachineListeningMixin.hpp"
#include <memory>
#include <array>

class MEMLNautModeRoboOp {
public:
    constexpr static size_t kN_InputParams     = InterfaceRLBase::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;
    constexpr static size_t kN_Params         = 8;

    inline static TRxSAudioApp<kN_Params> audioApp;

    using InterfaceRL_t = InterfaceRL<kN_Params>;
    InterfaceRL_t interface;
    std::shared_ptr<InterfaceRL_t> interfacePtr;
    std::shared_ptr<MIDIInOut> midi_interf;
    MachineListeningMixin mlMixin;

    void setupInterface() {
        interface.setup(kN_InputParams, kN_Params);
        interface.bindInterface(InterfaceRLBase::INPUT_MODES::JOYSTICK, true);
        interface.setModeInfo("roboop", "RoboOp");
        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() { return "RoboOp Mode"; }

    __force_inline stereosample_t process(stereosample_t x) {
        return audioApp.Process(x);
    }

    void setupMIDI(std::shared_ptr<MIDIInOut> new_midi_interf) {
        midi_interf = new_midi_interf;
        midi_interf->Setup(kN_Params);
        midi_interf->SetMIDISendChannel(1);
        midi_interf->SetParamCCNumbers({1, 2, 3, 4, 5, 6, 7, 8});
        interface.bindMIDI(midi_interf);
    }

    void setupAudio(float sample_rate) {
        audioApp.Setup(sample_rate, interfacePtr);
        mlMixin.setup(interface);
    }

    void addViews() {
        interface.addInputSourceView(false);
    }

    __force_inline void loop() { audioApp.loop(); }

    __force_inline void analyse(stereosample_t x) { mlMixin.analyse(x); }
    __force_inline void processAnalysisParams() { mlMixin.processAnalysisParams(); }
    AudioDriver::codec_config_t getCodecConfig() {
        return { .mic_input = true, .line_level = 3, .mic_gain_dB = 0, .output_volume = 0.55f };
    }
    void loopCore0() {}
};
