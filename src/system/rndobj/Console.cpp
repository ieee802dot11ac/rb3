#include "rndobj/Console.h"
#include "obj/DataFile.h"
#include "obj/DataUtl.h"
#include "obj/ObjMacros.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/File.h"
#include "os/HolmesClient.h"
#include "os/Keyboard.h"
#include "os/System.h"
#include "rndobj/Overlay.h"
#include "rndobj/Rnd.h"
#include "utl/Cheats.h"
#include "utl/Std.h"
#include <string.h>

static RndConsole *gConsole;

static DataNode DataContinue(DataArray *) {
    gConsole->Continue();
    return 0;
}

static DataNode DataBreak(DataArray *da) {
    gConsole->Break(da);
    return 0;
}

static DataNode DataStep(DataArray *) {
    gConsole->Step(0);
    return 0;
}

static DataNode DataNext(DataArray *) {
    gConsole->Step(-1);
    return 0;
}

static DataNode DataFinish(DataArray *) {
    gConsole->Step(-2);
    return 0;
}

static DataNode DataList(DataArray *) {
    gConsole->List();
    return 0;
}

static DataNode DataWhere(DataArray *) {
    gConsole->Where();
    return 0;
}

static DataNode DataSetBreak(DataArray *da) {
    gConsole->SetBreak(da);
    return 0;
}

static DataNode DataBreakpoints(DataArray *) {
    gConsole->Breakpoints();
    return 0;
}

static DataNode DataClear(DataArray *da) {
    int clearInt;
    if (da->Size() < 2)
        clearInt = 0;
    else if (da->Type(1) == kDataSymbol && da->Sym(1) == "all")
        clearInt = -1;
    else
        clearInt = da->Int(1);
    gConsole->Clear(clearInt);
    return 0;
}

static DataNode DataNop(DataArray *) { return 0; }

static DataNode DataUp(DataArray *) {
    gConsole->MoveLevel(-1);
    return 0;
}

static DataNode DataDown(DataArray *) {
    gConsole->MoveLevel(1);
    return 0;
}

static DataNode DataCppBreak(DataArray *) {
    PlatformDebugBreak();
    return 0;
}

static DataNode DataHelp(DataArray *da) {
    gConsole->Help(da->Size() > 1 ? da->Sym(1) : "");
    return 0;
}

RndConsole::Breakpoint::~Breakpoint() {
    if (parent) {
        DataArray *cmd = parent->Command(index);
        cmd->Node(0) = DataNop;
    }
}

void RndConsole::MoveLevel(int level) {
    if (mDebugging) {
        mLevel += level;
        int callstackIdx = gCallStack - gCallStackPtr;
        ClampEq(mLevel, callstackIdx + 2, 0);
        mDebugging = gCallStackPtr[mLevel - 2];
        List();
    } else
        MILO_FAIL("Can't move level unless debugging");
}

void RndConsole::InsertBreak(DataArray *arr, int i) {
    DataArray *localArr = new DataArray(1);
    localArr->Node(0) = DataBreak;
    DataArray *cmd = arr->Command(i);
    localArr->SetFileLine(cmd->File(), cmd->Line());
    arr->Insert(i, DataNode(localArr, kDataCommand));
    localArr->Release();
    mBreakpoints.push_back(Breakpoint());
    mBreakpoints.back().parent = arr;
    mBreakpoints.back().index = i;
}

void RndConsole::SetBreak(DataArray *arr) {
    if (arr->Size() > 1) {
        DataArray *arr9 = nullptr;
        if (arr->Size() > 2) {
            Hmx::Object *myObj = arr->Obj<Hmx::Object>(1);
            if (myObj) {
                if (myObj->TypeDef()) {
                    arr9 = myObj->TypeDef()->FindArray(arr->Sym(2));
                }
            } else if (arr->Size() > 3) {
                arr9 = SystemConfig(
                    "objects", arr->Sym(1), "types", arr->Sym(2), arr->Sym(3)
                );
            }
        } else if (DataThis()->TypeDef()) {
            arr9 = DataThis()->TypeDef()->FindArray(arr->Sym(1));
        }
        if (arr9) {
            for (int i = 0; i < arr9->Size(); i++) {
                if (arr9->Type(i) == kDataCommand) {
                    InsertBreak(arr9, i);
                    return;
                }
            }
        }
    } else if (gCallStackPtr - gCallStack > 1) {
        DataArray *arr9 = gCallStackPtr[-3];
        DataArray *arr7 = gCallStackPtr[-2];
        for (int i = 0; i < arr9->Size(); i++) {
            if (arr9->Type(i) == kDataCommand && arr9->UncheckedArray(i) == arr7) {
                InsertBreak(arr9, i);
                return;
            }
        }
    }
    MILO_FAIL("Can't insert break");
}

