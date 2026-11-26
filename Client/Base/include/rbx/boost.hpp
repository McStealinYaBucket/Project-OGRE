/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

#pragma once

#include <algorithm>	// defines std::min and std::max before Windows.h takes over

#include "boost/config/user.hpp"
#ifndef ROBLOX_BOOST_CONFIGS
#error	// Please re-get the full boost directory
#endif
#include "boost/shared_ptr.hpp"


#include "boost/bind.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>

// for placement_any
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/throw_exception.hpp>
#include <boost/static_assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <new>
#include <typeinfo>

using boost::shared_ptr;
using boost::scoped_ptr;
using boost::weak_ptr;

#ifdef _WIN32
//#include <windows.h>
#else 
#include "RbxFormat.h"
#include <pthread.h>
// This is a hack. Truncates a pointer.
#define GetCurrentThreadId() (static_cast<unsigned>(reinterpret_cast<long>(pthread_self())))
#define SwitchToThread() {sched_yield();}
// We may decide to use the following instead on Mac, but we would prefer the above.
//#define SwitchToThread() {struct timespec req = {0, 1}; nanosleep(&req, NULL);}
#endif

namespace RBX
{
	// TODO: Does boost have a nicer way of doing this?
	template<class T>
	void del_fun(T* t)
	{
		delete t;
	}

	// Inline in 2013 (no out-of-line copy in any build; ios has no symbol): the 64-bit exponent
	// test of boost::math::isfinite (and 0x7ff00000 on the high word, cmp of the low words;
	// RunningAverage<double,double>::sample PLAYER 0x004612a0).
	inline bool isFinite(double value) { return (boost::math::isfinite)(value); }
	inline bool isFinite(int value) { return true; }
}

namespace rbx
{

		
	namespace implementation
	{
		// 2013: { type, destruct, construct } (12 bytes). typed_holder<bool>::singleton, inlined in
		// placement_any::operator=<bool> PLAYER 0x0044c630, stores +0 typeid, +8 construct, +4 destruct;
		// Item ctor 0x0079f820 copies through [holder+8], addArgument 0x007a00d0 destroys through [holder+4].
		class type_holder : boost::noncopyable
        {
		public: // operations
			const std::type_info* type;
			void (*destruct)(char* dest);
			void (*construct)(const char* src, char* dest);
        };

        template<typename ValueType>
        class typed_holder : public type_holder
        {
			typed_holder()
            {
				type = &typeid(ValueType);
				construct = &construct_func;
				destruct = &destruct_func;
			}

        public:
			static const typed_holder* singleton()
			{
				static typed_holder<ValueType> s;
				return &s;
			}
			
			static void construct_func(const char* src, char* dest)
			{
				const ValueType* value = reinterpret_cast<const ValueType*>(src);
				ValueType* v = reinterpret_cast<ValueType*>(dest);
				new (v) ValueType(*value);
			} 

			static void destruct_func(char* dest) 
			{
				ValueType* value = reinterpret_cast<ValueType*>(dest);
				value->~ValueType();
			}
        };
	}

	// placement_any is a reworking of boost::any that embeds the value inside of
	// itself, rather than in the heap. This eliminates new/delete operations. However,
	// you must know in advance how big the values are able to be. Also, placement_any
	// allocates enough memory for the largest possible object, even when it is void.
	// SizeType must be a class that is as large as the largest sized object that
	// will be placed inside placement_any
	template<typename SizeType>
    class placement_any
    {
    public: // structors
		placement_any() 
			: holder(0)
		{
		}

		placement_any(const placement_any& other)
			: holder(0)
		{
			if (other.holder)
				(*other.holder->construct)(other.data, data);
			holder = other.holder;	// construct didn't throw, so we can assign the holder now
		}

        template<typename ValueType>
        explicit placement_any(const ValueType& value)
			: holder(0)
        {
			// If this fails, then make ValueType the new SizeType!
			BOOST_STATIC_ASSERT((sizeof(ValueType) <= sizeof(SizeType)));

			ValueType* v = reinterpret_cast<ValueType*>(data);
			new (v) ValueType(value);
			holder = implementation::typed_holder<ValueType>::singleton();	// construct didn't throw, so we can assign it now
        }

