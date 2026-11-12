// StreamRegion.cpp -- October 2013 Roblox reconstruction.
//
// StreamRegion is a header-only utility namespace (Util/StreamRegion.h). All the
// geometry helpers live in the header so they can constant-fold.
//
// The only out-of-line translation unit content tied to RBX::StreamRegion::Id in
// the October 2013 binaries is the RakNet::BitStream serialization pair below
// (used by the network streaming code). In the March 2016 source these are
// declared in Network/StreamingUtil.h; they are kept here until the Network
// module lands, at which point they should move there.

#include "Util/StreamRegion.h"
#include "Util/StreamingUtil.h"

namespace RBX {

	// mac: 0x83E3FB RBX::operator<<(RakNet::BitStream&, const RBX::StreamRegion::Id&)
	// Compact form: a 0 bit followed by the three coordinate bytes when every
	// coordinate fits in a signed byte; otherwise a 1 bit followed by three ints.
	RakNet::BitStream& operator<<(RakNet::BitStream& stream, const StreamRegion::Id& id)
	{
		const Vector3int32& v = id.value();

		if (v.x > 127 || v.y > 127 || v.z > 127 ||
			v.x < -128 || v.y < -128 || v.z < -128)
		{
			stream.Write1();
			stream.Write((int)v.x);
			stream.Write((int)v.y);
			stream.Write((int)v.z);
		}
		else
		{
			stream.Write0();
			unsigned __int8 x = (unsigned __int8)(v.x & 0xFF);
			unsigned __int8 y = (unsigned __int8)(v.y & 0xFF);
			unsigned __int8 z = (unsigned __int8)(v.z & 0xFF);
			stream.WriteBits(&x, 8, true);
			stream.WriteBits(&y, 8, true);
			stream.WriteBits(&z, 8, true);
		}
		return stream;
	}

	// mac: 0x83E506 RakNet::BitStream& RBX::operator>><RBX::StreamRegion::Id>(RakNet::BitStream&, RBX::StreamRegion::Id&)
	// Inverse of the above; the byte form sign-extends each coordinate.
	// STUB: PLAYER 0x005a7eb0
	// STUB: STUDIO 0x00617b30
	RakNet::BitStream& operator>>(RakNet::BitStream& stream, StreamRegion::Id& id)
	{
		int x = 0, y = 0, z = 0;
		bool large = false;
		stream >> large;

		if (large)
		{
			stream >> x;
			stream >> y;
			stream >> z;
		}
		else
		{
			char bx = 0, by = 0, bz = 0;
			stream >> bx;
			stream >> by;
			stream >> bz;
			x = bx; // (char) sign-extends into the int, per 0x83E506
			y = by;
			z = bz;
		}

		id = StreamRegion::Id(x, y, z);
		return stream;
	}

	// std::size_t RBX::hash_value(const RBX::StreamRegion::Id&)
	// Declared in the header; not found as an out-of-line function in the
	// October 2013 binaries (only boost_compatible_hash_value appears), so it is
	// implemented here for the std::unordered_ user in the 2016 style.
	std::size_t hash_value(const StreamRegion::Id& key)
	{
		StreamRegion::Id::boost_compatible_hash_value h;
		return h(key);
	}

} // namespace RBX

// TODO(ownership): the following functions in the StreamRegion export dump are
// NOT part of this module and belong to the Network module (they are listed here
// so the Network agent can find them; EAs are from the symbolled Studio binary):
//   - 0x91D54A RBX::Network::Replicator::sendClusterChunk
//   - 0x94740C RBX::Network::ClusterPacketCache::fetchIfUpToDate
//   - 0x9474AA RBX::Network::ClusterPacketCache::update
//   - 0x94E85A/0x94E8A8 RBX::Network::Replicator::StreamJob::StreamRegionIterator::{getNextSortedRegion,getNextRegion}
//     (getNextRegion asserts at Client/Network/Replicator.StreamJob.cpp line 117 "worldMinRegion != worldMaxRegion")
//   - 0x94F65C RBX::Network::Replicator::StreamJob::isTerrainRegionCollectedByCellPos
//   - 0x94F6FE RBX::Network::Replicator::StreamJob::isRegionInPendingStreamItemQueue
//   - 0x951224 RBX::Network::Replicator::StreamJob::getNextUncollectedRegion
//   - 0x955FDC/0x9563FC RBX::Network::ClientReplicator::GCJob::{gcRegion,insertRegion}
//   - the boost::unordered::detail and std::_Rb_tree / std::list template
//     instantiations keyed on RBX::StreamRegion::Id (0x94796C .. 0x971B4E, incl.
//     all 0x9718xx/0x9719xx/0x971Axx/0x971Bxx thunks) -- these come from
//     boost::unordered_set/map<RBX::StreamRegion::Id, ...> usages in Network and
//     are satisfied by vendored boost headers, not by this module.
// Keyed-container facts the Network module relies on:
//   - unordered_set<RBX::StreamRegion::Id> node size 0x14, hash stored at node+0x10
//   - unordered_map<Id, ClusterPacketCache::CachedBitStream> node size 0x20
//   - unordered_map<Id, Network::RegionInfo> node size 0x28
//   - StreamRegion::Id is a 12-byte POD (three ints) copied as a _QWORD + dword.
