#pragma once

#include "../src/memllib/interface/MIDIInOut.hpp"
#include "./AudioApps/SaxFXAudioApp.hpp"
#include "MEMLNautMode.hpp"
#include <memory>
#include "../src/memllib/audio/AudioDriver.hpp"
#include "../src/memllib/examples/InterfaceRL.hpp"
#include "../src/memllib/PicoDefs.hpp"
#include "../MachineListeningMixin.hpp"
#include "../src/memllib/hardware/memlnaut/MEMLNaut.hpp"
#include "../src/memllib/hardware/memlnaut/display/BlockSelectView.hpp"
#include "../src/memllib/hardware/memlnaut/display/VoiceSpaceSelectView.hpp"
#include "../src/memllib/audio/FocusManager.hpp"
#include <array>

class MEMLNautModeSaxFX {
public:
    constexpr static size_t kN_InputParams     = InterfaceRLBase::kMaxNNInputs;
    constexpr static size_t kDesiredSampleRate = 48000;

    // Focus groups: Reverb, Grain timing, Feedback, Pitch, Pitch spread.
    static constexpr size_t kSaxFX_NGroups = 5;

    using InterfaceRL_t = InterfaceRL<SaxFXAudioApp<>::kN_Params>;
    InterfaceRL_t interface;
    std::shared_ptr<InterfaceRL_t> interfacePtr;
    MachineListeningMixin mlMixin;

    inline static SaxFXAudioApp<> audioAppSaxFX;
    std::shared_ptr<MIDIInOut> midi_interf;
    std::shared_ptr<BlockSelectView> enableView;
    std::shared_ptr<VoiceSpaceSelectView> voiceSpaceView;
    std::array<String, SaxFXAudioApp<>::nVoiceSpaces> voiceSpaceList;

    FocusManager<SaxFXAudioApp<>::kN_Params, kSaxFX_NGroups> focusManager;

    void setupInterface() {
        interface.setup(kN_InputParams, SaxFXAudioApp<>::kN_Params);
        interface.bindInterface(MEMLNAUT_INPUT_MODE, JOYSTICK_IS_4D);
        interface.setInputSource(InterfaceRLBase::INPUT_SOURCE::MACHINE_LISTENING);
        interface.setModeInfo("saxfx", "SaxFX");

        interface.setExtraSaveCallback([this]() -> std::vector<uint8_t> {
            std::vector<uint8_t> data(3);
            data[0] = static_cast<uint8_t>(audioAppSaxFX.getVoiceSpace());
            data[1] = audioAppSaxFX.enableReverb ? 1 : 0;
            data[2] = (audioAppSaxFX.enableGrain1 ? 1 : 0)
                    | (audioAppSaxFX.enableGrain2 ? 2 : 0)
                    | (audioAppSaxFX.enableGrain3 ? 4 : 0);
            return data;
        });
        interface.setExtraLoadCallback([this](const uint8_t* data, uint16_t size, uint16_t) {
            if (size >= 1) {
                const size_t idx = data[0];
                audioAppSaxFX.setVoiceSpace(idx);
                if (voiceSpaceView)
                    voiceSpaceView->setSelection(idx);
            }
            if (size >= 2) {
                const bool reverbOn = data[1] != 0;
                audioAppSaxFX.enableReverb = reverbOn;
                if (enableView)
                    enableView->setAltState(0, reverbOn);  // highlight = enabled
            }
            if (size >= 3) {
                const bool g1 = data[2] & 1, g2 = data[2] & 2, g3 = data[2] & 4;
                audioAppSaxFX.enableGrain1 = g1;
                audioAppSaxFX.enableGrain2 = g2;
                audioAppSaxFX.enableGrain3 = g3;
                if (enableView) {
                    enableView->setAltState(1, g1);
                    enableView->setAltState(2, g2);
                    enableView->setAltState(3, g3);
                }
            }
        });

        // Name each focus group (bit index → group).
        focusManager.setGroupName(0, "Rev");   // Reverb
        focusManager.setGroupName(1, "Time");  // Grain timing (length + start, all 3 lines)
        focusManager.setGroupName(2, "Fdbk");  // Feedback
        focusManager.setGroupName(3, "Ptch");  // Pitch
        focusManager.setGroupName(4, "Sprd");  // Pitch spread

        // Assign each param to its group, matching the voice-space param layout.
        // Per grain line: +0 length, +1 start, +2 feedback, +3 pitch, +4 spread, +7 freeze.
        constexpr uint32_t kRev  = 1u << 0;
        constexpr uint32_t kTime = 1u << 1;
        constexpr uint32_t kFdbk = 1u << 2;
        constexpr uint32_t kPtch = 1u << 3;
        constexpr uint32_t kSprd = 1u << 4;
        std::array<uint32_t, SaxFXAudioApp<>::kN_Params> masks = {};
        for (size_t line = 0; line < 3; ++line) {
            const size_t base = line * 8;  // 0, 8, 16
            masks[base + 0] = kTime;       // grain length
            masks[base + 1] = kTime;       // grain start time
            masks[base + 2] = kFdbk;       // feedback
            masks[base + 3] = kPtch;       // pitch
            masks[base + 4] = kSprd;       // pitch spread
            // base+7 (freeze) and unused slots stay ungrouped (latched while focusing).
        }
        for (size_t i = 24; i <= 34; ++i)  // reverb block
            masks[i] = kRev;
        focusManager.setParamGroups(masks);

        // NOTE: no paramTransformHook here. SaxFX runs in MACHINE_LISTENING mode,
        // so the live audio analysis must keep driving ALL params. Focus only steers
        // training/exploration via setActiveDims (see addViews) — it must not latch
        // the live output the way the joystick-driven DJFX mode does.

        interfacePtr = make_non_owning(interface);
    }

