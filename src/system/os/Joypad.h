#pragma once
#include "utl/Symbol.h"
#include "obj/Msg.h"

enum {
    kNumJoypads = 4,
    kNumPressureButtons = 8,
};

enum JoypadAction {
    kAction_None = 0,
    kAction_Confirm = 1,
    kAction_Cancel = 2,
    kAction_Option = 3,
    kAction_Start = 4,
    kAction_ViewModify = 5,
    kAction_Up = 6,
    kAction_Right = 7,
    kAction_Down = 8,
    kAction_Left = 9,
    kAction_PageUp = 10,
    kAction_PageDown = 11,
    kAction_ExtendedOption1 = 12,
    kAction_ExtendedOption2 = 13,
    kAction_WiiHomeMenu = 14,
    kAction_Option1 = 15,
    kAction_NumActions = 16,
};
enum JoypadButton {
    kPad_L2 = 0,
    kPad_R2 = 1,
    kPad_L1 = 2,
    kPad_R1 = 3,
    kPad_Tri = 4,
    kPad_Circle = 5,
    kPad_X = 6,
    kPad_Square = 7,
    kPad_Select = 8,
    kPad_L3 = 9,
    kPad_R3 = 10,
    kPad_Start = 11,
    kPad_DUp = 12,
    kPad_DRight = 13,
    kPad_DDown = 14,
    kPad_DLeft = 15,
    kPad_LStickUp = 16,
    kPad_LStickRight = 17,
    kPad_LStickDown = 18,
    kPad_LStickLeft = 19,
    kPad_RStickUp = 20,
    kPad_RStickRight = 21,
    kPad_RStickDown = 22,
    kPad_RStickLeft = 23,
    kPad_NumButtons = 24,
    kPad_Xbox_LT = 0,
    kPad_Xbox_RT = 1,
    kPad_Xbox_LB = 2,
    kPad_Xbox_RB = 3,
    kPad_Xbox_Y = 4,
    kPad_Xbox_B = 5,
    kPad_Xbox_A = 6,
    kPad_Xbox_X = 7,
    kPad_Xbox_LS = 9,
    kPad_Xbox_RS = 10,
};

enum JoypadType {
    kJoypadNone = 0,
    kJoypadDigital = 1,
    kJoypadAnalog = 2,
    kJoypadDualShock = 3,
    kJoypadMidi = 4,
    kJoypadXboxHxGuitar = 5,
    kJoypadXboxHxGuitarRb2 = 6,
    kJoypadXboxRoGuitar = 7,
    kJoypadXboxDrums = 8,
    kJoypadXboxDrumsRb2 = 9,
    kJoypadXboxRoDrums = 10,
    kJoypadXboxStageKit = 11,
    kJoypadPs3HxGuitar = 12,
    kJoypadPs3HxGuitarRb2 = 13,
    kJoypadPs3HxDrums = 14,
    kJoypadPs3HxDrumsRb2 = 15,
    kJoypadPs3RoGuitar = 16,
    kJoypadPs3RoDrums = 17,
    kJoypadPs3KonamiDrums = 18,
    kJoypadWiiCore = 19,
    kJoypadWiiFS = 20,
    kJoypadWiiClassic = 21,
    kJoypadWiiGamecube = 22,
    kJoypadWiiGuitar = 23,
    kJoypadWiiDrums = 24,
    kJoypadWiiHxGuitar = 25,
    kJoypadWiiHxGuitarRb2 = 26,
    kJoypadWiiHxDrums = 27,
    kJoypadWiiHxDrumsRb2 = 28,
    kJoypadXboxCoreGuitar = 29,
    kJoypadXboxButtonGuitar = 30,
    kJoypadXboxRealGuitar22Fret = 31,
    kJoypadXboxMidiBoxKeyboard = 32,
    kJoypadXboxMidiBoxDrums = 33,
    kJoypadXboxKeytar = 34,
    kJoypadPs3CoreGuitar = 35,
    kJoypadPs3ButtonGuitar = 36,
    kJoypadPs3RealGuitar22Fret = 37,
    kJoypadPs3MidiBoxKeyboard = 38,
    kJoypadPs3MidiBoxDrums = 39,
    kJoypadPs3Keytar = 40,
    kJoypadWiiCoreGuitar = 41,
    kJoypadWiiButtonGuitar = 42,
    kJoypadWiiRealGuitar22Fret = 43,
    kJoypadWiiMidiBoxKeyboard = 44,
    kJoypadWiiMidiBoxDrums = 45,
    kJoypadWiiKeytar = 46,
    kJoypadNumTypes = 47
};

