#pragma once
#include "os/Debug.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

// BEGIN OBJPTR TEMPLATE
// -------------------------------------------------------------------------------

/**
 * @brief Basic ref-counting pointer to an Hmx::Object.
 *
 * @tparam T1 Hmx::Object derivative
 * @tparam T2 ObjectDir type (?)
 */
template <class T1, class T2 = class ObjectDir>
class ObjPtr : public ObjRef {
public:
    ObjPtr(Hmx::Object *owner, T1 *ptr = nullptr) : mOwner(owner), mPtr(ptr) {
        if (mPtr != nullptr)
            mPtr->AddRef(this);
    }
    ObjPtr(const ObjPtr &oPtr) : mOwner(oPtr.mOwner), mPtr(oPtr.mPtr) {
        if (mPtr != nullptr)
            mPtr->AddRef(this);
    }
    virtual ~ObjPtr() {
        if (mPtr != nullptr)
            mPtr->Release(this);
    }
    virtual Hmx::Object *RefOwner() { return mOwner; }
    virtual void Replace(Hmx::Object *o1, Hmx::Object *o2) {
        if (mPtr == o1)
            *this = dynamic_cast<T1 *>(o2);
    }

    Hmx::Object *Owner() const { return mOwner; }
    T1 *Ptr() { return mPtr; }
    T1 *Ptr() const { return mPtr; }
    operator T1 *() const { return mPtr; }
    T1 *operator->() const { return mPtr; }

    void operator=(T1 *t) {
        if (t != mPtr) {
            if (mPtr)
                mPtr->Release(this);
            if (mPtr = t)
                t->AddRef(this);
        }
    }

    void operator=(const ObjPtr<T1, T2> &oPtr) { *this = oPtr.mPtr; }
    bool Load(BinStream &bs, bool b, class ObjectDir *dir);

    /** The owner of this Object. */
    Hmx::Object *mOwner;
    /** The pointer to this Object. */
    T1 *mPtr;
};

template <class T1>
DONT_INLINE BinStream &operator<<(BinStream &bs, const ObjPtr<T1, class ObjectDir> &f) {
    MILO_ASSERT(f.Owner(), 0x2D1);
    const char *objName = (f.Ptr()) ? f.Ptr()->Name() : "";
    bs << objName;
    return bs;
}

template <class T1>
DONT_INLINE BinStream &operator>>(BinStream &bs, ObjPtr<T1, class ObjectDir> &ptr) {
    ptr.Load(bs, true, 0);
    return bs;
}

// END OBJPTR TEMPLATE
// ---------------------------------------------------------------------------------

// BEGIN OBJOWNERPTR TEMPLATE
// --------------------------------------------------------------------------

/**
 * @brief Ref-counting pointer similar to ObjPtr that takes "ownership" of the object.
 *
 * @tparam T1
 * @tparam T2
 */
template <class T1, class T2 = class ObjectDir>
class ObjOwnerPtr : public ObjRef {
public:
    ObjOwnerPtr(Hmx::Object *owner, T1 *ptr = nullptr) : mOwner(owner), mPtr(ptr) {
        if (mPtr != nullptr)
            mPtr->AddRef(mOwner);
    }

    ObjOwnerPtr(const ObjOwnerPtr &oPtr) : mOwner(oPtr.mOwner), mPtr(oPtr.mPtr) {
        if (mPtr != nullptr)
            mPtr->AddRef(mOwner);
    }

    virtual ~ObjOwnerPtr() {
        if (mPtr != nullptr)
            mPtr->Release(mOwner);
    }

    virtual Hmx::Object *RefOwner() { return mOwner; }
    virtual void Replace(Hmx::Object *, Hmx::Object *) {
        MILO_FAIL("Should go to owner");
    }

    T1 *Ptr() { return mPtr; }
    operator T1 *() const { return mPtr; }
    T1 *operator->() const {
        MILO_ASSERT(mPtr, 0xAB);
        return mPtr;
    }

    void operator=(T1 *t) {
        if (t != mPtr) {
            if (mPtr != nullptr)
                mPtr->Release(mOwner);
            mPtr = t;
            if (mPtr != nullptr)
                t->AddRef(mOwner);
        }
    }

    void operator=(const ObjOwnerPtr<T1, T2> &oPtr) { *this = oPtr.mPtr; }
    bool Load(BinStream &bs, bool b, class ObjectDir *dir);

    /** The owner of this Object. */
    Hmx::Object *mOwner;
    /** The pointer to this Object. */
    T1 *mPtr;
};

