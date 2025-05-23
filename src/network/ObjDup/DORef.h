#pragma once
#include "DOHandle.h"
#include "Platform/RootObject.h"

namespace Quazal {
    class DuplicatedObject;

    class DORef : public RootObject {
    public:
        DORef();
        ~DORef();

        void SetSoft();
        void Release();
        void Acquire();

        void EmptyInit() {
            m_bLockRelevance = true;
            m_hReferencedDO = 0;
            m_poReferencedDO = 0;
        }

        DuplicatedObject *m_poReferencedDO; // 0x0
        DOHandle m_hReferencedDO; // 0x4
        bool m_bLockRelevance; // 0x8
    };
}