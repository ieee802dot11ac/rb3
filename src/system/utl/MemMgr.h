#ifndef UTL_MEMMGR_H
#define UTL_MEMMGR_H
#include <stddef.h>
#include "utl/PoolAlloc.h"

void* operator new(size_t) throw();
void operator delete(void*) throw();
void* operator new[](size_t) throw();
void operator delete[](void*) throw();

void* _MemOrPoolAlloc(int, PoolType);
void _MemOrPoolFree(int, PoolType, void*);
void* _MemOrPoolAllocSTL(int, PoolType);
void _MemOrPoolFreeSTL(int, PoolType, void*);

#endif