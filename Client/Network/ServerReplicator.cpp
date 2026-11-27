// Network/ServerReplicator.cpp -- October 2013, partial.

#include "ServerReplicator.h"
#include "V8DataModel/NetworkSettings.h"
#include "FastLog.h"

// 2013 registers it in App's ScriptContext.obj (initializer PLAYER 0x00a66170, after NetworkReadItem's);
// defined here until that file carries it. Default 1 (.data byte).
namespace FLog {
	// GLOBAL: PLAYER 0x00d30f1e
	// GLOBAL: STUDIO 0x00ec180a
	Channel JoinSendExtraItemCount = 1;
	int JoinSendExtraItemCountIniter = RegisterLogGroup("JoinSendExtraItemCount", &JoinSendExtraItemCount, NULL, FASTVARTYPE_STATIC); }

namespace RBX {
namespace Network {

// GLOBAL: PLAYER 0x00b17468
// GLOBAL: STUDIO 0x00bd6a78
const char* const sServerReplicator = "ServerReplicator";

// FUNCTION: PLAYER 0x00571870
// FUNCTION: STUDIO 0x00654010
bool ServerReplicator::canUseProtocolVersion(int protocolVersion) const
{
	return remoteProtocolVersion == 0 || protocolVersion <= remoteProtocolVersion;
}

// FUNCTION: PLAYER 0x005718b0
// FUNCTION: STUDIO 0x00654050
void ServerReplicator::receiveCluster(RakNet::BitStream& inBitstream, Instance* instance)
{
	if (acceptsTerrainChanges)
		Super::receiveCluster(inBitstream, instance);
}

// While joining (no send-buffer limit) the server sends extra item packets per step until the first
// marker goes out.
// FUNCTION: PLAYER 0x00571a90
// FUNCTION: STUDIO 0x00654220
bool ServerReplicator::sendItemsPacket()
{
	if (!Super::sendItemsPacket())
		return false;
	if (settings().m_unk0xec == -1 && waitingForMarker)
		for (int i = 0; i < FLog::JoinSendExtraItemCount; i++)
			if (!Super::sendItemsPacket())
				return false;
	return true;
}

// The FLog::ForEachVariable visitor serializeSFFlags passes (2016 name; a file static). Not written yet.
// STUB: PLAYER 0x00571af0
// STUB: STUDIO 0x00654280
static void serializeSFFlag(const std::string& name, const std::string& value, void* context)
{
	STUB(0x00571af0);
}

// FUNCTION: PLAYER 0x00573160
// FUNCTION: STUDIO 0x006550f0
void ServerReplicator::serializeSFFlags(RakNet::BitStream& outBitStream)
{
	outBitStream.Write(FLog::GetNumSynchronizedVariable());
	FLog::ForEachVariable(&serializeSFFlag, &outBitStream, FASTVARTYPE_SYNC);
}

} // namespace Network
} // namespace RBX
