#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "./AudioApps/DJFXAudioApp.hpp"
#include "./DJRLView.hpp"
#include "./DJFXEnableView.hpp"
#include "../src/memllib/hardware/memlnaut/MEMLNaut.hpp"
#include "../src/memllib/audio/FocusManager.hpp"
#include "../src/memllib/hardware/memlnaut/display/BlockSelectView.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include <array>
#include "../src/memllib/audio/AudioDriver.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "../MachineListeningMixin.hpp"


// One focus group per effect — 10 groups, matching the param layout in DJFXAudioApp.
static constexpr size_t kDJFX_NGroups = 10;

class MEMLNautModeDJFX {
public:
    constexpr static size_t kN_InputParams = InterfaceRLBase::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;
    using InterfaceRL_t = InterfaceRL<DJFXAudioApp<>::kN_Params>;
    InterfaceRL_t interface;
    std::shared_ptr<InterfaceRL_t> interfacePtr;
    MachineListeningMixin mlMixin;

    inline static DJFXAudioApp<> audioAppDJFX;
    std::shared_ptr<MIDIInOut> midi_interf;

    FocusManager<DJFXAudioApp<>::kN_Params, kDJFX_NGroups> focusManager;


    void setupInterface() {
        auto djView = std::make_shared<DJRLView>("RL", DJFXAudioApp<>::kN_Params);
        djView->setBPMCallback([this](float bpm) {
            audioAppDJFX.setBPMQueued(bpm);
        });
        interface.nnOutputsGraphView = djView;

        interface.setup(kN_InputParams, DJFXAudioApp<>::kN_Params);
        interface.setRVX1Override([this](float value) {
            audioAppDJFX.setRVX1Queued(value);
        });
        interface.bindInterface(MEMLNAUT_INPUT_MODE, JOYSTICK_IS_4D);
        interface.setModeInfo("djfx", "DJFX");

        interface.setExtraSaveCallback([this]() -> std::vector<uint8_t> {
            std::vector<uint8_t> data(4);
            const uint32_t mask = audioAppDJFX.enableMask_;
            memcpy(data.data(), &mask, 4);
            return data;
        });
        interface.setExtraLoadCallback([this](const uint8_t* data, uint16_t size, uint16_t) {
            if (size >= 4) {
                uint32_t mask;
                memcpy(&mask, data, 4);
                audioAppDJFX.enableMask_ = mask;
            }
        });

        interfacePtr = make_non_owning(interface);

        // Name each focus group.
        focusManager.setGroupName(0, "Gr1");
        focusManager.setGroupName(1, "Gr2");
        focusManager.setGroupName(2, "Dly");
        focusManager.setGroupName(3, "Flgr");
        focusManager.setGroupName(4, "Chr");
        focusManager.setGroupName(5, "AP");
        focusManager.setGroupName(6, "Dsp");
        focusManager.setGroupName(7, "Stt");
        focusManager.setGroupName(8, "Rng");
        focusManager.setGroupName(9, "Rev");

        // Assign each of the 57 params to its group (matching ProcessParams layout).
        std::array<uint32_t, DJFXAudioApp<>::kN_Params> masks = {
            // 0-7: Grain Delay 1
            1u<<0, 1u<<0, 1u<<0, 1u<<0, 1u<<0, 1u<<0, 1u<<0, 1u<<0,
            // 8-15: Grain Delay 2
            1u<<1, 1u<<1, 1u<<1, 1u<<1, 1u<<1, 1u<<1, 1u<<1, 1u<<1,
            // 16-17: Grain crossfade and mix (Gr1 group)
            1u<<0, 1u<<0,
            // 18-20: Dynamic Delay
            1u<<2, 1u<<2, 1u<<2,
            // 21-25: Flanger
            1u<<3, 1u<<3, 1u<<3, 1u<<3, 1u<<3,
            // 26-30: Chorus
            1u<<4, 1u<<4, 1u<<4, 1u<<4, 1u<<4,
            // 31-35: Allpass + LFO
            1u<<5, 1u<<5, 1u<<5, 1u<<5, 1u<<5,
            // 36-37: Downsampler
            1u<<6, 1u<<6,
            // 38-40: Stutter Gate
            1u<<7, 1u<<7, 1u<<7,
            // 41-42: Ring Mod
            1u<<8, 1u<<8,
            // 43-45: Delay feedback bandpass (freq, Q, mix)
            1u<<2, 1u<<2, 1u<<2,
            // 46-56: Reverb
            1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9, 1u<<9
        };
        focusManager.setParamGroups(masks);

        interface.paramTransformHook = [this](std::vector<float>& p) {
            focusManager.applyInPlace(p);
        };

    }

