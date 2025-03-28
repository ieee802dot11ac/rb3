#include "os/HolmesKeyboard.h"
#include "os/PlatformMgr.h"
#include "os/Keyboard.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"

HolmesInput::HolmesInput(CWnd *cwnd) {
    mJoypadBuffer = new MemStream(true);
    mKeyboardBuffer = new MemStream(true);
    mOwner = cwnd;
}

HolmesInput::~HolmesInput() {
    delete mKeyboardBuffer;
    mKeyboardBuffer = 0;
    delete mJoypadBuffer;
    mJoypadBuffer = 0;
}

void HolmesInput::LoadKeyboard(BinStream &bs) {
    int asdf;
    mKeyboardBuffer->Seek(0, BinStream::kSeekEnd);
    bs >> asdf;
    if (0 < asdf) {
        mKeyboardBuffer->WriteStream(bs, asdf);
    }
}

void HolmesInput::LoadJoypad(BinStream &bs) {
    int asdf;
    mKeyboardBuffer->Seek(0, BinStream::kSeekEnd);
    bs >> asdf;
    if (0 < asdf) {
        mJoypadBuffer->WriteStream(bs, asdf);
    }
}

void HolmesInput::SendKeyboardMessages() {
    mKeyboardBuffer->Seek(0, BinStream::kSeekBegin);
    bool bbb;
    while (!mKeyboardBuffer->Eof()) {
        bbb = ThePlatformMgr.mScreenSaver;
        ThePlatformMgr.SetScreenSaver(false);
        ThePlatformMgr.SetScreenSaver(bbb);
        int i;
        bool b1, b2, b3;
        *mKeyboardBuffer >> i;
        *mKeyboardBuffer >> b1;
        *mKeyboardBuffer >> b2;
        *mKeyboardBuffer >> b3;
        KeyboardSendMsg(i, b1, b2, b3);
    }
    mKeyboardBuffer->Compact();
}

int HolmesInput::SendJoypadMessages() {
    static DataNode &fake_controllers = DataVariable("fake_controllers");
    int mask = 0;
    mJoypadBuffer->Seek(0, BinStream::kSeekBegin);
    while (!mJoypadBuffer->Eof()) {
        fake_controllers = 1;
        bool bbb = ThePlatformMgr.mScreenSaver;
        ThePlatformMgr.SetScreenSaver(false);
        ThePlatformMgr.SetScreenSaver(bbb);
        int up;
        JoypadButton btn;
        JoypadAction action;
        *mJoypadBuffer >> up;
        *mJoypadBuffer >> (int &)btn;
        *mJoypadBuffer >> (int &)action;
        JoypadData *jdata = JoypadGetPadData(0);
        if (!up) {
            mask |= 1 << btn;
            JoypadPushThroughMsg(ButtonDownMsg(jdata->mUser, btn, action, 0));
        } else
            JoypadPushThroughMsg(ButtonUpMsg(jdata->mUser, btn, action, 0));
    }
    mJoypadBuffer->Compact();
    return mask;
}

BEGIN_HANDLERS(HolmesInput)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_CHECK(0xC9)
END_HANDLERS