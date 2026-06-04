#pragma once

#include "../src/memllib/hardware/memlnaut/display/RLView.hpp"
#include <functional>
#include <array>
#include <cmath>

class DJRLView : public RLView {
public:
    using BPMCallback = std::function<void(float)>;

    DJRLView(String name, size_t nOutputs, int barwidth = 4,
             int colour = TFT_GREEN, float rangeLow = 0.f, float rangeHigh = 1.f)
        : RLView(name, nOutputs, barwidth, colour, rangeLow, rangeHigh)
    {}

    void setBPMCallback(BPMCallback cb) { bpmCallback_ = cb; }
    float getBPM() const { return bpm_; }

    void OnDraw() override {
        RLView::OnDraw();
        drawTapButton();
    }

    void OnTouchDown(size_t x, size_t y) override {
        if (!inTapArea(x)) return;
        tapPressed_ = true;
        redraw();
    }

    void OnTouchUp(size_t x, size_t y) override {
        if (!inTapArea(x)) return;
        tapPressed_ = false;
        recordTap();
        redraw();
    }

protected:
    rect getBarGraphArea() const override {
        return {area.x, area.y, (area.w * 3) / 4, area.h - kStatusBarHeight};
    }

private:
    static constexpr size_t   kMaxTaps      = 4;
    static constexpr uint32_t kTapTimeoutMs = 2000;

    bool inTapArea(size_t x) const {
        return x >= static_cast<size_t>(area.x + (area.w * 3) / 4);
    }

    void drawTapButton() {
        int bx = area.x + (area.w * 3) / 4;
        int bw = area.w / 4;
        int bh = area.h - kStatusBarHeight;
        uint16_t col = tapPressed_ ? TFT_PURPLE : TFT_GREEN;
        // Draw directly (no large sprite alloc — that failed intermittently and left the
        // button blank). Reliable and cheap, so it repaints fine on every OnDraw.
        scr->fillRect(bx, area.y, bw, bh, col);
        scr->drawRect(bx, area.y, bw, bh, TFT_WHITE);
        scr->setTextFont(2);
        scr->setTextColor(TFT_WHITE, col);
        scr->setTextDatum(TL_DATUM);
        scr->drawString(String((int)roundf(bpm_)) + " bpm", bx + 4, area.y + bh / 2 - 8);
    }

    void recordTap() {
        uint32_t now = millis();
        if (lastTapMs_ > 0 && (now - lastTapMs_) < kTapTimeoutMs) {
            intervals_[tapCount_ % kMaxTaps] = now - lastTapMs_;
            tapCount_++;
            size_t n = std::min(tapCount_, kMaxTaps);
            uint32_t sum = 0;
            for (size_t i = 0; i < n; i++) sum += intervals_[i];
            bpm_ = 60000.f / (sum / static_cast<float>(n));
            bpm_ = fmaxf(20.f, fminf(300.f, bpm_));
        } else {
            tapCount_ = 0;
        }
        lastTapMs_ = now;
        if (bpmCallback_) bpmCallback_(bpm_);
    }

    BPMCallback bpmCallback_;
    float    bpm_       {100.f};
    bool     tapPressed_{false};
    uint32_t lastTapMs_ {0};
    size_t   tapCount_  {0};
    std::array<uint32_t, kMaxTaps> intervals_{};
};