    String getHelpTitle() { return "Sax FX Mode"; }

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
        // FX Enable: button 0 = Reverb, 1-3 = Grain delay lines 1-3.
        enableView = std::make_shared<BlockSelectView>(
            "FX Enable", TFT_YELLOW, 4, 70, 60, TFT_BLACK,
            std::vector<String>{ "Reverb", "Gr1", "Gr2", "Gr3" },
            TFT_BLUE, 2);
        enableView->SetOnSelectCallback([this](size_t id) {
            using CM = SaxFXAudioApp<>::controlMessages;
            CM msg;
            switch (id - 1) {
                case 0: msg = CM::MSG_ENABLE_REVERB; break;
                case 1: msg = CM::MSG_ENABLE_GRAIN1; break;
                case 2: msg = CM::MSG_ENABLE_GRAIN2; break;
                case 3: msg = CM::MSG_ENABLE_GRAIN3; break;
                default: return;
            }
            enableView->toggleAlt(id - 1);
            queue_try_add(&audioAppSaxFX.controlMessageQueue, &msg);
        });
        // Highlight = enabled. Init to match the (default-on) state before the view is
        // built (setAltColour is safe pre-OnSetup; setAltState needs the buttons to exist).
        enableView->setAltColour(0, audioAppSaxFX.enableReverb);
        enableView->setAltColour(1, audioAppSaxFX.enableGrain1);
        enableView->setAltColour(2, audioAppSaxFX.enableGrain2);
        enableView->setAltColour(3, audioAppSaxFX.enableGrain3);
        MEMLNaut::Instance()->disp->AddView(enableView);

        voiceSpaceView = std::make_shared<VoiceSpaceSelectView>("Voice Spaces");
        MEMLNaut::Instance()->disp->InsertViewAfter(interface.nnOutputsGraphView, voiceSpaceView);
        voiceSpaceView->setOptions(voiceSpaceList);
        voiceSpaceView->setNewVoiceCallback([this](size_t idx) {
            audioAppSaxFX.setVoiceSpace(idx);
        });

        // Mark which NN output dims are live based on the focused groups.
        auto updateActiveDims = [this]() {
            const uint32_t mask = focusManager.getSelectedMask();
            constexpr size_t N = SaxFXAudioApp<>::kN_Params;
            std::vector<bool> active(N);
            for (size_t i = 0; i < N; i++)
                active[i] = (mask == 0) || ((focusManager.paramGroupMask[i] & mask) != 0);
            interface.setActiveDims(active);
        };
        updateActiveDims();

        // Focus screen — one toggle button per parameter group.
        // 5 buttons => 2 rows x 3 cols. Maximised for the 320x240 screen:
        // width 3*95+30=315<=320, height 2*76+20=172 leaves room for the message line.
        auto focusView = std::make_shared<BlockSelectView>(
            "Focus", TFT_DARKGREY, (int)kSaxFX_NGroups, 95, 76, TFT_WHITE,
            std::vector<String>{ "Rev", "Time", "Fdbk", "Ptch", "Sprd" },
            TFT_GREENYELLOW, 4 /* fontNum */);
        focusView->SetOnSelectCallback([this, focusView, updateActiveDims](size_t id) {
            const size_t groupIdx = id - 1;
            const uint32_t newMask = focusManager.getSelectedMask() ^ (1u << groupIdx);
            focusManager.setFocus(newMask, interface.getLastAction());
            focusView->toggleAlt(groupIdx);
            updateActiveDims();
        });
        MEMLNaut::Instance()->disp->InsertViewAfter(interface.nnOutputsGraphView, focusView);
    }

    void setupAudio(float sample_rate) {
        audioAppSaxFX.Setup(sample_rate, interfacePtr);
        voiceSpaceList = audioAppSaxFX.getVoiceSpaceNames();
        mlMixin.setup(interface);
    }

    __force_inline void loop() { audioAppSaxFX.loop(); }

    __force_inline void analyse(stereosample_t x)    { mlMixin.analyse(x); }
    __force_inline void processAnalysisParams()       { mlMixin.processAnalysisParams(); }

    AudioDriver::codec_config_t getCodecConfig() { return audioAppSaxFX.GetDriverConfig(); }

    void loopCore0() {}
};
