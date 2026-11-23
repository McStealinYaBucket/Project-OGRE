// Base/include/rbx/ArrayDynamic.h: October 2013 reconstruction (Project OGRE).
// No October 2013 binary has a trace of this container: no ArrayDynamic or ArrayBase symbol on Mac
// or iOS, no string or RTTI in PLAYER or STUDIO, and nothing in this tree includes the file. The
// 2016 header (a 766-line aligned vector with trivial-type fast paths) is therefore not carried
// over. What follows is a small aligned dynamic array written for this tree, keeping the 2016
// public names (ArrayBase, ArrayNoInit, ArrayDynamic and their common members) so a later import
// that expects them compiles. Its code generation is not evidence of anything.
#pragma once

#include "rbx/Debug.h"
#include "boost/cstdint.hpp"

#include <malloc.h>
#include <new>
#include <cstring>

namespace RBX {

// A view of size() contiguous T's it does not own.
template<class T>
class ArrayBase
{
public:
	T& operator[](size_t i) { RBXASSERT_VERY_FAST(i < mSize); return mData[i]; }
	const T& operator[](size_t i) const { RBXASSERT_VERY_FAST(i < mSize); return mData[i]; }
	T& at(size_t i) { return (*this)[i]; }
	const T& at(size_t i) const { return (*this)[i]; }

	size_t size() const { return mSize; }
	bool empty() const { return mSize == 0; }

	T& front() { return (*this)[0]; }
	const T& front() const { return (*this)[0]; }
	T& back() { return (*this)[mSize - 1]; }
	const T& back() const { return (*this)[mSize - 1]; }

	T* begin() { return mData; }
	const T* begin() const { return mData; }
	const T* cbegin() const { return mData; }
	T* end() { return mData + mSize; }
	const T* end() const { return mData + mSize; }
	const T* cend() const { return mData + mSize; }
	T* data() { return mData; }
	const T* data() const { return mData; }
	const T* cdata() const { return mData; }

protected:
	ArrayBase(T* data, size_t size) : mData(data), mSize(size) {}

	T* mData;
	size_t mSize;
};

// Tag: leave the elements of a sized constructor unconstructed (for plain data filled later).
struct ArrayNoInit {};

// An owning, growable array whose storage is aligned to mAlignment bytes.
template<class T>
class ArrayDynamic : public ArrayBase<T>
{
public:
	static const boost::uint32_t defaultAlignment = 16;
	typedef ArrayBase<T> Base;

	ArrayDynamic() : Base(0, 0), mCapacity(0), mAlignment(defaultAlignment) {}
	explicit ArrayDynamic(size_t count, ArrayNoInit, boost::uint32_t align = defaultAlignment)
		: Base(0, 0), mCapacity(0), mAlignment(align)
	{
		reserve(count);
		this->mSize = count;
	}
	explicit ArrayDynamic(size_t count, boost::uint32_t align = defaultAlignment)
		: Base(0, 0), mCapacity(0), mAlignment(align)
	{
		resize(count);
	}
	ArrayDynamic(const ArrayDynamic& other) : Base(0, 0), mCapacity(0), mAlignment(other.mAlignment) { append(other); }
	ArrayDynamic(const Base& other) : Base(0, 0), mCapacity(0), mAlignment(defaultAlignment) { append(other); }
	~ArrayDynamic()
	{
		clear();
		_aligned_free(this->mData);
	}

	ArrayDynamic& operator=(const ArrayDynamic& other) { return assignFrom(other); }
	ArrayDynamic& operator=(const Base& other) { return assignFrom(other); }

	size_t capacity() const { return mCapacity; }

	void clear()
	{
		while (this->mSize)
			this->mData[--this->mSize].~T();
	}

	void reserve(size_t wanted)
	{
		if (wanted <= mCapacity)
			return;
		T* fresh = static_cast<T*>(_aligned_malloc(wanted * sizeof(T), mAlignment));
		if (!fresh)
			throw std::bad_alloc();
		for (size_t i = 0; i < this->mSize; ++i)
		{
			new (fresh + i) T(this->mData[i]);
			this->mData[i].~T();
		}
		_aligned_free(this->mData);
		this->mData = fresh;
		mCapacity = wanted;
	}

	void resize(size_t count)
	{
		reserve(count);
		while (this->mSize < count)
			new (this->mData + this->mSize++) T();
		while (this->mSize > count)
			this->mData[--this->mSize].~T();
	}

	void push_back(const T& value)
	{
		if (this->mSize == mCapacity)
		{
			T copy(value); // value may live in the storage being moved
			reserve(mCapacity ? mCapacity * 2 : 4);
			new (this->mData + this->mSize++) T(copy);
		}
		else
			new (this->mData + this->mSize++) T(value);
	}

	void pop_back() { this->mData[--this->mSize].~T(); }

	// Inserts value before index i, shifting the tail up by one.
	void insert(size_t i, const T& value)
	{
		RBXASSERT(i <= this->mSize);
		push_back(value);
		for (size_t k = this->mSize - 1; k > i; --k)
			std::swap(this->mData[k], this->mData[k - 1]);
	}

	void assign(size_t count, const T& value)
	{
		clear();
		reserve(count);
		while (this->mSize < count)
			new (this->mData + this->mSize++) T(value);
	}

private:
	void append(const Base& other)
	{
		reserve(this->mSize + other.size());
		for (size_t i = 0; i < other.size(); ++i)
			new (this->mData + this->mSize++) T(other[i]);
	}

	ArrayDynamic& assignFrom(const Base& other)
	{
		if (&other != this)
		{
			clear();
			append(other);
		}
		return *this;
	}

	size_t mCapacity;
	boost::uint32_t mAlignment;
};

}
