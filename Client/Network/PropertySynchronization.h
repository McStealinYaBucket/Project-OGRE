#pragma once

// Network/PropertySynchronization.h -- October 2013, partial. The 2013 binary records this path
// (c:\buildagent\...\client\network\PropertySynchronization.h); names and shape from the 2016 tree.
// Only what ServerReplicator's written functions use is declared; the map and queue are placeholders.

#include "rbx/rbxtime.h"
#include "reflection/property.h"
#include "Network/API.h"
#include "decomp.h"

namespace RBX {
namespace Network {
namespace PropSync {

namespace detail
{
	class MasterItem;
	class SlaveItem;

	// SIZE 0x48 (Master puts propertyRejectionCount at 0x48)
	template<class Item>
	class Base
	{
	protected:
		RBX::Time::Interval expirationTime;      // 0x00 ctor arg; ServerReplicator::setPropSyncExpiration 0x00520740
		undefined m_unk0x08[0x48 - 0x08];        // 0x08 2016: boost::unordered_map<PropertyKey, Item> map, rbx::timestamped_safe_queue<PropertyKey> expirationQueue

		// Base<MasterItem>: PLAYER 0x005becb0, not written yet.
		Base(RBX::Time::Interval expirationTime);

	public:
		// Base<MasterItem>: PLAYER 0x00574f90 (the Master and Slave copies fold), written in ServerReplicator.cpp.
		void expireItems();
	};
}

// SIZE 0x50. The ctor is inline (ServerReplicator ctor 0x0057c48f: Base(Interval(2)), then +0x48 = 0).
class Master : protected detail::Base<detail::MasterItem>
{
	typedef detail::Base<detail::MasterItem> Super;
public:
	unsigned int propertyRejectionCount;         // 0x48

	void expireItems()
	{
		Super::expireItems();
	}

	// Inline in 2016; 2013 keeps out-of-line copies in ServerReplicator.obj (PLAYER 0x00578a70 /
	// 0x00578e70), not written yet.
	void onPropertyChanged(Reflection::ConstProperty prop);
	FilterResult onReceivedPropertyChanged(Reflection::ConstProperty prop);

	void setExpiration(RBX::Time::Interval expirationTime)
	{
		this->expirationTime = expirationTime;
	}
};

} // namespace PropSync
} // namespace Network
} // namespace RBX
