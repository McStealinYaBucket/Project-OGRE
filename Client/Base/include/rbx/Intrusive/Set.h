// Base/include/rbx/Intrusive/Set.h: October 2013 reconstruction (Project OGRE).
// Evidence: 14 asserts at Base\include\RBX/Intrusive/Set.h (PLAYER and STUDIO, lines 72-213, kept
// on the same lines here with the same expression text); iOS 1029 RTTI
// Set<PartInstance, PhysicsService>::Hook and ::NextRef, Set<RobloxExtraSpace>::Hook::remove()
// and Set<RobloxExtraSpace>::erase(Iterator) (luaconf.h's RobloxExtraSpace embeds a Hook). No file
// in this tree includes it yet.
//
// An intrusive, unordered, doubly linked set: the element carries the links (it derives from
// Set<Item, Tag>::Hook, Tag telling several sets apart), so insert, remove and the membership
// test are constant time, nothing is allocated, and nothing throws. An element belongs to at
// most one set per Tag: inserting it elsewhere first takes it out of the old set, and destroying
// it unlinks it. Copying an element copies none of its membership.
#pragma once

#include "RBX/Debug.h"
#include "boost/noncopyable.hpp"

namespace RBX { namespace Intrusive {

template<class Item, class Tag = Item>
class Set : boost::noncopyable
{
private:
	// The forward link. The set's own head is a bare NextRef, so a Hook's prev can point at either.
	class NextRef
	{
		friend class Set;
	protected:
		Item* next;
		inline NextRef() throw() : next(0) {}
		inline NextRef(const NextRef& other) throw() : next(0) {}
		inline NextRef& operator=(const NextRef& other) throw() {} // keeps this object's links
	};

public:
	class Hook : public NextRef
	{
		friend class Set;
		Set* _container;
		NextRef* prev;
	public:
		inline Hook() throw() : _container(0), prev(0) {}
		inline Hook(const Hook& other) throw() : _container(0), prev(0) {}
		inline Hook& operator=(const Hook& other) throw() {} // keeps this object's links
		inline ~Hook() throw() { remove(); }

		// How the links work. The set's head_ref is a bare NextRef whose next is the first element.
		// Every element's prev points at the NextRef in front of it: the head for the first element,
		// the previous element's Hook otherwise. So unlinking never needs to know whether an element
		// is first, and needs no pointer to the set except to keep count.

		//   head_ref.next -> A ; A.prev -> &head_ref ; A.next -> B ; B.prev -> &A ; B.next -> 0

		// An element is linked exactly when _container is set; the asserts below check that this
		// agrees with the links (a linked element always has a prev, since the head counts).
		// Elements of type Item reach their links through Set::Hook:: and Set::NextRef::, which keeps
		// two Set<Item, TagA>/<Item, TagB> bases of the same Item apart.
		// 2013 holds remove()'s assert at line 72 and is_linked()'s at line 88.

		// Hook is the per-element part: the back link, the owning set, and NextRef's
		// forward link. It is what Item derives from.

		// (spacing: keeps the asserts below on their 2013 line numbers)



		// Unlinks from whatever set holds this element; a no-op when unlinked.
		inline void remove() throw()
		{
			if (!is_linked())
				return;
			RBXASSERT(prev!=0 || Set::NextRef::next!=0);
			if (prev)
				prev->Set::NextRef::next = NextRef::next;
			if (NextRef::next)
				NextRef::next->Set::Hook::prev = prev;
			_container->count--;
			NextRef::next = 0;
			prev = 0;
			_container = 0;
		}



		// A linked element always has a back link (the head counts), and only then a container.
		inline bool is_linked() const throw()
		{
			RBXASSERT((_container != 0) == ((Set::NextRef::next != 0) || (prev != 0)));
			return _container != 0;
		}

		inline Set* container() throw() { return _container; }
	};



