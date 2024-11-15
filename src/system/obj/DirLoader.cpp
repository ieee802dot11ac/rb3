#include "DirLoader.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "types.h"
#include "utl/BinStream.h"
#include "utl/ChunkStream.h"
#include "utl/Loader.h"
#include "utl/LogFile.h"
#include "utl/MakeString.h"
#include "os/Archive.h"
#include "obj/Utl.h"
#include "utl/Symbols.h"
#include "utl/ClassSymbols.h"

#include "decomp.h"

bool gHostCached;
bool DirLoader::sCacheMode = false;

void BeginTrackObjMem(const char* cc1, const char* cc2){

}

void EndTrackObjMem(Hmx::Object* obj, const char* cc1, const char* cc2){

}

DECOMP_FORCEACTIVE(DirLoader,
    "MemPoint Overflow",
    "MemPoint Underflow"
)

DirLoader* DirLoader::Find(const FilePath& fp){
    if(fp.empty()) return 0;
    std::list<Loader*>& refs = TheLoadMgr.mLoaders;
    for(std::list<Loader*>::iterator it = refs.begin(); it != refs.end(); it++){
        if((*it)->mFile == fp){
            DirLoader* dl = dynamic_cast<DirLoader*>(*it);
            if(dl) return dl;
        }
    }
    return 0;
}

DirLoader* DirLoader::FindLast(const FilePath& fp){
    if(fp.empty()) return 0;
    std::list<Loader*>& refs = TheLoadMgr.mLoaders;
    for(std::list<Loader*>::reverse_iterator it = refs.rbegin(); it != refs.rend(); it++){
        if((*it)->mFile == fp){
            DirLoader* dl = dynamic_cast<DirLoader*>(*it);
            if(dl) return dl;
        }
    }
    return 0;
}

void DirLoader::PrintLoaded(const char* text) {
    TextStream* cout = &TheDebug;
    if (text) {
        cout = new LogFile(text);
    }
    std::list<Loader*>& refs = TheLoadMgr.mLoaders;
    for(std::list<Loader*>::iterator it = refs.begin(); it != refs.end(); it++){
        if(*it && (*it)->IsLoaded()){
            FilePath& itFile = (*it)->mFile;
            const char* text2 = itFile.c_str();
            if(itFile.empty()) text2 = "unknown_dir";
            cout->Print(MakeString("%s\n", text2));
        }
    }
    if (cout) delete cout;
}

class ObjectDir* DirLoader::GetDir() {
    MILO_ASSERT(IsLoaded(), 364);
    mAccessed = true;
    return mDir;
}

class ObjectDir* DirLoader::LoadObjects(const FilePath& f, Loader::Callback* c, BinStream* b) {
    DirLoader l(f, kLoadFront, c, b, NULL, false);
    TheLoadMgr.PollUntilLoaded(&l, NULL);
    return l.GetDir();
}

Symbol DirLoader::GetDirClass(const char* cc){
    ChunkStream cs(cc, ChunkStream::kRead, 0x10000, true, kPlatformNone, false);
    if(cs.Fail()){
        return Symbol("");
    }
    else {
        EofType t;
        while(t = cs.Eof(), t != NotEof){
            MILO_ASSERT(t == TempEof, 0x199);
        }
        int i;
        cs >> i;
        Symbol s;
        cs >> s;
        return s;
    }
}

DECOMP_FORCEACTIVE(DirLoader,
    "ObjectDir",
    "system",
    "dir_sort",
    "0"
)

void DirLoader::SetCacheMode(bool b){
    sCacheMode = b;
}

const char* DirLoader::CachedPath(const char* cc, bool b){
    const char* ext = FileGetExt(cc);
    if((sCacheMode || b) && ext){
        bool isMilo = strcmp(ext, "milo") == 0;
        if(isMilo){
            return MakeString("%s/gen/%s.milo_%s", FileGetPath(cc, 0), FileGetBase(cc, 0), PlatformSymbol(TheLoadMgr.GetPlatform()));
        }
    }
    return cc;
}

bool DirLoader::SaveObjects(const char*, class ObjectDir*) {
    MILO_ASSERT(0, 587);
    return 0;
}