        ~placement_any()
        {
			if (holder)
				(*holder->destruct)(data);
        }

    public: // modifiers
		placement_any& swap(placement_any& rhs)
        {
			placement_any temp(*this);
			*this = rhs;
			rhs = temp;
            return *this;
        }

        template<typename ValueType>
        placement_any& operator=(const ValueType& rhs)
        {
			const implementation::typed_holder<ValueType>* s = implementation::typed_holder<ValueType>::singleton();
			if (holder == s)
			{
				// Optimization. Is this worth it?
				ValueType* dest = reinterpret_cast<ValueType*>(data);
				*dest = rhs;
			}
			else
			{
				if (holder)
				{
					(*holder->destruct)(data);
					holder = 0;
				}

				// If this fails, then make ValueType the new SizeType!
				BOOST_STATIC_ASSERT((sizeof(ValueType) <= sizeof(SizeType)));

				ValueType* v = reinterpret_cast<ValueType*>(data);
				new (v) ValueType(rhs);

				holder = s;
			}
			return *this;
        }

        placement_any& operator=(const placement_any& rhs)
        {
			if (&rhs == this)
				return *this;

			if (holder)
			{
				(*holder->destruct)(data);
				holder = 0;
			}
			if (rhs.holder)
			{
				(*rhs.holder->construct)(rhs.data, data); 
				holder = rhs.holder;
			}
			return *this;
        }

    public: // queries

        bool empty() const
        {
            return holder == 0;
        }

		// 2013: typeid(void) when empty (Variant::convert<ValueMap> 0x0079f780 loads 0x00dc6cb0).
		const std::type_info& type() const
		{
			return holder ? *holder->type : typeid(void);
		}

        const char* getData() const
        {
			return holder ? data : NULL;
        }

        char* getData()
        {
			return holder ? data : NULL;
        }

    public: // representation (read by the any_cast functions below)

		const implementation::type_holder* holder;
		char data[sizeof(SizeType)];

    };

	// 2013 boost::any_cast-style pointer cast: the type_info address is compared first, then
	// type_info::operator== (out of line: PLAYER 0x0079f730 for ValueTable, 0x006fdad0 for ValueArray;
	// inlined into Variant::convert<ValueMap> 0x0079f780).
	template<typename ValueType, typename SizeType>
	ValueType* any_cast(placement_any<SizeType>* operand)
	{
		return operand && (&operand->type() == &typeid(ValueType) || operand->type() == typeid(ValueType))
			? reinterpret_cast<ValueType*>(operand->data)
			: 0;
	}

	template<typename ValueType, typename SizeType>
	const ValueType* any_cast(const placement_any<SizeType>* operand)
	{
		return any_cast<ValueType>(const_cast<placement_any<SizeType>*>(operand));
	}

	// RTTI .?AVbad_placement_any_cast@rbx@@, vftable PLAYER 0x00b11c5c, what() 0x00447380.
	class bad_placement_any_cast : public std::bad_cast
	{
	public:
		virtual const char* what() const throw()
		{
			return "rbx::bad_placement_any_cast: failed conversion using rbx::placement_cast";
		}
	};

	// 2013 reference cast (any_cast<bool&> PLAYER 0x00701140, any_cast<ValueMap const&> 0x0058ef20):
	// the address test, then type_info::operator!=, then boost::throw_exception.
	template<typename ValueType, typename SizeType>
	ValueType any_cast(placement_any<SizeType>& operand)
	{
		typedef typename boost::remove_reference<ValueType>::type nonref;
		if (&operand.type() != &typeid(nonref) && operand.type() != typeid(nonref))
			boost::throw_exception(bad_placement_any_cast());
		return *reinterpret_cast<nonref*>(operand.data);
	}

	// boost::any_cast's const overload: forwards to the reference cast. The by-value
	// any_cast<CoordinateFrame> PLAYER 0x0058ebd0 calls any_cast<CoordinateFrame const&> 0x00701380
	// and copies; withVariantValue 0x008a67b0 calls any_cast<bool const&> 0x0044d360.
	template<typename ValueType, typename SizeType>
	inline ValueType any_cast(const placement_any<SizeType>& operand)
	{
		typedef typename boost::remove_reference<ValueType>::type nonref;
		return any_cast<const nonref&>(const_cast<placement_any<SizeType>&>(operand));
	}
}

