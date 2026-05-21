#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "./AudioApps/SaxFXAudioApp.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>
#include "../src/memllib/audio/AudioDriver.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "../MachineListeningMixin.hpp"



class MEMLNautModeSaxFX {
public:
    constexpr static size_t kN_InputParams = InterfaceRL::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;
    InterfaceRL interface;
    std::shared_ptr<InterfaceRL> interfacePtr;
    MachineListeningMixin mlMixin;

    inline static SaxFXAudioApp<> audioAppSaxFX;
    std::array<String, SaxFXAudioApp<>::nVoiceSpaces> voiceSpaceList;
    std::shared_ptr<MIDIInOut> midi_interf;
    std::shared_ptr<BlockSelectView> enableView;


    void setupInterface() {
        interface.setup(kN_InputParams, SaxFXAudioApp<>::kN_Params);
        interface.setRVX1Override([this](float value) {
            audioAppSaxFX.setWetDryQueued(value);
        });
        interface.bindInterface(MEMLNAUT_INPUT_MODE, JOYSTICK_IS_4D);
        interface.setModeInfo("saxfx", "SaxFX");
        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() {
        return "Sax FX Mode";
    }
    
    __force_inline stereosample_t process(stereosample_t x) {
        return audioAppSaxFX.Process(x);
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
            queue_t& q = audioAppSaxFX.controlMessageQueue;
            switch(id) {
                case 1: { auto msg = SaxFXAudioApp<>::controlMessages::MSG_ENABLE_FILTERBANK;    queue_try_add(&q, &msg); } break;
                case 2: { auto msg = SaxFXAudioApp<>::controlMessages::MSG_ENABLE_REVERB;        queue_try_add(&q, &msg); } break;
                case 3: { auto msg = SaxFXAudioApp<>::controlMessages::MSG_ENABLE_SHORT_DELAY;   queue_try_add(&q, &msg); } break;
                case 4: { auto msg = SaxFXAudioApp<>::controlMessages::MSG_ENABLE_MEDIUM_DELAY;  queue_try_add(&q, &msg); } break;
                case 5: { auto msg = SaxFXAudioApp<>::controlMessages::MSG_ENABLE_LONG_DELAY;    queue_try_add(&q, &msg); } break;
                case 6: { auto msg = SaxFXAudioApp<>::controlMessages::MSG_ENABLE_DELAY_TO_REVERB; queue_try_add(&q, &msg); } break;
            }
        });
        MEMLNaut::Instance()->disp->AddView(enableView);

        std::shared_ptr<VoiceSpaceSelectView> voiceSpaceSelectView;
        voiceSpaceSelectView = std::make_shared<VoiceSpaceSelectView>("Voice Spaces");
        MEMLNaut::Instance()->disp->InsertViewAfter(interface.nnOutputsGraphView, voiceSpaceSelectView);
        voiceSpaceSelectView->setOptions(voiceSpaceList);
        voiceSpaceSelectView->setNewVoiceCallback(
            [this](size_t idx) {
                audioAppSaxFX.setVoiceSpace(idx);
            });
        interface.addInputSourceView();
    };

    void setupAudio(float sample_rate) {
        audioAppSaxFX.Setup(sample_rate, interfacePtr);
        voiceSpaceList = audioAppSaxFX.getVoiceSpaceNames();
        mlMixin.setup(interface);
    }

    __force_inline void loop() {
        audioAppSaxFX.loop();
    }

    __force_inline void analyse(stereosample_t x) { mlMixin.analyse(x); }

    __force_inline void processAnalysisParams() { mlMixin.processAnalysisParams(); }
    
    AudioDriver::codec_config_t getCodecConfig() { return audioAppSaxFX.GetDriverConfig(); }

    void loopCore0() {}

};