DirLoader::DirLoader(const FilePath& f, LoaderPos p, Loader::Callback* c, BinStream* bs, class ObjectDir* d, bool b) : Loader(f, p),
    mRoot(), mOwnStream(false), mStream(bs), mObjects(NULL, kObjListAllowNull),
    mCallback(c), mDir(d), mPostLoad(0), mLoadDir(1), mDeleteSelf(0), mProxyName(0), mProxyDir(0), mTimer(), mAccessed(false), unk99(0) {
    if(d){
        mDeleteSelf = true;
        mProxyName = d->Name();
        class ObjectDir* dDir = d->Dir();
        mProxyDir = dDir;
        if(dDir) mProxyDir->AddRef(this);
        mDir->mLoader = this;
    }
    if(!bs && !d && !b){
        DataArray* da = SystemConfig()->FindArray("force_milo_inline", false);
        if(da){
            for(int i = 1; i < da->Size(); i++) {
                const char* filename = da->Str(i);
                if (FileMatch(f.c_str(), filename)) MILO_FAIL("Can't dynamically load milo files matching %s", filename);
            }
        }
    }
    mState = &DirLoader::OpenFile;
}

bool DirLoader::IsLoaded() const {
    return mState == &DirLoader::DoneLoading;
}

const char* DirLoader::StateName() const {
    if(mState == &DirLoader::OpenFile) return "OpenFile";
    else if(mState == &DirLoader::LoadHeader) return "LoadHeader";
    else if(mState == &DirLoader::LoadDir) return "LoadDir";
    else if(mState == &DirLoader::LoadResources) return "LoadResources";
    else if(mState == &DirLoader::CreateObjects) return "CreateObjects";
    else if(mState == &DirLoader::LoadObjs) return "LoadObjs";
    else if(mState == &DirLoader::DoneLoading) return "DoneLoading";
    else return "INVALID";
}

void DirLoader::PollLoading() {
    while (!IsLoaded()) { // wrong
        (this->*mState)();
        if (TheLoadMgr.mTimer.SplitMs() > TheLoadMgr.mPeriod) return;
        if ((TheLoadMgr.unk20.empty() ? NULL : TheLoadMgr.unk20.front()) != this) return;
    }
}