template <class T1>
BinStream &operator>>(BinStream &bs, ObjOwnerPtr<T1, class ObjectDir> &ptr) {
    ptr.Load(bs, true, 0);
    return bs;
}

// END OBJOWNERPTR TEMPLATE
// ----------------------------------------------------------------------------

// BEGIN OBJPTRLIST TEMPLATE
// ---------------------------------------------------------------------------

enum ObjListMode {
    kObjListNoNull,
    kObjListAllowNull,
    kObjListOwnerControl
};

/**
 * @brief Doubly-linked list of ref-counting pointers to Objects.
 *
 * @tparam T1 Hmx::Object derivative
 * @tparam T2 ObjectDir type (?)
 */
template <class T1, class T2 = class ObjectDir>
class ObjPtrList : public ObjRef {
public:
    struct Node {
        T1 *obj;
        struct Node *next;
        struct Node *prev;

        NEW_POOL_OVERLOAD(Node);
        DELETE_POOL_OVERLOAD(Node);
    };

    class iterator {
    public:
        // if you wanna check the iterator methods in objdiff, go to CharHair.cpp
        // CharHair.cpp has plenty of ObjPtr and ObjPtrList methods for you to double
        // check

        iterator() : mNode(0) {}
        iterator(Node *node) : mNode(node) {}
        T1 *operator*() { return mNode->obj; }

        // RB2 says this returns an iterator rather than an iterator&
        // apparently this can return an iterator if inlining is off?
        iterator operator++() {
            mNode = mNode->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator!=(iterator it) { return mNode != it.mNode; }
        bool operator==(iterator it) { return mNode == it.mNode; }
        bool operator!() { return mNode == 0; }

        struct Node *mNode;
    };

    /** The collection of Objects. */
    Node *mNodes;
    /** The owner of all of these Objects. */
    Hmx::Object *mOwner;
    /** The number of Objects in this list. */
    int mSize : 24;
    /** The mode of this ObjPtrList. */
    ObjListMode mMode : 8;

    Hmx::Object *Owner() const { return mOwner; }

    ObjPtrList(Hmx::Object *owner, ObjListMode mode = kObjListNoNull)
        : mNodes(0), mOwner(owner), mSize(0), mMode(mode) {
        if (mode == kObjListOwnerControl) {
            MILO_ASSERT(owner, 0xFC);
        }
    }

    ObjPtrList(const ObjPtrList &pList)
        : mNodes(0), mOwner(pList.mOwner), mSize(0), mMode(pList.mMode) {
        for (Node *nodes = pList.mNodes; nodes != 0; nodes = nodes->next) {
            push_back(nodes->obj);
        }
    }

    // this also seems okay
    virtual ~ObjPtrList() { clear(); }
    // okay as well
    virtual void Replace(Hmx::Object *from, Hmx::Object *to) {
        if (mMode == kObjListOwnerControl) {
            mOwner->Replace(from, to);
            return;
        } else {
            Node *it = mNodes;
            while (it != 0) {
                if (it->obj == from) {
                    if (mMode == kObjListNoNull && !to) {
                        it = erase(it).mNode;
                        continue;
                    } else {
                        from->Release(this);
                        it->obj = dynamic_cast<T1 *>(to);
                        if (to)
                            to->AddRef(this);
                    }
                }
                it = it->next;
            }
        }
    }
    // refowner moved down here because that's how the weak funcs are ordered
    virtual Hmx::Object *RefOwner() { return mOwner; }
    ObjListMode Mode() const { return mMode; }

    void clear() {
        while (mSize != 0)
            pop_back();
    }

    void push_front(T1 *obj) { insert(begin(), obj); }

    // https://decomp.me/scratch/ESkuY
    // push_back__36ObjPtrList<11RndDrawable,9ObjectDir>FP11RndDrawable
    // fn_8049C424 - push_back
    // seems to be okay - shows as 100% in EventTrigger
    void push_back(T1 *obj) { insert(end(), obj); }

    // fn_805D8160 - used in RndEnviron::RemoveLight
    void remove(T1 *target) {
        for (Node *it = mNodes; it != 0; it) {
            Node *old = it;
            it = it->next;
            if (old->obj == target)
                erase(old);
        }
    }

    // fn_805D7F38 - used in RndEnviron::IsLightInList
    iterator find(const Hmx::Object *target) const {
        for (Node *it = mNodes; it != 0; it = it->next) {
            if (it->obj == target)
                return it;
        }
        return end();
    }

    // seems to be okay - shows as 100% in EventTrigger
    void pop_back() {
        MILO_ASSERT(mNodes, 0x16D);
        erase(mNodes->prev);
    }

