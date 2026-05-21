#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "./AudioApps/VerbFXAudioApp.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>
#include "../src/memllib/audio/AudioDriver.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"



class MEMLNautModeVerbFX {
public:
    constexpr static size_t kN_InputParams = InterfaceRL::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;
    InterfaceRL interface;
    std::shared_ptr<InterfaceRL> interfacePtr;

    inline static VerbFXAudioApp<> audioAppVerbFX;
    std::array<String, VerbFXAudioApp<>::nVoiceSpaces> voiceSpaceList;
    std::shared_ptr<MIDIInOut> midi_interf;
    std::shared_ptr<BlockSelectView> enableView;


    void setupInterface() {
        interface.setup(kN_InputParams, VerbFXAudioApp<>::kN_Params);
        interface.setRVX1Override([this](float value) {
            audioAppVerbFX.setWetDryQueued(value);
        });
        interface.bindInterface(MEMLNAUT_INPUT_MODE, JOYSTICK_IS_4D);
        interface.setModeInfo("verbfx", "VerbFX");
        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() {
        return "VerbFX Mode";
    }
    
    __force_inline stereosample_t process(stereosample_t x) {
        return audioAppVerbFX.Process(x);
    }

    void setupMIDI(std::shared_ptr<MIDIInOut> new_midi_interf) {
        midi_interf = new_midi_interf;
        midi_interf->Setup(0);
        midi_interf->SetMIDISendChannel(1);
        interface.bindMIDI(midi_interf, true);
    }

    void addViews() {
        enableView = std::make_shared<BlockSelectView>("FX Enable", TFT_YELLOW, 6, 80, 70, TFT_BLACK,
            std::vector<String>{ "FilterBnk", "Reverb", "ShortDly", "MedDly", "LongDly", "Dly->Verb" },
            TFT_BLUE, 2);
        enableView->SetOnSelectCallback([this](size_t id) {
            enableView->toggleAlt(id - 1);
            queue_t& q = audioAppVerbFX.controlMessageQueue;
            switch(id) {
                case 1: { auto msg = VerbFXAudioApp<>::controlMessages::MSG_ENABLE_FILTERBANK;    queue_try_add(&q, &msg); } break;
                case 2: { auto msg = VerbFXAudioApp<>::controlMessages::MSG_ENABLE_REVERB;        queue_try_add(&q, &msg); } break;
                case 3: { auto msg = VerbFXAudioApp<>::controlMessages::MSG_ENABLE_SHORT_DELAY;   queue_try_add(&q, &msg); } break;
                case 4: { auto msg = VerbFXAudioApp<>::controlMessages::MSG_ENABLE_MEDIUM_DELAY;  queue_try_add(&q, &msg); } break;
                case 5: { auto msg = VerbFXAudioApp<>::controlMessages::MSG_ENABLE_LONG_DELAY;    queue_try_add(&q, &msg); } break;
                case 6: { auto msg = VerbFXAudioApp<>::controlMessages::MSG_ENABLE_DELAY_TO_REVERB; queue_try_add(&q, &msg); } break;
            }
        });
        MEMLNaut::Instance()->disp->AddView(enableView);

        std::shared_ptr<VoiceSpaceSelectView> voiceSpaceSelectView;
        voiceSpaceSelectView = std::make_shared<VoiceSpaceSelectView>("Voice Spaces");
        MEMLNaut::Instance()->disp->InsertViewAfter(interface.nnOutputsGraphView, voiceSpaceSelectView);
        voiceSpaceSelectView->setOptions(voiceSpaceList);
        voiceSpaceSelectView->setNewVoiceCallback(
            [this](size_t idx) {
                audioAppVerbFX.setVoiceSpace(idx);
            });
        interface.addInputSourceView();
    };

    void setupAudio(float sample_rate) {
        audioAppVerbFX.Setup(sample_rate, interfacePtr);
        voiceSpaceList = audioAppVerbFX.getVoiceSpaceNames();
    }

    __force_inline void loop() {
        audioAppVerbFX.loop();
    }

    __force_inline void analyse(stereosample_t x) {}

    __force_inline void processAnalysisParams() {}
    
    AudioDriver::codec_config_t getCodecConfig() { return audioAppVerbFX.GetDriverConfig(); }

    void loopCore0() {}

};