void RndConsole::Clear(int iii) {
    if (iii > 0) {
        FOREACH (it, mBreakpoints) {
            iii--;
            if (iii == 0) {
                mBreakpoints.erase(it);
                return;
            }
        }
    } else if (iii == 0) {
        FOREACH (it, mBreakpoints) {
            if ((*it).parent->UncheckedArray((*it).index) == mDebugging) {
                mBreakpoints.erase(it);
                return;
            }
        }
    } else {
        mBreakpoints.clear();
        return;
    }
    MILO_FAIL("Couldn't clear breakpoint");
}

void RndConsole::Breakpoints() {
    mOutput->Clear();
    int idx = 1;
    FOREACH (it, mBreakpoints) {
        DataArray *cmd = (*it).parent->Command((*it).index);
        MILO_LOG("%d. %s:%d\n", idx++, cmd->File(), cmd->Line());
    }
}

void RndConsole::Break(DataArray *arr) {
    if (mDebugging)
        MILO_FAIL("Can't break while debugging, did you mean set_break?");
    if (arr->UncheckedFunc(0) != DataNop) {
        bool drawing = TheRnd->Drawing();
        bool showing = mShowing;
        Hmx::Color oldClear = TheRnd->mClearColor;
        if (drawing) {
            TheRnd->EndDrawing();
        }
        if (!showing) {
            SetShowing(true);
        }
        TheRnd->SetClearColor(Hmx::Color(0, 0, 0));
        mDebugging = arr;
        mLevel = 0;
        gPreExecuteFunc = nullptr;
        mInput->Clear();
        if (arr->UncheckedFunc(0) == DataBreak) {
            *mInput << "Break at ";
        } else
            *mInput << "Step to ";
        *mInput << arr->File() << ":" << arr->Line() << "\n";
        List();
        while (mDebugging) {
            KeyboardPoll();
            TheRnd->BeginDrawing();
            TheRnd->EndDrawing();
        }
        mOutput->Clear();
        TheRnd->SetClearColor(oldClear);
        if (!showing)
            SetShowing(false);
        if (drawing)
            TheRnd->BeginDrawing();
    }
}

void RndConsole::List() {
    if (mDebugging) {
        mOutput->Clear();
        File *f = NewFile(mDebugging->File(), 2);
        if (f) {
            int i = 1;
            int numlines = mOutput->NumLines() / 2;
            int i4 = mDebugging->Line() - numlines;
            int totalLines = numlines + mDebugging->Line();
            do {
                char buf;
                int read = f->Read(&buf, 1);
                if (i > i4) {
                    *mOutput << buf;
                }
                if (buf == '\n' || read == 0) {
                    i++;
                    if (i > i4 && i < totalLines) {
                        *mOutput << MakeString(
                            "%3d%c", i, i == mDebugging->Line() ? '>' : ':'
                        );
                    }
                }
            } while (i < totalLines);
            delete f;
        }
    } else
        MILO_FAIL("Can't list unless debugging");
}

void RndConsole::Where() {
    if (mDebugging) {
        mOutput->Clear();
        for (DataArray **it = &gCallStackPtr[-2]; it >= gCallStack; it--) {
            if (*it != mDebugging) {
                MILO_LOG("  ");
            }
            MILO_LOG("%s:%d\n", (*it)->File(), (*it)->Line());
        }
    } else
        MILO_FAIL("Can't where unless debugging");
}

void RndConsole::Step(int i) {
    if (mDebugging) {
        mDebugging = nullptr;
        gPreExecuteFunc = *DataBreak;
        int idx = gCallStackPtr - gCallStack;
        gPreExecuteLevel = idx + mLevel + i;
    } else
        MILO_FAIL("Can't step unless debugging");
}

void RndConsole::Continue() {
    if (mDebugging)
        mDebugging = nullptr;
    else
        MILO_FAIL("Can't continue unless debugging");
}