inline bool MovedLeftStick(JoypadButton btn) {
    return btn == kPad_LStickUp || btn == kPad_LStickRight || btn == kPad_LStickDown
        || btn == kPad_LStickLeft;
}

struct ProGuitarStringInfo {
    bool mDown : 1;
    unsigned char mVelocity : 7;
};

struct ProGuitarData {
    unsigned char mString4FretBottomHalf : 3;
    unsigned char mString5Fret : 5;

    bool unk1upper : 1;
    unsigned char mString3Fret : 5;
    unsigned char mString4FretTopHalf : 2;

    unsigned char mString1FretBottomHalf : 3;
    unsigned char mString2Fret : 5;

    bool unk3upper : 1; // shifted?
    unsigned char mString0Fret : 5;
    unsigned char mString1FretTopHalf : 2;

    union {
        struct {
            bool unk4bool : 1;
            unsigned char mString5Velocity : 7;
            bool unk5bool : 1;
            unsigned char mString4Velocity : 7;
            bool unk6bool : 1;
            unsigned char mString3Velocity : 7;
            bool unk7bool : 1;
            unsigned char mString2Velocity : 7;
            bool unk8bool : 1;
            unsigned char mString1Velocity : 7;
            bool unk9bool : 1;
            unsigned char mString0Velocity : 7;
        };
        struct {
            ProGuitarStringInfo mStringInfos[6];
        };
    };

    bool unkabool : 1;
    unsigned char unkachar : 7;

    bool unkbbool : 1;
    unsigned char unkbchar : 7;

    bool unkcbool : 1;
    unsigned char unkcchar : 7;

    bool mStompBox : 1;
    unsigned char unkdchar : 7;

    bool unkebool : 1;
    unsigned char mMuting : 7;

    unsigned char unkf; // appears to be unused
};

struct ProKeysData {
    unsigned char unk0[8]; // an array of bitfielded uchars? 1 for bool, 7 for actual
                           // uchar data
    // used in GetSlottedKeyVelocityFromExtended

    bool mSustain : 1;
    unsigned char unk8char : 7;

    bool mStompPedal : 1;
    unsigned char mExpressionPedal : 7;

    bool unkabool : 1;
    unsigned char unkachar : 7; // used for both mModVal and accelerometer axis val at
                                // index 0?

    bool unkbbool : 1;
    unsigned char unkbchar : 7;

    bool unkcbool : 1;
    unsigned char unkcchar : 7;

    bool unkdbool : 1;
    unsigned char unkdchar : 7;

    bool unkebool : 1; // bit 7
    unsigned char unkemiddle : 2; // bits 5-6
    unsigned char mLowHandPlacement : 5; // bits 0-4

    unsigned char mConnectedAccessories;
};

class JoypadData {
public:
    unsigned int mButtons; // 0x0
    unsigned int mNewPressed; // 0x4
    unsigned int mNewReleased; // 0x8
    float mSticks[2][2]; // 0xC = LX; 0x10 = LY; 0x14 = RX; 0x18 = RY
    float mTriggers[2]; // 0x1C = LT; 0x20 = RT
    float mSensors[3]; // 0x24 = SX; 0x28 = SY; 0x2C = SZ
    float mPressures[8]; // 0x30 - 0x50

    union {
        ProGuitarData mProGuitarData;
        ProKeysData mProKeysData;
    }; // 0x50

    class LocalUser *mUser; // 0x60
    bool mConnected; // 0x64
    bool mVibrateEnabled; // 0x65

    bool unk66, unk67, unk68;

