#pragma once
#include "rbx/atomic.h"
#include "rbx/Declarations.h"

namespace RBX
{

namespace Diagnostics
{

// 2013: the ctor and dtor count through the out-of-line interlocked helpers
// (RbxInterlockedIncrement 0x009b8f10 / RbxInterlockedDecrement 0x009b8f00), inlined into
// every derived ctor and dtor: Instance ctor 0x00620efc, DataModel ctor 0x0064ecc2 and dtor
// 0x0064dda5, SoundChannel ctor 0x006f2ffc and dtor 0x006ededd, ThreadRef(lua_State*)
// 0x00673934 (before its members, so the count is a base, as in 2016). 2016 uses
// rbx::atomic's ++/-- instead.
template<typename T>
class RBXBaseClass Countable
{
public:
    // Public: a few 2013 bodies still count by hand where the counted object is a
    // temporary the tree does not model yet (ThreadRef pins in ThreadRef.cpp).
    static rbx::atomic<int> count;
    static long getCount() { return count; }
    ~Countable()
    {
        RbxInterlockedDecrement(reinterpret_cast<long volatile*>(&count));
    }
protected:
    // (0x009b94d0 / STUDIO 0x00a6b420 is RBX::Limits::Countable's ctor, not this one:
    // this class is empty, 1 byte at Instance+0x34.)
    Countable()
    {
        RbxInterlockedIncrement(reinterpret_cast<long volatile*>(&count));
    }
};

template<class T>
rbx::atomic<int> Countable<T>::count;

} // namespace Diagnostics

} // namespace RBX
