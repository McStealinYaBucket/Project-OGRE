// Network/Item.cpp -- October 2013: ItemQueue. Asserts on their 2013 lines (52 61 73 108 112 119
// 127 129 136 138); the shape is 2016's minus head_time, which 2013 does not have.
// VC9 emits deleteAll and ~ItemQueue after push_front (PLAYER 0x005d4260, 0x005d42a0): deleteAll
// calls pop_if_present, defined later, so it is compiled after it; the markers below follow the
// source, not the address order, for those two.
#include "Item.h"

namespace RBX {
namespace Network {






































// FUNCTION: PLAYER 0x005d3e20
// FUNCTION: STUDIO 0x0066d1a0
bool ItemQueue::preValidate(int i)
{
	RBXASSERT(inCode == 0);
	inCode = i;
	return true;
}

// FUNCTION: PLAYER 0x005d3e90
// FUNCTION: STUDIO 0x0066d210
bool ItemQueue::postValidate(int i)
{
	RBXASSERT(inCode == i);
	inCode = 0;
	return true;
}
ItemQueue::ItemQueue()
{
	inCode = 0;
}
// FUNCTION: PLAYER 0x005d42a0
// FUNCTION: STUDIO 0x0066d630
ItemQueue::~ItemQueue()
{
	RBXASSERT(inCode == 0);
}
// FUNCTION: PLAYER 0x005d3ef0
// FUNCTION: STUDIO 0x0066d270
bool ItemQueue::empty() const
{
	return itemList.empty();
}
// FUNCTION: PLAYER 0x005d3f10
// FUNCTION: STUDIO 0x0066d2a0
RBX::Time::Interval ItemQueue::head_wait() const
{
	if (itemList.empty())
		return RBX::Time::Interval::zero();
	return RBX::Time::now<RBX::Time::Fast>() - itemList.front().timestamp;
}
// FUNCTION: PLAYER 0x005d4260
// FUNCTION: STUDIO 0x0066d5f0
void ItemQueue::deleteAll()
{
	Item* item;
	while (pop_if_present(item))
		delete item;
}







// FUNCTION: PLAYER 0x005d3f70
// FUNCTION: STUDIO 0x0066d300
bool ItemQueue::pop_if_present(Item*& item)
{
	RBXASSERT(preValidate(3));

	if (itemList.empty())
	{
		RBXASSERT(postValidate(3));
		return false;
	}

	item = &itemList.front();
	itemList.pop_front();

	RBXASSERT(postValidate(3));
	return true;
}
// FUNCTION: PLAYER 0x005d40a0
// FUNCTION: STUDIO 0x0066d430
void ItemQueue::push_back(Item* item)
{
	item->timestamp = RBX::Time::now<RBX::Time::Fast>();
	RBXASSERT(preValidate(4));
	itemList.push_back(*item);
	RBXASSERT(postValidate(4));
}
// FUNCTION: PLAYER 0x005d4180
// FUNCTION: STUDIO 0x0066d510
void ItemQueue::push_front(Item* item)
{
	item->timestamp = RBX::Time::now<RBX::Time::Fast>();
	RBXASSERT(preValidate(5));
	itemList.push_front(*item);
	RBXASSERT(postValidate(5));
}

}}	// namespace
