#pragma once

#include "../src/memllib/hardware/memlnaut/display/View.hpp"

class DJFXEnableView : public ViewBase {
public:
    static constexpr size_t kNFX   = 9;
    static constexpr size_t kNCols = 4;
    static constexpr size_t kNRows = (kNFX + kNCols - 1) / kNCols;  // 3 rows, last row has 1 cell

    DJFXEnableView(String name, volatile uint32_t* mask)
        : ViewBase(name), mask_(mask) {}

    void OnSetup() override {}

    void OnDraw() override {
        scr->fillRect(area.x, area.y, area.w, area.h, TFT_BLACK);
        for (int r = 0; r < (int)kNRows; ++r)
            for (int c = 0; c < (int)kNCols; ++c)
                if ((size_t)r * kNCols + (size_t)c < kNFX)
                    drawCell(c, r);
    }

    void OnTouchDown(size_t x, size_t y) override {
        if ((int)x < area.x || (int)y < area.y) return;
        const int col = ((int)x - area.x) / cellW();
        const int row = ((int)y - area.y) / cellH();
        if (col >= (int)kNCols || row >= (int)kNRows) return;
        const size_t idx = (size_t)row * kNCols + (size_t)col;
        if (idx >= kNFX) return;
        *mask_ ^= (1u << idx);
        drawCell(col, row);
    }

private:
    static const char* fxName(size_t idx) {
        static const char* kNames[kNFX] = {
            "Allpass", "Flanger", "Chorus",  "Grain 1",
            "Grain 2", "Delay",   "Downsamp","Stutter",
            "Ring Mod"
        };
        return idx < kNFX ? kNames[idx] : "";
    }

    int cellW() const { return area.w / (int)kNCols; }
    int cellH() const { return area.h / (int)kNRows; }

    void drawCell(int col, int row) {
        const size_t idx = (size_t)row * kNCols + (size_t)col;
        const bool on = (*mask_ >> idx) & 1u;
        const int x = area.x + col * cellW();
        const int y = area.y + row * cellH();
        const int w = cellW();
        const int h = cellH();

        const uint16_t bg  = on ? scr->color565(0, 100, 0)    : scr->color565(30, 30, 30);
        const uint16_t fg  = on ? TFT_WHITE                    : scr->color565(120, 120, 120);
        const uint16_t brd = on ? TFT_GREEN                    : scr->color565(60, 60, 60);

        scr->fillRect(x + 1, y + 1, w - 2, h - 2, bg);
        scr->drawRect(x, y, w, h, brd);
        scr->setTextFont(2);
        scr->setTextColor(fg, bg);
        scr->setTextDatum(MC_DATUM);
        scr->drawString(fxName(idx), x + w / 2, y + h / 2);
        scr->setTextDatum(TL_DATUM);
    }

    volatile uint32_t* mask_;
};
