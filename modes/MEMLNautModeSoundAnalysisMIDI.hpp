#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "../ThruAudioApp.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>
#include "../src/memllib/audio/AudioDriver.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "../MachineListeningMixin.hpp"



class MEMLNautModeSoundAnalysisMIDI {
public:
    constexpr static size_t kN_InputParams = InterfaceRL::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;
    InterfaceRL interface;
    std::shared_ptr<InterfaceRL> interfacePtr;
    MachineListeningMixin mlMixin;

    ThruAudioApp<> audioAppSoundAnalysisMIDI;
    std::array<String, ThruAudioApp<>::nVoiceSpaces> voiceSpaceList;
    std::shared_ptr<MIDIInOut> midi_interf;

    void setupInterface() {
        interface.setup(kN_InputParams, ThruAudioApp<>::kN_Params);
        interface.bindInterface(InterfaceRL::INPUT_MODES::JOYSTICK_AND_MACHINE_LISTENING);
        interface.setModeInfo("samidi", "SoundAnalMIDI");
        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() {
        return "Sound Analysis MIDI Mode";
    }
    
    __force_inline stereosample_t process(stereosample_t x) {
        return audioAppSoundAnalysisMIDI.Process(x);
    }

    void setupMIDI(std::shared_ptr<MIDIInOut> new_midi_interf) {
        midi_interf = new_midi_interf;
        midi_interf->Setup(8);
        midi_interf->SetMIDISendChannel(1);
        interface.bindMIDI(midi_interf);
    }

    void addViews() {
        interface.addInputSourceView();
    };

    void setupAudio(float sample_rate) {
        audioAppSoundAnalysisMIDI.Setup(sample_rate, interfacePtr);
        mlMixin.setup(interface);
    }

    __force_inline void loop() {
        audioAppSoundAnalysisMIDI.loop();
    }

    __force_inline void analyse(stereosample_t x) { mlMixin.analyse(x); }

    __force_inline void processAnalysisParams() { mlMixin.processAnalysisParams(); }

    AudioDriver::codec_config_t getCodecConfig() { return audioAppSoundAnalysisMIDI.GetDriverConfig(); }

    void loopCore0() {}

};
