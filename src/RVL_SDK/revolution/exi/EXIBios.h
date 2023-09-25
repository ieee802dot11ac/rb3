#ifndef RVL_SDK_EXI_BIOS_H
#define RVL_SDK_EXI_BIOS_H
#include "revolution/exi/EXICommon.h"
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    EXI_FREQ_1MHZ,
    EXI_FREQ_2MHZ,
    EXI_FREQ_4MHZ,
    EXI_FREQ_8MHZ,
    EXI_FREQ_16MHZ,
    EXI_FREQ_32HZ,
} EXIFreq;

typedef struct EXIItem {
    u32 dev;              // at 0x0
    EXICallback callback; // at 0x4
} EXIItem;

typedef struct EXIData {
    EXICallback exiCallback; // at 0x0
    EXICallback tcCallback;  // at 0x4
    EXICallback extCallback; // at 0x8
    volatile s32 state;      // at 0xC
    s32 bytesRead;           // at 0x10
    void* buffer;            // at 0x14
    u32 dev;                 // at 0x18
    u32 id;                  // at 0x1C
    u32 WORD_0x20;
    s32 numItems;     // at 0x24
    EXIItem items[3]; // at 0x28
} EXIData;

bool32 EXIImm(EXIChannel chan, void* buf, s32 len, u32 type,
            EXICallback callback);
bool32 EXIImmEx(EXIChannel chan, void* buf, s32 len, u32 type);
bool32 EXIDma(EXIChannel chan, void* buf, s32 len, u32 type,
            EXICallback callback);
bool32 EXISync(EXIChannel chan);
void EXIClearInterrupts(EXIChannel chan, bool32 exi, bool32 tc, bool32 ext);
EXICallback EXISetExiCallback(EXIChannel chan, EXICallback callback);
void EXIProbeReset(void);
bool32 EXIProbe(EXIChannel chan);
bool32 EXIAttach(EXIChannel chan, EXICallback callback);
bool32 EXIDetach(EXIChannel chan);
bool32 EXISelect(EXIChannel chan, u32 dev, u32 freq);
bool32 EXIDeselect(EXIChannel chan);
void EXIInit(void);
bool32 EXILock(EXIChannel chan, u32 dev, EXICallback callback);
bool32 EXIUnlock(EXIChannel chan);
s32 EXIGetID(EXIChannel chan, u32 dev, u32* out);

#ifdef __cplusplus
}
#endif
#endif