// TEMPLATE: PLAYER 0x009b2040
// TEMPLATE: STUDIO 0x00a697e0
// ?invoke@?$void_function_obj_invoker0@V?$bind_t@XP6AXABV?$function0@X@boost@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@ZV?$list2@V?$value@V?$function0@X@boost@@@_bi@boost@@V?$value@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@23@@_bi@2@@_bi@boost@@X@function@detail@boost@@SAXAATfunction_buffer@234@@Z

// TEMPLATE: PLAYER 0x009b2070
// TEMPLATE: STUDIO 0x00a69810
// ?invoke@?$void_function_obj_invoker0@V?$bind_t@XP6AXV?$shared_ptr@Udata@worker_thread@RBX@@@boost@@ABV?$function0@W4work_result@worker_thread@RBX@@@2@@ZV?$list2@V?$value@V?$shared_ptr@Udata@worker_thread@RBX@@@boost@@@_bi@boost@@V?$value@V?$function0@W4work_result@worker_thread@RBX@@@boost@@@23@@_bi@2@@_bi@boost@@X@function@detail@boost@@SAXAATfunction_buffer@234@@Z

// TEMPLATE: PLAYER 0x009b2120
// TEMPLATE: STUDIO 0x00a698c0
// ??1?$list2@V?$value@V?$shared_ptr@Udata@worker_thread@RBX@@@boost@@@_bi@boost@@V?$value@V?$function0@W4work_result@worker_thread@RBX@@@boost@@@23@@_bi@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x009b2240
// TEMPLATE: STUDIO 0x00a699e0
// ??1?$list2@V?$value@V?$function0@X@boost@@@_bi@boost@@V?$value@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@23@@_bi@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x009b2250
// TEMPLATE: STUDIO 0x00a699f0
// ??1?$bind_t@XP6AXV?$shared_ptr@Udata@worker_thread@RBX@@@boost@@ABV?$function0@W4work_result@worker_thread@RBX@@@2@@ZV?$list2@V?$value@V?$shared_ptr@Udata@worker_thread@RBX@@@boost@@@_bi@boost@@V?$value@V?$function0@W4work_result@worker_thread@RBX@@@boost@@@23@@_bi@2@@_bi@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x009b2480
// TEMPLATE: STUDIO 0x00a69c20
// ??1?$bind_t@XP6AXABV?$function0@X@boost@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@ZV?$list2@V?$value@V?$function0@X@boost@@@_bi@boost@@V?$value@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@23@@_bi@2@@_bi@boost@@QAE@XZ

// TEMPLATE: PLAYER 0x009b25f0
// TEMPLATE: STUDIO 0x00a69d90
// ?manager@?$functor_manager@V?$bind_t@XP6AXV?$shared_ptr@Udata@worker_thread@RBX@@@boost@@ABV?$function0@W4work_result@worker_thread@RBX@@@2@@ZV?$list2@V?$value@V?$shared_ptr@Udata@worker_thread@RBX@@@boost@@@_bi@boost@@V?$value@V?$function0@W4work_result@worker_thread@RBX@@@boost@@@23@@_bi@2@@_bi@boost@@@function@detail@boost@@CAXABTfunction_buffer@234@AAT5234@W4functor_manager_operation_type@234@U?$bool_@$0A@@mpl@4@@Z

// TEMPLATE: PLAYER 0x009b2a70
// TEMPLATE: STUDIO 0x00a6a210
// ?manager@?$functor_manager@V?$bind_t@XP6AXABV?$function0@X@boost@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@ZV?$list2@V?$value@V?$function0@X@boost@@@_bi@boost@@V?$value@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@23@@_bi@2@@_bi@boost@@@function@detail@boost@@CAXABTfunction_buffer@234@AAT5234@W4functor_manager_operation_type@234@U?$bool_@$0A@@mpl@4@@Z