    String getHelpTitle() {
        return "DJ FX Mode";
    }

    __force_inline stereosample_t process(stereosample_t x) {
        return audioAppDJFX.Process(x);
    }

    void setupMIDI(std::shared_ptr<MIDIInOut> new_midi_interf) {
        midi_interf = new_midi_interf;
        midi_interf->Setup(0);
        midi_interf->SetMIDISendChannel(1);
        interface.bindMIDI(midi_interf, true);
    }

    void addViews() {
        // Lambda to mark active NN dimensions based on which groups are focused.
        auto updateActiveDims = [this]() {
            const uint32_t mask = focusManager.getSelectedMask();
            constexpr size_t N = DJFXAudioApp<>::kN_Params;
            std::vector<bool> active(N);
            for (size_t i = 0; i < N; i++)
                active[i] = (mask == 0) || ((focusManager.paramGroupMask[i] & mask) != 0);
            interface.setActiveDims(active);
        };
        updateActiveDims();

        // Focus view — 10 toggle buttons, one per effect group.
        // 10 buttons => 2 rows x 5 cols. Maximised for 320x240: width 5*53+50=315<=319,
        // height 2*76+20=172 leaves room for the message line.
        auto focusView = std::make_shared<BlockSelectView>(
            "Focus", TFT_DARKGREY, (int)kDJFX_NGroups, 53, 76, TFT_WHITE,
            std::vector<String>{"Gr1","Gr2","Dly","Flgr","Chr","AP","Dsp","Stt","Rng","Rev"},
            TFT_GREENYELLOW, 2 /* fontNum */);

        focusView->SetOnSelectCallback([this, focusView, updateActiveDims](size_t id) {
            const size_t groupIdx = id - 1;
            const uint32_t newMask = focusManager.getSelectedMask() ^ (1u << groupIdx);
            focusManager.setFocus(newMask, interface.getLastAction());
            focusView->toggleAlt(groupIdx);
            updateActiveDims();
        });
        MEMLNaut::Instance()->disp->InsertViewAfter(interface.nnOutputsGraphView, focusView);

        interface.addInputSourceView(false);  // no MIDI CC Out screen for this mode

        auto enableView = std::make_shared<DJFXEnableView>("FX Enable", &audioAppDJFX.enableMask_);
        MEMLNaut::Instance()->disp->AddView(enableView);
    };

    void setupAudio(float sample_rate) {
        audioAppDJFX.Setup(sample_rate, interfacePtr);
        mlMixin.setup(interface);
    }

    __force_inline void loop() {
        audioAppDJFX.loop();
        const auto src = interface.getInputSource();
        if (src == InterfaceRLBase::INPUT_SOURCE::JOYSTICK_3D ||
            src == InterfaceRLBase::INPUT_SOURCE::JOYSTICK_4D) {
            const auto& inputs = interface.getControlInput();
            const size_t n = interface.getActiveInputCount();
            float maxDev = 0.f;
            for (size_t i = 0; i < n && i < inputs.size(); ++i) {
                const float d = fabsf(inputs[i] - 0.5f);
                if (d > maxDev) maxDev = d;
            }
            audioAppDJFX.setWetDryQueued(fminf(maxDev * 2.f, 1.f));
        }
    }

    __force_inline void analyse(stereosample_t x) { mlMixin.analyse(x); }

    __force_inline void processAnalysisParams() { mlMixin.processAnalysisParams(); }

    AudioDriver::codec_config_t getCodecConfig() { return audioAppDJFX.GetDriverConfig(); }

    void loopCore0() {}

};
