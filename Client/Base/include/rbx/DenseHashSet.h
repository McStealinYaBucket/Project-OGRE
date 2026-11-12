/* rbx/DenseHashSet.h — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Rbx_DenseHashSet.cpp.txt)
     - insert 0x56B58C: assert "key != empty_key" at ../Base/include/rbx/DenseHashSet.h line: 24;
       probe = h + (h>>3), mask = bucketCount-1, linear probe; rehash when filled*4 >= bucketCount*3
     - rehash 0x56B6C8: new capacity = 16 when empty else bucketCount*2
     - begin 0x5674B2: skips empty_key slots in the dense bucket array
   Layout (x86): { T* first; T* last; Hash hasher; size_t filled; T emptyKey; }
   Matches the 2014-era ROBLOX DenseHash design. */

#pragma once

#include <cstddef>
#include <assert.h>
#include <boost/functional/hash.hpp>

namespace RBX {

	template<class T, class Hash = boost::hash<T> >
	class DenseHashSet
	{
	public:
		class const_iterator
		{
		public:
			const_iterator(const DenseHashSet* set, size_t pos)
				: set(set), pos(pos)
			{
				skipEmpty();
			}

			const T& operator*() const { return set->items[pos]; }
			const T* operator->() const { return &set->items[pos]; }

			const_iterator& operator++()
			{
				++pos;
				skipEmpty();
				return *this;
			}

			bool operator==(const const_iterator& other) const { return set == other.set && pos == other.pos; }
			bool operator!=(const const_iterator& other) const { return !(*this == other); }

		private:
			void skipEmpty()
			{
				while (pos < set->bucketCount && set->items[pos] == set->emptyKey)
					++pos;
			}

			const DenseHashSet* set;
			size_t pos;
		};

		DenseHashSet(const T& emptyKey)
			: items(NULL /* OGRE vc9: was nullptr */), bucketCount(0), hasher(), filled(0), emptyKey(emptyKey)
		{
		}

		DenseHashSet(const T& emptyKey, const Hash& hasher_)
			: items(NULL /* OGRE vc9: was nullptr */), bucketCount(0), hasher(hasher_), filled(0), emptyKey(emptyKey)
		{
		}

		~DenseHashSet()
		{
			delete[] items;
		}

		void insert(const T& key)
		{
			assert(!(key == emptyKey));		// "key != empty_key" (DenseHashSet.h:24 in the original)

			if (bucketCount == 0 || filled * 4 >= bucketCount * 3)	// load factor 0.75 (0x56B58C)
				rehash();

			size_t h = hasher(key);
			size_t probe = h + (h >> 3);
			size_t mask = bucketCount - 1;

			for (;;) {
				size_t index = probe & mask;
				if (items[index] == emptyKey) {
					items[index] = key;
					++filled;
					return;
				}
				if (items[index] == key)
					return;

				++probe;	// linear probing
			}
		}

		bool contains(const T& key) const
		{
			if (bucketCount == 0)
				return false;

			size_t h = hasher(key);
			size_t probe = h + (h >> 3);
			size_t mask = bucketCount - 1;

			for (;;) {
				size_t index = probe & mask;
				if (items[index] == key)
					return true;
				if (items[index] == emptyKey)
					return false;
				++probe;
			}
		}

		size_t size() const
		{
			return filled;
		}

		bool empty() const
		{
			return filled == 0;
		}

		const_iterator begin() const
		{
			return const_iterator(this, 0);
		}

		const_iterator end() const
		{
			return const_iterator(this, bucketCount);
		}

	private:
		void rehash()
		{
			size_t newCount = (bucketCount == 0) ? 16 : bucketCount * 2;	// verified 0x56B6C8
			T* newItems = new T[newCount];
			for (size_t i = 0; i < newCount; ++i)
				newItems[i] = emptyKey;

			T* oldItems = items;
			size_t oldCount = bucketCount;
			items = newItems;
			bucketCount = newCount;
			filled = 0;

			for (size_t i = 0; i < oldCount; ++i) {
				if (!(oldItems[i] == emptyKey))
					insert(oldItems[i]);
			}

			delete[] oldItems;
		}

		T* items;
		size_t bucketCount;
		Hash hasher;
		size_t filled;
		T emptyKey;
	};

} // namespace RBX