	// Forward iterator; past the end is the null item.
	class Iterator
	{
		friend class Set;
		Item* item;
		Iterator(Item* item) throw() : item(item)
		{
			RBXASSERT(!item || item->Set::Hook::is_linked());
		}
	public:
		inline Iterator() throw() : item(0) {}

		inline bool operator==(const Iterator& other) const throw() { return item == other.item; }
		inline bool operator!=(const Iterator& other) const throw() { return item != other.item; }

		// Dereferencing asserts the iterator is not at the end and that the item is still
		// linked (2013 lines 122-123 and 129-130); erase() therefore advances before it
		// unlinks. Iterators stay valid when other elements are removed, not their own.

		// (spacing: keeps the asserts below on their 2013 line numbers)



		inline Item* operator->() throw()
		{
			RBXASSERT(item);
			RBXASSERT(!item || item->Set::Hook::is_linked());
			return item;
		}
		inline Item& operator*() throw()
		{

			RBXASSERT(item);
			RBXASSERT(!item || item->Set::Hook::is_linked());
			return *item;
		}
		inline Iterator& operator++() throw()
		{

			RBXASSERT(item);
			item = item->Set::Hook::next;


			RBXASSERT(!item || item->Set::Hook::is_linked());
			return *this;
		}

		inline bool empty() const throw() { return item == 0; }

		typedef std::forward_iterator_tag iterator_category;
		typedef Item& value_type;
		typedef void difference_type;
		typedef Item* pointer;
		typedef Item& reference;
	};

	inline Set() throw() : count(0) {}
	// Unlinks every element; the elements themselves live on.
	inline ~Set() throw()
	{
		Iterator it = begin();
		while (!it.empty())
			it = erase(it);
	}

	inline size_t size() const throw() { return count; }
	inline bool empty() const throw() { return count == 0; }

	// Removes *it and returns the iterator after it.
	Iterator erase(Iterator it) throw()
	{
		Item& victim(*it);
		++it;
		remove_element(victim);
		return it;
	}

	// false when item is not in this set
	bool remove_element(Item& item) throw()
	{
		if (item.Set::Hook::_container != this)
			return false;
		item.Set::Hook::remove();
		return true;
	}

	// insert() is the only place an element gains links. It never fails: an element already in
	// this set is left where it is, one in another set (same Tag) is moved, and the new element
	// always goes to the front, so iteration order is newest first.
	// 2013 asserts here at lines 198, 199, 204, 205 and 213.

	// (spacing: keeps the asserts below on their 2013 line numbers)



	// Links item at the front, taking it out of any other set first.
	void insert(Item& item) throw()
	{
		if (item.Set::Hook::_container == this)
			return;
		item.Set::Hook::remove();
		RBXASSERT(!item.Set::Hook::next);
		RBXASSERT(!item.Set::Hook::prev);
		Item* head = head_ref.next;
		if (head)
		{

			RBXASSERT(head->Set::Hook::is_linked());
			RBXASSERT(head->Set::Hook::container() == this);
			item.Set::NextRef::next = head;
			head->Set::Hook::prev = &item;
		}
		head_ref.next = &item;
		item.Set::Hook::prev = &head_ref;
		item.Set::Hook::_container = this;

		RBXASSERT(item.Set::Hook::next || item.Set::Hook::prev);
		count++;
	}

	inline Iterator begin() throw() { return Iterator(head_ref.next); }
	inline Iterator end() throw() { return Iterator(); }

	// Names the std algorithms and boost::intrusive-style callers expect: iterator, push_front
	// (always the front: this set has no other insertion point) and iterator_to, which yields
	// end() for an element of another set instead of asserting.
	typedef Iterator iterator;
	inline void push_front(Item& item) throw() { insert(item); }
	inline Iterator iterator_to(Item& item) throw()
	{
		return item.Set::Hook::_container == this ? Iterator(&item) : Iterator();
	}

private:
	size_t count;     // maintained by insert and Hook::remove, so size() is O(1)
	NextRef head_ref; // next = first element; never itself an element
};

}}