void RndConsole::Help(Symbol sym) {
    if (mDebugging) {
        mOutput->Clear();
        if (sym == "step")
            *mOutput << "Step into command\n";
        else if (sym == "next")
            *mOutput << "Step over command\n";
        else if (sym == "finish")
            *mOutput << "Finish executing current scope\n";
        else if (sym == "continue")
            *mOutput << "Continue program execution\n";
        else if (sym == "list")
            *mOutput << "List file around current line\n";
        else if (sym == "where") {
            *mOutput << "List script stack\n\n";
            *mOutput << "The current line is outdented\n";
        } else if (sym == "up")
            *mOutput << "Move up script stack\n";
        else if (sym == "down")
            *mOutput << "Move down script stack\n";
        else if (sym == "set_break") {
            *mOutput << "Set dynamic breakpoint\n\n";
            *mOutput << "set_break - current line\n";
            *mOutput << "set_break <handler> - in current object\n";
            *mOutput << "set_break <obj> <handler> - in given object\n";
            *mOutput << "set_break <class> <type> <handler> - in given class and type\n";
        } else if (sym == "breakpoints")
            *mOutput << "List dynamic breakpoints\n";
        else if (sym == "clear") {
            *mOutput << "Clear dynamic breakpoint\n\n";
            *mOutput << "clear - current line\n";
            *mOutput << "clear <n> - numbered breakpoint\n";
            *mOutput << "clear all - all breakpoints\n";
        } else if (sym == "cppbreak")
            *mOutput << "Break into the C++ debugger.\n";
        else {
            *mOutput << "Help on debug commands:\n";
            *mOutput << "help step\n";
            *mOutput << "help next\n";
            *mOutput << "help finish\n";
            *mOutput << "help continue\n";
            *mOutput << "help list\n";
            *mOutput << "help where\n";
            *mOutput << "help up\n";
            *mOutput << "help down\n";
            *mOutput << "help set_break\n";
            *mOutput << "help breakpoints\n";
            *mOutput << "help clear\n\n";
            *mOutput << "help cppbreak\n\n";
            *mOutput << "eval <$var> - return variable value\n";
            *mOutput << "eval [<prop>] - return property value\n";
            *mOutput << "set <$var> <value> - set variable value\n";
            *mOutput << "set [<prop>] <value> - set property value\n";
            *mOutput << "<func> <args> - call script function\n";
            *mOutput << "<obj> <handler> <args> - call object handler\n";
        }
    } else
        MILO_FAIL("Can't help unless debugging");
}

RndConsole::RndConsole()
    : mShowing(0), mBuffer(), mTabLen(0), mCursor(0), mPumpMsgs(0), mDebugging(0),
      mLevel(0) {
    gConsole = this;
    mBufPtr = mBuffer.begin();
    mOutput = RndOverlay::Find("output", true);
    mInput = RndOverlay::Find("input", true);
    DataArray *rndCfg = SystemConfig("rnd");
    rndCfg->FindData("console_buffer", mMaxBuffer, true);
    DataRegisterFunc("break", DataBreak);
    DataRegisterFunc("continue", DataContinue);
    DataRegisterFunc("step", DataStep);
    DataRegisterFunc("next", DataNext);
    DataRegisterFunc("finish", DataFinish);
    DataRegisterFunc("list", DataList);
    DataRegisterFunc("where", DataWhere);
    DataRegisterFunc("set_break", DataSetBreak);
    DataRegisterFunc("clear", DataClear);
    DataRegisterFunc("breakpoints", DataBreakpoints);
    DataRegisterFunc("up", DataUp);
    DataRegisterFunc("down", DataDown);
    DataRegisterFunc("cppbreak", DataCppBreak);
    DataRegisterFunc("help", DataHelp);
}

RndConsole::~RndConsole() { TheDebug.SetReflect(0); }

void RndConsole::SetShowing(bool show) {
#ifdef MILO_DEBUG
    if (mShowing != show) {
        if (show) {
            mInput->Clear();
            mKeyboardOverride = KeyboardOverride(this);
            TheDebug.SetReflect(mOutput);
        } else {
            KeyboardOverride(mKeyboardOverride);
            TheDebug.SetReflect(nullptr);
            mDebugging = nullptr;
        }
        mShowing = show;
        mOutput->SetShowing(show);
        mInput->SetShowing(show);
        Message msg("rnd_console_showing", show);
        HolmesClientSendMessage(msg);
    }
#endif
}

BEGIN_HANDLERS(RndConsole)
    HANDLE_MESSAGE(KeyboardKeyMsg)
    HANDLE_CHECK(592)
END_HANDLERS