    // RB3 apparently also has pop_front? gross //
    // pop_front__36ObjPtrList<Q23Hmx6Object,9ObjectDir>Fv
    void pop_front() {
        MILO_ASSERT(mNodes, 0x16E);
        erase(mNodes);
    }

    iterator erase(iterator it) {
        Node *unlinked = unlink(it.mNode);
        delete it.mNode;
        return unlinked;
    }

    void DeleteAll() {
        while (!empty()) {
            T1 *cur = front();
            pop_front();
            delete cur;
        }
    }

    typedef bool SortFunc(T1 *, T1 *);
    void sort(SortFunc *func) {
        if (mNodes && mNodes->next) {
            Node *last = mNodes->prev;
            for (Node *n = last->prev; n != last; n = n->prev) {
                for (Node *x = n; x != last; x = x->next) {
                    Node *nextX = x->next;
                    if (func(nextX->obj, x->obj)) {
                        T1 *tmp = x->obj;
                        x->obj = nextX->obj;
                        nextX->obj = tmp;
                    } else
                        break;
                }
            }
        }
    }

    template <class Cmp>
    void sort(Cmp cmp) {
        if (!mNodes || !mNodes->next)
            return;
        else {
            Node *last = mNodes->prev;
            for (Node *n = last->prev; n != last; n = n->prev) {
                for (Node *x = n; x != last; x = x->next) {
                    Node *nextX = x->next;
                    if (cmp(nextX->obj, x->obj)) {
                        T1 *tmp = x->obj;
                        x->obj = nextX->obj;
                        nextX->obj = tmp;
                    } else
                        break;
                }
            }
        }
    }

    // unlink__36ObjPtrList<11RndDrawable,9ObjectDir>F P Q2
    // 36ObjPtrList<11RndDrawable,9ObjectDir> 4Node fn_80389E34 in RB3 retail
    Node *unlink(Node *n) {
        MILO_ASSERT(n && mNodes, 0x24D);
        if (n->obj)
            n->obj->Release(this);
        if (n == mNodes) {
            if (mNodes->next != 0) {
                mNodes->next->prev = mNodes->prev;
                mNodes = mNodes->next;
            } else
                mNodes = 0;
            n = mNodes;
        } else if (n == mNodes->prev) {
            mNodes->prev = mNodes->prev->prev;
            mNodes->prev->next = 0;
            n = mNodes->prev;
        } else {
            n->prev->next = n->next;
            n->next->prev = n->prev;
            n = n->next;
        }
        mSize--;
        return n;
    }

    T1 *front() const {
        MILO_ASSERT(mNodes, 0x16B);
        return mNodes->obj;
    }

    T1 *back() const {
        MILO_ASSERT(mNodes, 0x16C);
        return mNodes->prev->obj;
    }

    bool empty() const { return mSize == 0; }

    // if inlining is off, these could work too? explore this
    iterator begin() const { return iterator(mNodes); }
    iterator end() const { return iterator(0); }

    int size() const { return mSize; }

    // insert__36ObjPtrList<11RndDrawable,9ObjectDir>F Q2
    // 36ObjPtrList<11RndDrawable,9ObjectDir> 8iterator P11RndDrawable fn_8049C470 -
    // insert
    iterator insert(iterator it, T1 *obj) {
        if (mMode == kObjListNoNull)
            MILO_ASSERT(obj, 0x15A);
        Node *node = new Node;
        node->obj = obj;
        link(it, node);
        return node;
    }

    // link__36ObjPtrList<11RndDrawable,9ObjectDir>F Q2
    // 36ObjPtrList<11RndDrawable,9ObjectDir> 8iterator P Q2
    // 36ObjPtrList<11RndDrawable,9ObjectDir> 4Node
    void link(iterator it, Node *n) {
        Node *&itNode = it.mNode;

        if (n->obj) {
            n->obj->AddRef(this);
        }
        n->next = itNode;

        if (itNode == mNodes) { // mNodes = ivar1, itNode = ivar2

            if (mNodes) {
                n->prev = mNodes->prev;
                mNodes->prev = n;
            } else {
                n->prev = n;
            }

            mNodes = n;
        } else if (!itNode) {
            n->prev = mNodes->prev;
            mNodes->prev->next = n;
            mNodes->prev = n;
        } else {
            n->prev = itNode->prev;
            itNode->prev->next = n;
            itNode->prev = n;
        }

        int size = mSize;
        int tmpSize = size + 1;
        MILO_ASSERT(tmpSize < 8388607, 0x244);
        mSize = size + 1;
    }

