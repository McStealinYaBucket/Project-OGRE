/* Util/ObscureValue.h -- October 2013 reconstruction.
   No 2013 trace: neither PLAYER, STUDIO nor the Mac/iOS symtabs have an
   instantiation, and its only user (HeapValue) is unused in 2013 too
   (DataModel.h notes the 2016 HeapValue members are absent). Kept because
   Util/HeapValue.h includes it; nothing in the build instantiates it.

   Stores a T XOR-ed word by word with the object's own address, so the
   plain value never sits in memory. Reading or assigning goes through a
   union of T and a long array. */
#pragma once

#include "FastLog.h"

namespace RBX {

template<typename T>
class ObscureValue
{
	enum { kWords = sizeof(T) < sizeof(long) ? 1 : sizeof(T) / sizeof(long) };

	union Bits
	{
		long word[kWords];
		T value;
	};

	Bits masked;

	long key() const { return static_cast<long>(reinterpret_cast<uintptr_t>(this)); }

	void store(const T& v)
	{
		Bits plain;
		plain.value = v;
		for (int n = 0; n < kWords; ++n)
			masked.word[n] = plain.word[n] ^ key();
	}

public:
	explicit ObscureValue(const T& v) { store(v); }

	ObscureValue& operator=(const T& v)
	{
		store(v);
		return *this;
	}

	operator const T() const
	{
		Bits plain;
		for (int n = 0; n < kWords; ++n)
			plain.word[n] = masked.word[n] ^ key();
		return plain.value;
	}

private:
	// not default-constructible, copyable or assignable from another ObscureValue
	ObscureValue();
	ObscureValue(const ObscureValue&);
	ObscureValue& operator=(const ObscureValue&);
};

} // namespace RBX