int RndConsole::OnMsg(const KeyboardKeyMsg &msg) {
    if (!mShowing)
        return 0;
    if (msg.GetKey() == 0x12E) {
#ifdef MILO_DEBUG
        SetShowing(false);
#endif
    } else if (msg.GetKey() == 9) {
        if (mTabLen == 0)
            mTabLen = mInput->CurrentLine().length();
        if (!mBuffer.empty()) {
            if (mBufPtr == mBuffer.end()) {
                mBufPtr = mBuffer.begin();
            }
            do {
                ++mBufPtr;
                if (mBufPtr == mBuffer.end()) {
                    mBufPtr = mBuffer.begin();
                }
                if (strncmp(mInput->CurrentLine().c_str(), (*mBufPtr).c_str(), mTabLen)
                    == 0) {
                    mInput->CurrentLine() = *mBufPtr;
                    break;
                }
            } while (mBufPtr != mBuffer.end());
        }
        MinEq<int>(mCursor, mInput->CurrentLine().length());
    } else if (msg.GetKey() == 0x142) {
        if (!mBuffer.empty()) {
            if (mBufPtr != mBuffer.end()) {
                ++mBufPtr;
            }
            if (mBufPtr == mBuffer.end()) {
                mBufPtr = mBuffer.begin();
            }
            mInput->CurrentLine() = *mBufPtr;
        }
        mCursor = mInput->CurrentLine().length();
    } else if (msg.GetKey() == 0x143) {
        if (!mBuffer.empty()) {
            if (mBufPtr != mBuffer.end()) {
                --mBufPtr;
            } else
                mBufPtr = mBuffer.begin();
            if (mBufPtr == mBuffer.begin()) {
                mBufPtr = PrevItr(mBuffer.begin(), 1);
            }
            mInput->CurrentLine() = *mBufPtr;
        }
        MinEq<int>(mCursor, mInput->CurrentLine().length());
    } else if (msg.GetKey() == 8) {
        String &curLine = mInput->CurrentLine();
        if (mCursor != 0) {
            mCursor--;
            curLine.erase(mCursor, 1);
        }
    } else if (msg.GetKey() == 0x137) {
        String &curLine = mInput->CurrentLine();
        if (mCursor < mInput->CurrentLine().length()) {
            curLine.erase(mCursor, 1);
        }
    } else if (msg.GetKey() == 0x140) {
        mCursor = Max(mCursor - 1, 0);
    } else if (msg.GetKey() == 0x141) {
        mCursor = Min<int>(mCursor + 1, mInput->CurrentLine().length());
    } else if (msg.GetKey() == 0x139) {
        mCursor = mInput->CurrentLine().length();
    } else if (msg.GetKey() == 0x138) {
        mCursor = 0;
    } else if (msg.GetKey() == 10) {
        mCursor = 0;
        MILO_TRY { ExecuteLine(); }
        MILO_CATCH(errMsg) {
            mInput->CurrentLine().erase();
            *mInput << errMsg << "\n";
            MILO_LOG("%s\n", errMsg);
        }
    } else if (msg.GetKey() == 0x7D) {
        if (msg.GetCtrl()) {
            String &curLine = mInput->CurrentLine();
            curLine.insert(0, 1, '{');
            curLine.insert(curLine.length(), "} ");
            mCursor = curLine.length();
        } else {
            char buf[2] = { '\0', '\0' };
            buf[0] = msg.GetKey();
            if (mCursor > mInput->CurrentLine().length()) {
                mCursor = mInput->CurrentLine().length();
            }
            mInput->CurrentLine().insert(mCursor, buf);
            mCursor++;
        }
    }
    mInput->SetCursorChar(msg.GetKey() == 10 ? -1 : mCursor);
    if (msg.GetKey() != 9) {
        mTabLen = 0;
    }
    return 0;
}

void RndConsole::ExecuteLine() {
    String &line_txt = mInput->CurrentLine();
    DataNode n40, n48;
    if (line_txt.empty())
        MILO_FAIL("Empty command");
    mBuffer.push_front(line_txt);
    if (line_txt[line_txt.length() - 1] == '/') {
        line_txt.erase(line_txt.length() - 1, 1);
        SetShowing(false);
    }
    if (mBuffer.size() > mMaxBuffer) {
        mBuffer.pop_back();
    }
    mBufPtr = mBuffer.begin();
    MILO_LOG("> %s\n", line_txt);
    n40 = DataNode(DataReadString(line_txt.c_str()), kDataArray);
    n40.Array()->Release();
    mInput->CurrentLine().erase();
    LogCheat(-1, 0, n40.Array());
    if (n40.Array()->Type(0) == kDataCommand && n40.Array()->Size() == 1) {
        n48 = n40.Array()->Command(0)->Execute();
    } else {
        n48 = n40.Array()->Execute();
    }
    String output;
    output << "Evaluates to " << n48 << "\n";
    mInput->Print(output.c_str());
    MILO_LOG("%s", output);
}
