#pragma once
#include "utl/FilePath.h"
#include "rndobj/Dir.h"
#include <string.h>

class UIResource {
public:
    struct Compare {
        bool operator()(const UIResource* left, const UIResource* right) const {
            return left->mResourcePath < right->mResourcePath;
        }

        bool operator()(const UIResource* left, const char* right) const {
            return strcmp(left->mResourcePath.c_str(), right) < 0;
        }

        bool operator()(const char* left, const UIResource* right) const {
            return strcmp(left, right->mResourcePath.c_str()) < 0;
        }

        bool operator()(const char* left, const char* right) const {
            return strcmp(left, right) < 0;
        }
    };

    int mRefCount; // 0x0
    FilePath mResourcePath; // 0x4
    ObjDirPtr<RndDir> mDir; // 0x10

    UIResource(const FilePath&);
    void Load(bool);
    void PostLoad();
    void Release();
    void ForceRelease();
    RndDir* Dir() { return mDir; }
};