    // fn_80453DC4 in retail
    // Set__37ObjPtrList<12EventTrigger,9ObjectDir> F Q2
    // 37ObjPtrList<12EventTrigger,9ObjectDir> 8iterator P12EventTrigger
    // ObjPtrList::Set(iterator, T1*)
    // https://decomp.me/scratch/OniGf - again, seems to check out?
    void Set(iterator it, T1 *obj) {
        if (mMode == kObjListNoNull)
            MILO_ASSERT(obj, 0x14E);
        if (it.mNode->obj)
            it.mNode->obj->Release(this);
        it.mNode->obj = obj;
        if (obj)
            obj->AddRef(this);
    }

    // remove a particular item inside iterator otherIt, from list otherList,
    // and insert it into this list at the position indicated by thisIt
    void MoveItem(iterator thisIt, ObjPtrList<T1, T2> &otherList, iterator otherIt) {
        if (otherIt != thisIt) {
            otherList.unlink(otherIt.mNode);
            link(thisIt, otherIt.mNode);
        }
    }

    void operator=(const ObjPtrList<T1, T2> &x) {
        if (this == &x)
            return;
        while (mSize > x.mSize)
            pop_back();
        Node *curNodes = mNodes;
        Node *otherNodes = x.mNodes;
        while (curNodes) {
            Set(curNodes, otherNodes->obj);
            curNodes = curNodes->next;
            otherNodes = otherNodes->next;
        }
        for (; otherNodes != 0; otherNodes = otherNodes->next) {
            push_back(otherNodes->obj);
        }
    }

    // fn_8056349C in retail
    // seems to be okay - shows as 100% in EventTrigger
    bool Load(BinStream &bs, bool b);
};

// fn_80563460 in retail (BinStream >> ObjPtrList<Hmx::Object, ObjectDir>)
template <class T1>
BinStream &operator>>(BinStream &bs, ObjPtrList<T1, class ObjectDir> &ptr) {
    ptr.Load(bs, true);
    return bs;
}

// END OBJPTRLIST TEMPLATE
// -----------------------------------------------------------------------------

// LOAD FUNCTIONS
// They have to be done here outside of their class bodies due to a conflict with
// ObjectDir and DirLoader's headers
#include "obj/Dir.h"

template <class T1, class T2>
inline bool ObjPtr<T1, T2>::Load(BinStream &bs, bool warn, class ObjectDir *dir) {
    char buf[0x80];
    bs.ReadString(buf, 0x80);
    if (!dir && mOwner)
        dir = mOwner->Dir();
    if (mOwner && dir) {
        *this = dynamic_cast<T1 *>(dir->FindObject(buf, false));
        if (mPtr == nullptr && buf[0] != '\0' && warn) {
            MILO_WARN("%s couldn't find %s in %s", PathName(mOwner), buf, PathName(dir));
        }
        return false;
    } else {
        *this = nullptr;
        if (buf[0] != '\0' && warn)
            MILO_WARN("No dir to find %s", buf);
    }
    return true;
}

template <class T1, class T2>
inline bool ObjOwnerPtr<T1, T2>::Load(BinStream &bs, bool b, class ObjectDir *dir) {
    char buf[0x80];
    bs.ReadString(buf, 0x80);
    if (!dir && mOwner)
        dir = mOwner->Dir();
    if (mOwner && dir) {
        *this = dynamic_cast<T1 *>(dir->FindObject(buf, false));
        if (mPtr == 0 && buf[0] != '\0' && b) {
            MILO_WARN("%s couldn't find %s in %s", PathName(mOwner), buf, PathName(dir));
        }
        return false;
    } else {
        *this = 0;
        if (buf[0] != '\0')
            MILO_WARN("No dir to find %s", buf);
    }
    return true;
}

template <class T1, class T2>
inline bool ObjPtrList<T1, T2>::Load(BinStream &bs, bool warn) {
    bool ret = true;
    clear();
    int count;
    bs >> count;
    class ObjectDir *dir = 0;
    if (mOwner)
        dir = mOwner->Dir();
    if (warn)
        MILO_ASSERT(dir, 0x207);
    while (count != 0) {
        char buf[0x80];
        bs.ReadString(buf, 0x80);
        if (dir) {
            T1 *casted = dynamic_cast<T1 *>(dir->FindObject(buf, false));
            if (!casted && buf[0] != '\0') {
                if (warn)
                    MILO_WARN(
                        "%s couldn't find %s in %s", PathName(mOwner), buf, PathName(dir)
                    );
                ret = false;
            } else if (casted) {
                push_back(casted);
            }
        }
        count--;
    }
    return ret;
}

// ObjPtrList work:
// https://decomp.me/scratch/xXn7G
// https://decomp.me/scratch/3EvHB
