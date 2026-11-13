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
// TaskScheduler::TaskScheduler 0x009b55e0 for its member at +8); its class is not identified, so
// the lines before the class below are padding that puts our assert on line 224.
// atomic(T) has no assert in 2013: ScriptContext's ctor 0x0068ad20 stores timedoutCount and
// timedout (atomic<int>(0)) with a bare `mov [esi+0x124], ebx`.






















































































































































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
            return BOOST_INTERLOCKED_EXCHANGE(&v, value);
        }
    };
} // namespace rbx

namespace RBX
{
    // Out-of-line interlocked helpers (bodies in Base/rbx/atomic.cpp;
    // EAs 0x153433/0x153415 batch452). The binary routes all Diagnostics
    // counting through these -- never inline intrinsics -- so the ports
    // call them too.
    long RbxInterlockedIncrement(long volatile* value);
    long RbxInterlockedDecrement(long volatile* value);
    long RbxInterlockedExchange(long volatile* value, long exchange);
    long RbxInterlockedCompareExchange(long volatile* value, long exchange, long comparand);
} // namespace RBX
#else // you are using the wrong atomic
#error "not supported"
#endif