    bool mHasAnalogSticks; // 0x69
    bool mTranslateSticks; // 0x6a
    int mIgnoreButtonMask; // 0x6c
    int mGreenCymbalMask; // 0x70
    int mYellowCymbalMask; // 0x74
    int mBlueCymbalMask; // 0x78
    int mSecondaryPedalMask; // 0x7c
    int mCymbalMask; // 0x80
    bool mIsDrum; // 0x84
    JoypadType mType; // 0x88
    Symbol mControllerType; // 0x8c
    float mDistFromRest; // 0x90
    bool mHasGreenCymbal; // 0x94
    bool mHasYellowCymbal; // 0x95
    bool mHasBlueCymbal; // 0x96
    bool mHasSecondaryPedal; // 0x97
    int unk98; // 0x98

    JoypadData();
    float GetAxis(Symbol) const;
    int FloatToBucket(float) const;
    int GetVelocityBucket(Symbol) const;
    int GetPressureBucket(JoypadButton) const;
    bool IsButtonInMask(int i) const { return (mButtons & 1 << i) ? true : false; }
    bool IsButtonNewlyPressed(int i) const { return (mNewPressed & 1 << i); }
    void SetButtons(unsigned int btns) {
        unsigned int oldBtns = mButtons;
        unsigned int mask = oldBtns ^ btns;
        mButtons = btns;
        mNewPressed = btns & mask;
        mNewReleased = oldBtns & mask;
    }

    float GetLX() const { return mSticks[0][0]; }
    float GetLY() const { return mSticks[0][1]; }
    float GetRX() const { return mSticks[1][0]; }
    float GetRY() const { return mSticks[1][1]; }
    float GetLT() const { return mTriggers[0]; }
    float GetRT() const { return mTriggers[1]; }
    float GetSX() const { return mSensors[0]; }
    float GetSY() const { return mSensors[1]; }
    float GetSZ() const { return mSensors[2]; }
};

struct WaitInfo {
    WaitInfo(int pad);
    int mPadNum; // 0x0
    unsigned int mButtons; // 0x4
};

class LocalUser; // forward dec

extern "C" bool JoypadIsCalbertGuitar(int);
extern "C" int ButtonToVelocityBucket(JoypadData *, JoypadButton);
extern "C" void JoypadInitCommon(class DataArray *);
extern "C" void AssociateUserAndPad(LocalUser *, int);
extern "C" void ResetAllUsersPads();
extern "C" int GetUsersPadNum(LocalUser *);
extern "C" LocalUser *JoypadGetUserFromPadNum(int);
extern "C" int JoypadGetUsersPadNum(LocalUser *);
extern "C" void JoypadSetCalbertMode(int, int);
extern "C" void JoypadSetActuatorsImp(int, int, int);
extern "C" void JoypadKeepEverythingAlive();
extern "C" void JoypadPollCommon();

void JoypadSetVibrate(int, bool);
Symbol JoypadControllerTypePadNum(int padNum);
bool JoypadIsConnectedPadNum(int);
class JoypadData *JoypadGetPadData(int);
void JoypadReset();
bool JoypadIsControllerTypePadNum(int, Symbol);
bool JoypadTypeHasLeftyFlip(Symbol);
int JoypadTypePadShiftButton(Symbol);
int JoypadTypeCymbalShiftButton(Symbol);
bool JoypadIsShiftButton(int, JoypadButton);
JoypadAction ButtonToAction(JoypadButton, Symbol);
const char *JoypadGetBreedString(int);
float JoypadGetCalbertValue(int, bool);
bool JoypadVibrate(int);
unsigned int JoypadPollForButton(int);
void JoypadPoll();
void JoypadKeepAlive(int, bool);

bool UserHasController(LocalUser *);
bool UserHasGHDrums(LocalUser *);
bool UserHas22FretGuitar(LocalUser *);
bool UserHasButtonGuitar(LocalUser *);

// forward dec
namespace Hmx {
    class Object;
}

void JoypadSubscribe(Hmx::Object *);
void JoypadUnsubscribe(Hmx::Object *);
void JoypadPushThroughMsg(const Message &);
