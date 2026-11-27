#pragma once

// Network/Item.h -- October 2013, partial: Item and ItemQueue (the replication send queue).
// No assert in this header is recorded in PLAYER or STUDIO.
//
// Item's layout matches the 2016 shape: the item ctors (PingItem 0x005ecb20, MarkerItem 0x005eccc0,
// PingBackItem 0x005e9e40) zero the hook at 0x0c/0x10, store the replicator at 0x14 and Time() at
// 0x18. MSVC pads the vfptr to the class's 8-byte alignment (the double in RBX::Time), so
// noncopyable sits at 0x08 and the hook at 0x0c. Item itself has no vftable in PLAYER (novtable,
// abstract); PooledItem's is 0x00b17c20.
// ItemQueue: boost::intrusive::list keeps its size at 0x00 and its root node at 0x04/0x08
// (ItemQueue::empty 0x005d3ef0 reads [this+4]); inCode at 0x0c (preValidate 0x005d3e20).

#include "rbx/Declarations.h"
#include "rbx/boost.hpp"
#include "rbx/Debug.h"
#include "rbx/rbxtime.h"
#include "rbx/memory.h"
#include "boost/intrusive/list.hpp"

namespace RakNet {
	class BitStream;
}

namespace RBX {
namespace Network {

class Replicator;

struct ItemTag;
typedef boost::intrusive::list_base_hook< boost::intrusive::tag<ItemTag> > ItemHook;

// SIZE 0x20
class RBXBaseClass Item : boost::noncopyable, public ItemHook
{
protected:
	Replicator& replicator;                 // 0x14
	Item(Replicator& replicator):replicator(replicator){}

public:
	RBX::Time timestamp;                    // 0x18

	// iOS: writeItemType(BitStream&, Item::ItemType), PLAYER 0x005d3da0. Values 1-7 and 11 are proven by
	// Replicator::readItem's switch (PLAYER 0x005a4b40: a jump table over 1..11, 8-10 to the default);
	// names from 2016.
	enum ItemType
	{
		ItemTypeEnd = 0,
		ItemTypeDelete = 1,
		ItemTypeNew = 2,
		ItemTypeChangeProperty = 3,
		ItemTypeMarker = 4,
		ItemTypePing = 5,
		ItemTypePingBack = 6,
		ItemTypeEventInvocation = 7,
		ItemTypeJoinData = 11,
	};
	static void writeItemType(RakNet::BitStream& bitStream, ItemType type);

	virtual ~Item() {}                                      // vtable+0x00
	virtual bool write(RakNet::BitStream& bitStream) = 0;   // vtable+0x04
};

// Memory pool enabled Item. The deleting destructor stores this vftable and frees through the
// pool pointer stored before the object (AutoMemPool::free, PLAYER 0x005738b0); PingBackItem and
// PingItem share it.
// VTABLE: PLAYER 0x00b17c20
// VTABLE: STUDIO 0x00bd229c
// SIZE 0x20
class PooledItem : public Item, public AutoMemPool::Object
{
public:
	PooledItem(Replicator& replicator) : Item(replicator) {}
	virtual ~PooledItem() {}                                // vtable+0x00

	// The deleting destructor (PLAYER 0x005e9e70, STUDIO 0x00632650) is identical for every pooled
	// item; the linker keeps one body, named after PingBackItem in our PDB (see
	// Replicator.PingBackItem.h).
};

// Debugs and checks for re-entrant calls.  Right now NOT thread safe.   Just here for safety watch.
// SIZE 0x10
class RBXBaseClass ItemQueue : boost::noncopyable
{
public:
	typedef boost::intrusive::list<Item, boost::intrusive::base_hook<ItemHook> > ItemList;

private:
	ItemList itemList;                      // 0x00

	volatile int inCode;                    // 0x0c

	bool preValidate(int i);

	bool postValidate(int i);

public:
	ItemQueue();

	~ItemQueue();

	bool empty() const;
	size_t size() const { return itemList.size(); }
	RBX::Time::Interval head_wait() const;

	void deleteAll();

	void clear() { itemList.clear(); }

	bool pop_if_present(Item*& item);

	void push_back(Item* item);

	void push_front(Item* item);

	ItemList::iterator begin() { return itemList.begin(); }
	ItemList::iterator end() { return itemList.end(); }
};

} // namespace Network
} // namespace RBX
