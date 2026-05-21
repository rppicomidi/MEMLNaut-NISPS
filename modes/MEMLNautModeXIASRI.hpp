#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "../XIASRIAudioApp.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>
#include "../src/memllib/audio/AudioDriver.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "../MachineListeningMixin.hpp"



class MEMLNautModeXIASRI {
public:
    constexpr static size_t kN_InputParams = InterfaceRL::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;
    InterfaceRL interface;
    std::shared_ptr<InterfaceRL> interfacePtr;
    MachineListeningMixin mlMixin;

    XIASRIAudioApp<> audioAppXIASRI;
    std::shared_ptr<MIDIInOut> midi_interf;

    void setupInterface() {
        interface.setup(kN_InputParams, XIASRIAudioApp<>::kN_Params);
        interface.bindInterface(InterfaceRL::INPUT_MODES::MACHINE_LISTENING);
        interface.setModeInfo("xiasri", "XIASRI");
        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() {
        return "XIASRI Mode";
    }
    
    __force_inline stereosample_t process(stereosample_t x) {
        return audioAppXIASRI.Process(x);
    }

    void setupMIDI(std::shared_ptr<MIDIInOut> new_midi_interf) {
        midi_interf = new_midi_interf;
        midi_interf->Setup(0);
        midi_interf->SetMIDISendChannel(1);
        interface.bindMIDI(midi_interf, true);
    }

    void addViews() {
        interface.addInputSourceView();
    };

    void setupAudio(float sample_rate) {
        audioAppXIASRI.Setup(sample_rate, interfacePtr);
        mlMixin.setup(interface);
    }

    __force_inline void loop() {
        audioAppXIASRI.loop();
    }

    __force_inline void analyse(stereosample_t x) { mlMixin.analyse(x); }

    __force_inline void processAnalysisParams() { mlMixin.processAnalysisParams(); }

    AudioDriver::codec_config_t getCodecConfig() { return audioAppXIASRI.GetDriverConfig(); }

    void loopCore0() {}

};