// this matches, but dear god i hope this isn't what HMX actually wrote
Symbol DirLoader::FixClassName(Symbol sym){
    if(mRev < 0x1C){
        if(sym == CharClipSamples) sym = CharClip;
        if(mRev < 0x1B){
            if(sym == BandMeshLauncher) sym = PartLauncher;
            if(mRev < 0x1A){
                if(sym == P9TransDraw) sym = CharTransDraw;
                if(mRev < 0x19){
                    if(sym == RenderedTex) sym = TexRenderer;
                    else if(sym == CompositeTexture) sym = LayerDir;
                    if(mRev < 0x18){
                        if(sym == BandFx) return WorldFx;
                        if(mRev < 0x16){
                            if(sym == Slider) return BandSlider;
                            if(mRev < 0x15){
                                if(sym == TextEntry) return BandTextEntry;
                                if(mRev < 0x14){
                                    if(sym == Placer) return BandPlacer;
                                    if(mRev < 0x13){
                                        if(sym == ButtonEx) return BandButton;
                                        else if(sym == LabelEx) return BandLabel;
                                        else if(sym == PictureEx) return BandPicture;
                                        if(mRev < 0x12){
                                            if(sym == UIPanel) return PanelDir;
                                            if(mRev < 0x10){
                                                if(sym == WorldInstance) return WorldObject;
                                                if(mRev < 0xF){
                                                    if(sym == View) return Group;
                                                    if(mRev < 7){
                                                        if(sym == String) return Line;
                                                        if(mRev < 6){
                                                            if(sym == MeshGenerator) return Generator;
                                                            if(mRev < 5){
                                                                if(sym == TexMovie) return Movie;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return sym;
}

void DirLoader::OpenFile() {
    mTimer.Start();
    const char* fileStr = mFile.c_str();
    if (mFile.empty()) {
        mRoot = FilePath::sRoot;
    } else {
        char buf[0x100];
        strcpy(buf, FileGetPath(fileStr, NULL));
        int len = strlen(buf);
        if(len - 4 > 0){
            bool isGen = strcmp("/gen", buf + len) == 0;
            if(isGen) buf[len] = 0;
        }
        mRoot = FileMakePath(FileRoot(), buf, NULL);
    }
    if (mStream == 0) {
        Archive* theArchive = TheArchive;
        bool cache_mode = sCacheMode;
        bool using_cd = UsingCD();

        bool matches = false;
        if(gHostFile){
            if(FileMatch(fileStr, gHostFile)) matches = true;
        }
        if(matches){
            SetCacheMode(gHostCached);
            SetUsingCD(false);
            TheArchive = 0;
        }

        const char* path = CachedPath(fileStr, false);
        mStream = new ChunkStream(path, ChunkStream::kRead, 0x10000, true, kPlatformNone, false);
        mOwnStream = true;
        if (matches) {
            SetCacheMode(cache_mode);
            SetUsingCD(using_cd);
            TheArchive = theArchive;
        }
        if (mStream->Fail()) {
            if (mProxyDir){
                Cleanup(MakeString("%s: could not load: %s", PathName(mProxyDir), path));
                return;
            }
            else {
                Cleanup(MakeString("Could not load: %s", path));
                return;
            }
        }
    }
    mState = &DirLoader::LoadHeader;
}

bool DirLoader::SetupDir(Symbol sym){
    BeginTrackObjMem(sym.Str(), mFile.c_str());
    if(mDir){
        if(mDir->ClassName() != sym){
            TheDebugNotifier << MakeString(MakeString("%s: Proxy class %s not %s, converting", mFile.c_str(),
                mDir->ClassName(), sym)); // for some reason
            class ObjectDir* newDir = dynamic_cast<class ObjectDir*>(Hmx::Object::NewObject(sym));
            if(!newDir){
                Cleanup(MakeString("%s: Trying to make non ObjectDir proxy class %s s", mFile.c_str(), mDir->ClassName(), sym));
                return false;
            }
            newDir->TransferLoaderState(mDir);
            ReplaceObject(mDir, newDir, true, true, false);
            mDir = newDir;
        }
    }
    else {
        mDir = dynamic_cast<class ObjectDir*>(Hmx::Object::NewObject(sym));
    }
    mDir->SetPathName(mFile.c_str());
    EndTrackObjMem(mDir, 0, mFile.c_str());
    return true;
}

void DirLoader::LoadHeader() {
    for (EofType i = NotEof; i != NotEof; i == mStream->Eof()) {
        MILO_ASSERT(i == TempEof, 997);
        if (TheLoadMgr.mTimer.SplitMs() > TheLoadMgr.mPeriod) return;
    }
    *mStream >> mRev;
    ResolveEndianness();
    if (mRev < 7) {
        Cleanup(MakeString("Can't load old ObjectDir %s", mFile));
        return;
    }
    Symbol s;
    if (!Hmx::Object::RegisteredFactory("RndDir")) {
        s = Symbol("ObjectDir");
    }
    Symbol s2;
    if (mRev > 13) *mStream >> s2;
    FixClassName(s2);
    char test[0x80];
    mStream->ReadString(test, 0x80);


    {
        if (SetupDir(s2) != 0) mDir->SetName(FileGetBase(mFile.c_str(), NULL), mDir);
    }
    mDir->mLoader = this;
    *mStream >> mCounter;
    if (mRev < 14) {
        mDir->Reserve(mCounter * 2, mCounter * 25);
    }
    mState = &DirLoader::CreateObjects;
}

void DirLoader::LoadResources(){
    if(mCounter-- != 0){
        FilePathTracker fpt(mRoot.c_str());
        FilePath fp2;
        *mStream >> fpt.mOldRoot;
        if(!fp2.empty()){
            TheLoadMgr.AddLoader(fp2, kLoadFront);
        }
    }
    else {
        if(mRev > 0xD) mState = &DirLoader::LoadDir;
        else mState = &DirLoader::LoadObjs;
    }
}

void ReadDead(BinStream& bs) {
    u8 buf;
    bs >> buf;
    while (true)
    {
        if (buf == 0xAD)
        {
            if ((bs >> buf, buf == 0xDE) &&
                (bs >> buf, buf == 0xAD) &&
                (bs >> buf, buf == 0xDE))
            {
                break;
            }
        }
        else
        {
            bs >> buf;
        }
    }
}

void DirLoader::DoneLoading() { }

void DirLoader::Replace(Hmx::Object* from, Hmx::Object* to) {
    MILO_ASSERT(from == mProxyDir && !to, 1393);
    mProxyDir = NULL;
    mProxyName = NULL;
    delete this; // uhhh.
}

void DirLoader::Cleanup(const char* s) {
    if (s) MILO_WARN(s);
    mObjects.clear();
    if (mOwnStream) {
        delete mStream;
        mStream = NULL;
    }
    if (mDir) {
        if (!IsLoaded()) {
            mDir->mLoader = NULL;
            if (mProxyName == NULL) {
                if (mDir) delete mDir;
                mDir = NULL;
            }
            if (mProxyName != NULL) {
                if (mDir->IsProxy()) mDir->ResetEditorState();
            }
        }
    }
}

DirLoader::~DirLoader() {
    mDeleteSelf = 0;
    if (!IsLoaded()) {
        Cleanup(NULL);
    }
    else if(mDir){
        mDir->mLoader = 0;
        if(!mAccessed && !mProxyName){
            delete mDir;
            mDir = 0;
        }
    }
    if(mProxyDir) mProxyDir->Release(this);
    if(mCallback && unk99){
        mCallback->FailedLoading(this);
        mCallback = 0;
    }
}

Hmx::Object* DirLoader::RefOwner() { return NULL; }
