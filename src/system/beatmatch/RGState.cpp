#include "beatmatch/RGState.h"
#include "beatmatch/RGUtl.h"
#include "os/Debug.h"
#include <string.h>

#define kNumRGFrets 24

RGState::RGState() { memset(this, 0, sizeof(RGState)); }

void RGState::FretDown(int string, int fret) {
    MILO_ASSERT(string < kNumRGStrings, 0x14);
    MILO_ASSERT(fret < kNumRGFrets, 0x15);
    mStrings[string] = fret;
}

void RGState::FretUp(int string, int fret) {
    MILO_ASSERT(string < kNumRGStrings, 0x1C);
    MILO_ASSERT(fret < kNumRGFrets, 0x1D);
    mStrings[string] = 0;
}

unsigned int RGState::GetFret(int string) const {
    MILO_ASSERT(string < kNumRGStrings, 0x24);
    return mStrings[string];
}

bool RGState::operator!=(const RGState &state) const {
    return memcmp(this, &state, sizeof(RGState)) != 0;
}

RGState &RGState::operator=(const RGState &state) {
    for (int i = 0; i < 6; i++) {
        mStrings[i] = state.mStrings[i];
    }
    return *this;
}

unsigned int PackRGData(int string, int fret) { return (string << 0x10) | fret; }

void UnpackRGData(unsigned int packed, int &string, int &fret) {
    string = (int)(packed & 0xFFFF0000) >> 0x10;
    fret = (int)packed & 0xFFFF;
}
