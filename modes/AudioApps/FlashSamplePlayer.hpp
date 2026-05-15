#ifndef __FLASH_SAMPLE_PLAYER_HPP__
#define __FLASH_SAMPLE_PLAYER_HPP__

#include <cstdint>
#include <cstring>

#define FLASH_AUDIO_ADDRESS  0x10200000U
#define FLASH_AUDIO_MAGIC    0x4F434950U  // 'PICO'

struct FlashSample {
    const float* samples    = nullptr;
    uint32_t     sampleCount = 0;
    float        sampleRate  = 48000.f;
    bool         found       = false;
};

inline FlashSample loadFlashSample(const char* name) {
    FlashSample result{};

    struct Header {
        uint32_t magic;
        uint32_t version;
        uint32_t fileCount;
        uint32_t sampleRate;
    };
    struct FileEntry {
        char     name[16];
        uint32_t offset;
        uint32_t sampleCount;
        float    duration;
        uint32_t reserved;
    };

    const uint8_t* base   = reinterpret_cast<const uint8_t*>(FLASH_AUDIO_ADDRESS);
    const Header*  header = reinterpret_cast<const Header*>(base);

    if (header->magic != FLASH_AUDIO_MAGIC) return result;

    const FileEntry* table = reinterpret_cast<const FileEntry*>(base + 16);
    for (uint32_t i = 0; i < header->fileCount; i++) {
        if (strncmp(table[i].name, name, 16) == 0) {
            result.samples     = reinterpret_cast<const float*>(base + table[i].offset);
            result.sampleCount = table[i].sampleCount;
            result.sampleRate  = static_cast<float>(header->sampleRate);
            result.found       = true;
            return result;
        }
    }
    return result;
}

class BreakbeatPlayer {
public:
    static constexpr size_t kSlices = 16;

    void load(const FlashSample& s) {
        samples     = s.samples;
        sampleCount = s.sampleCount;
        playhead    = 0.f;
    }

    void trigger(float slicePos) {
        if (!samples || sampleCount == 0) return;
        size_t slice = static_cast<size_t>(slicePos * kSlices);
        if (slice >= kSlices) slice = kSlices - 1;
        size_t sliceLen = sampleCount / kSlices;
        playhead = static_cast<float>(slice * sliceLen);
    }

    float process(float rate) {
        if (!samples || sampleCount == 0) return 0.f;
        size_t idx = static_cast<size_t>(playhead);
        if (idx >= sampleCount) idx = sampleCount - 1;
        float out = samples[idx];
        playhead += rate;
        if (playhead >= static_cast<float>(sampleCount))
            playhead -= static_cast<float>(sampleCount);
        return out;
    }

private:
    const float* samples     = nullptr;
    uint32_t     sampleCount = 0;
    float        playhead    = 0.f;
};

#endif  // __FLASH_SAMPLE_PLAYER_HPP__
