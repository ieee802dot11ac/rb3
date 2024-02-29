#ifndef UTL_POOLALLOC_H
#define UTL_POOLALLOC_H

// forward declaration
class FixedSizeAlloc;

class ChunkAllocator {
public:
    FixedSizeAlloc* mAllocs[32];

    ChunkAllocator(int, int, int);
    void* Alloc(int);
    void Free(void*, int);
    void UploadDebugStats();
};

class FixedSizeAlloc {
public:
    int mAllocSizeWords;
    int mNumAllocs;
    int mMaxAllocs;
    int mNumChunks;
    int* mFreeList;
    int mNodesPerChunk;

    FixedSizeAlloc(int, ChunkAllocator*, int);
    ~FixedSizeAlloc();
    void* Alloc();
    void Free(void*);
    void Refill();
    void* RawAlloc(int);
};

enum PoolType {
    MainPool,
    FastPool
};

bool AddrIsInPool(void*, PoolType);
void* _PoolAlloc(int, int, PoolType);
void _PoolFree(int, PoolType, void*);

#endif