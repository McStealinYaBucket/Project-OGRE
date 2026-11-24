#pragma once

#if defined(RBX_PLATFORM_IOS) || defined(__APPLE__) || __ANDROID__
namespace rbx
{
    template<typename T>
    class atomic
    {
        T v;
    public:
        atomic(T value = 0) : v(value) {}
        
        T operator=(T v)
        {
            this->v = v;
            return v;
        }
        
        operator T() const { return v; }
        
        T compare_and_swap(T value, T comparand) {
            return __sync_val_compare_and_swap(&v, comparand, value);
        }
        
        T operator++() {
            return __sync_add_and_fetch(&v, 1);
        }
        
        T operator--() {
            return __sync_sub_and_fetch(&v, 1);
        }
        
        T operator++(int) {
            return __sync_fetch_and_add(&v, 1);
        }
        
        T operator--(int) {
            return __sync_fetch_and_sub(&v, 1);
        }
        
        T swap(T value)
        {
            return __sync_lock_test_and_set(&v, value);
        }
    };
} // namespace rbx

#elif defined(_WIN32)  // Windows

#include "rbx/debug.h"
#include "boost/detail/interlocked.hpp"
#include "boost/static_assert.hpp"
#include <cstdint>

// 2013 layout: the parameterless ctor's assert is atomic.h line 224 in both targets
// ("(((long)(&(this->v))) & 0x3) == 0", PLAYER rbx::atomic<int>::atomic() 0x004022d0, out of line,
// 26 callers). A second copy of that assert sits at line 182 (PLAYER 0x005ba6f0, called once, by
// TaskScheduler::TaskScheduler 0x009b55e0 for its member at +8): atomic_unk0x005ba6f0 below. The
// blank lines around it put both asserts on their 2013 lines.
// atomic(T) has no assert in 2013: ScriptContext's ctor 0x0068ad20 stores timedoutCount and
// timedout (atomic<int>(0)) with a bare `mov [esi+0x124], ebx`.


namespace RBX
{
    // Out-of-line interlocked helpers (bodies in Base/rbx/atomic.cpp, PLAYER 0x009b8f00..0x009b8f40).
    // rbx::atomic::swap goes through RbxInterlockedExchange (SimpleThrottlingArbiter::isThrottled
    // 0x0064dfc0, Time::now<Fast> 0x009bb680); ++, -- and compare_and_swap stay intrinsics
    // (intrusive_ptr_add_weak_ref 0x0040d2d0 `lock xadd`, intrusive_ptr_try_lock 0x006cbce0).
    long RbxInterlockedIncrement(long volatile* value);
    long RbxInterlockedDecrement(long volatile* value);
    long RbxInterlockedExchange(long volatile* value, long exchange);
    long RbxInterlockedCompareExchange(long volatile* value, long exchange, long comparand);
} // namespace RBX




































































// TEMPLATE: PLAYER 0x005ba6f0
// TEMPLATE: STUDIO 0x0065e110
// ??0?$atomic_unk0x005ba6f0@H@rbx@@QAE@XZ




















namespace rbx
{
    // The second Windows atomic class of 2013 atomic.h. Its parameterless ctor (PLAYER 0x005ba6f0,
    // out of line, one caller: TaskScheduler::TaskScheduler 0x009b55e0 for taskCount at +8) asserts
    // at line 182 with the same text as rbx::atomic's line-224 assert, but stores v = 0 before it
    // tests FLog::Asserts: the member is volatile. ++ is `lock xadd` (Thread::runJob 0x009b6933).
    // No symbol names it (Mac, iOS and the 2016 tree have one class only); the name is a placeholder.
    template <typename T>
    class atomic_unk0x005ba6f0
    {
        BOOST_STATIC_ASSERT(sizeof(T) == sizeof(long));
        volatile long v;

    public:
        atomic_unk0x005ba6f0() : v(0)
        {
            RBXASSERT((((long)(&(this->v))) & 0x3) == 0);
        }

        operator T() const { return v; }

        T operator++() {
            return BOOST_INTERLOCKED_INCREMENT(&v);
        }

        T operator--() {
            return BOOST_INTERLOCKED_DECREMENT(&v);
        }

        T operator++(int) {
            return BOOST_INTERLOCKED_INCREMENT(&v)-1;
        }

        T operator--(int) {
            return BOOST_INTERLOCKED_DECREMENT(&v)+1;
        }
    };
} // namespace rbx








namespace rbx
{
    template <typename T>
    class atomic
    {
        BOOST_STATIC_ASSERT(sizeof(T) == sizeof(long));
        long v;

    public:
        // Out of line in 2013 (PLAYER 0x004022d0 for atomic<int>; Name::Name 0x006136c0 calls it).
        atomic() : v(0)
        {
            RBXASSERT((((long)(&(this->v))) & 0x3) == 0);
        }

        atomic(T value) : v(value)
        {
        }

        T operator=(T v)
        {
            this->v = v;
            return v;
        }

        operator T() const { return v; }

        T compare_and_swap(T value, T comparand) {
            return BOOST_INTERLOCKED_COMPARE_EXCHANGE(&v, value, comparand);
        }

        T operator++() {
            return BOOST_INTERLOCKED_INCREMENT(&v);
        }

        T operator--() {
            return BOOST_INTERLOCKED_DECREMENT(&v);
        }

        T operator++(int) {
            return BOOST_INTERLOCKED_INCREMENT(&v)-1;
        }

        T operator--(int) {
            return BOOST_INTERLOCKED_DECREMENT(&v)+1;
        }

        T swap(T value) {
            return RBX::RbxInterlockedExchange(&v, value);
        }
    };
} // namespace rbx

#else // you are using the wrong atomic
#error "not supported"
#endif
