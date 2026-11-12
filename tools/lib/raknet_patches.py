"""Regenerate staging/raknet/patches/*.patch from the edit lists below (pristine RakNet 4.034 -> 2013).

Run:
    python3 -P tools/lib/raknet_patches.py          # rewrite staging/raknet/patches/000N-*.patch
Then measure with tools/lib/raknet_match.py.

Each group is applied on top of the previous one and written as its own patch (diff -ruN, applied with
patch -p1 inside a copy of RakNet-4.034/Source). Every edit names the PLAYER bytes that need it.
An edit's `old` text must occur exactly `n` times (default 1) in the file at that point.
"""
import bisect
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "tools/lib"))
import raknet_match as rm  # noqa: E402
from probe_compare import read_obj  # noqa: E402
SRC = os.path.join(ROOT, "staging/raknet/RakNet-4.034/Source")
OUT = os.path.join(ROOT, "staging/raknet/patches")
# Upstream RakNet_PC-4.067 (same archive, sha256 81019665...a522): the text of upstream changes is taken
# from here, never from the later Roblox tree.
UP = os.path.join(ROOT, "3rdparty/_downloads/raknet-4.067-pc/x/Source")


def upstream(fn, start, end):
    """Text of upstream 4.067 `fn` from the line starting with `start` up to (excluding) `end`."""
    s = open(os.path.join(UP, fn), newline="").read().replace("\r\n", "\n")
    a = s.index(start)
    return s[a:s.index(end, a)]

NTS_LOOP = "for (i=0; i < {o}messageHandlerList.Size(); i++)"

# ---------------------------------------------------------------------------------------------------
# 0001: upstream changes between 4.034 and the 4.036 Roblox imported (all present in upstream 4.067).
# Plugins are split into thread-safe (pluginListTS) and reliability-layer (pluginListNTS) lists, chosen
# by the new PluginInterface2::UsesReliabilityLayer() virtual.
#   - RakPeer fields after activeSystemListSize shift by 4 and after banList by 0x10 (List = 12 bytes):
#     GetOfflinePingResponse 0x005f8200 lea [esi+0x26c] (4.034: 0x268), GetSplitMessageProgressInterval
#     0x005f88f0 [ecx+0x484] (4.034: 0x474). The +4 is Roblox's startingActiveSystemListIndex (0002).
#   - PluginInterface2 vftable 0x00b27304 has 17 slots, one more than 4.034 (UsesReliabilityLayer).
#   - AttachPlugin 0x005fbc30 calls vtbl[UsesReliabilityLayer] and picks a list; PushBackPacket
#     0x005fbd00 walks two lists; Receive 0x005fcef0 calls CallPluginCallbacks twice.
# ---------------------------------------------------------------------------------------------------
G1 = [  # see HEADERS for the revision-log entry behind each edit
    ("PluginInterface2.h",
     "\tvirtual void OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason) {(void) packet; (void) failedConnectionAttemptReason;}\n",
     "\tvirtual void OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason) {(void) packet; (void) failedConnectionAttemptReason;}\n\n"
     "\t/// Return true to receive the reliability layer callbacks (then attach/detach only while RakPeer is inactive)\n"
     "\tvirtual bool UsesReliabilityLayer(void) const {return false;}\n"),
    ("RakPeer.h",
     "\tDataStructures::List<PluginInterface2*> messageHandlerList;\n",
     "\tDataStructures::List<PluginInterface2*> pluginListTS, pluginListNTS;\n"),
    ("RakPeer.h",
     "\tvoid OnConnectedPong(RakNet::Time sendPingTime, RakNet::Time sendPongTime, RemoteSystemStruct *remoteSystem);\n",
     "\tvoid OnConnectedPong(RakNet::Time sendPingTime, RakNet::Time sendPongTime, RemoteSystemStruct *remoteSystem);\n"
     "\tvoid CallPluginCallbacks(DataStructures::List<PluginInterface2*> &pluginList, Packet *packet);\n"),
    # Startup / Shutdown / Receive(Update): both lists
    ("RakPeer.cpp",
     "\tfor (i=0; i < messageHandlerList.Size(); i++)\n\t{\n\t\tmessageHandlerList[i]->OnRakPeerStartup();\n\t}\n",
     "\tfor (i=0; i < pluginListTS.Size(); i++)\n\t{\n\t\tpluginListTS[i]->OnRakPeerStartup();\n\t}\n\n"
     "\tfor (i=0; i < pluginListNTS.Size(); i++)\n\t{\n\t\tpluginListNTS[i]->OnRakPeerStartup();\n\t}\n"),
    ("RakPeer.cpp",
     "\tfor (i=0; i < messageHandlerList.Size(); i++)\n\t{\n\t\tmessageHandlerList[i]->OnRakPeerShutdown();\n\t}\n",
     "\tfor (i=0; i < pluginListTS.Size(); i++)\n\t{\n\t\tpluginListTS[i]->OnRakPeerShutdown();\n\t}\n"
     "\tfor (i=0; i < pluginListNTS.Size(); i++)\n\t{\n\t\tpluginListNTS[i]->OnRakPeerShutdown();\n\t}\n"),
    ("RakPeer.cpp",
     "\tfor (i=0; i < messageHandlerList.Size(); i++)\n\t{\n\t\tmessageHandlerList[i]->Update();\n\t}\n",
     "\tfor (i=0; i < pluginListTS.Size(); i++)\n\t{\n\t\tpluginListTS[i]->Update();\n\t}\n"
     "\tfor (i=0; i < pluginListNTS.Size(); i++)\n\t{\n\t\tpluginListNTS[i]->Update();\n\t}\n"),
    ("RakPeer.cpp", "PUSHBACK", None),  # filled below
    ("RakPeer.cpp", "RECEIVE", None),
    ("RakPeer.cpp", "ATTACH", None),
    ("RakPeer.cpp", "DETACH", None),
    # Everything else (OnDirectSocketSend/Receive, the reliability layer's plugin list) is NTS only.
    ("RakPeer.cpp", "messageHandlerList", "pluginListNTS", "rest"),
    # ProcessOfflineNetworkPacket 0x005ffc90 (ID_OPEN_CONNECTION_REPLY_2 branch) initializes doSecurity
    ("RakPeer.cpp", "\t\t\tbool doSecurity;\n", "\t\t\tbool doSecurity=false;\n"),
    # Not taken: the lower length bound on ID_OUT_OF_BAND_INTERNAL that 4.067 has. ProcessOfflineNetworkPacket
    # 0x005ffc90 tests only `length < 0x1a9` (cmp ebx,0x1a9 / jae).
    # ReliabilityLayer::HandleSocketReceiveFromConnectedPlayer 0x005cd5a0: the IGNORED byte count is pushed
    # before the packet is freed (4.034 read internalPacket after ReleaseToInternalPacketPool).
    ("ReliabilityLayer.cpp", "RL_PUSH1", None),
    # ReliabilityLayer.h loses its DS_Multilist.h include (as in upstream 4.067): the header's own
    # _FILE_AND_LINE_ (BPSTracker::Push1 0x005ca3e0, push 0x5a) is one line earlier than in 4.034 (0x5b).
    # The MLKeyRef comparison operators that needed it are unreferenced; they are compiled out.
    ("ReliabilityLayer.h", '#include "DS_Multilist.h"\n', ""),
    ("ReliabilityLayer.cpp", "bool operator<( const DataStructures::MLKeyRef<SplitPacketIndexType> &inputKey, const InternalPacket *cls )\n",
     "#if 0\nbool operator<( const DataStructures::MLKeyRef<SplitPacketIndexType> &inputKey, const InternalPacket *cls )\n"),
    ("ReliabilityLayer.cpp", "\treturn inputKey.Get()->splitPacketIndex == cls->splitPacketIndex;\n}\n",
     "\treturn inputKey.Get()->splitPacketIndex == cls->splitPacketIndex;\n}\n#endif\n"),
    # CreateInternalPacketFromBitStream 0x005ca9f0
    ("ReliabilityLayer.cpp", "\tbool hasSplitPacket;\n", "\tbool hasSplitPacket=false;\n"),
    # RakString: GetLengthUTF8 / TruncateUTF8 (unreferenced, so not linked, but they move every later
    # __LINE__ in RakString.cpp by 64: RakString::Free 0x005c7a80 pushes 0x4e9, 4.034 has 0x4a9).
    ("RakString.cpp", "RS_UTF8", None),
    ("RakString.h", "\tvoid Truncate(unsigned length);\n",
     "\tvoid Truncate(unsigned int length);\n\tvoid TruncateUTF8(unsigned int length);\n"),
    ("RakString.h", "\tsize_t GetLength(void) const;\n", "\tsize_t GetLength(void) const;\n\tsize_t GetLengthUTF8(void) const;\n"),
]

# ---------------------------------------------------------------------------------------------------
# 0002: Roblox edits present in the 2013 build (each is marked ROBLOX/Roblox in the later tree, and PLAYER
# has the bytes). FastLog.h is the Log library header (include path: "2013 55% App/Log/include").
#   - RakPeer vftable 0x00b27f54 has 83 slots, RakPeerInterface 0x00b27d54 82: RunUpdateCycle is virtual
#     (RBX::Network::ProfiledRakPeer 0x00b1f0cc overrides it).
#   - RakPeer::startingActiveSystemListIndex (the +4 after activeSystemListSize, see 0001) drives a
#     round-robin start in RunUpdateCycle 0x00601f90, using ReliabilityLayer::Update's new bool result.
#   - FastLog strings in PLAYER: "RakNet: CloseConnection", "RakNet: Calling CloseConnectionInternal,
#     BCS_CLOSE_CONNECTION", "Raknet: connection lost" (+2 formatted), "Raknet: AckTimeout"; channel
#     "RakNetDisconnect".
# ---------------------------------------------------------------------------------------------------
G2 = [
    ("RakPeer.h", "\tunsigned int activeSystemListSize;\n",
     "\tunsigned int activeSystemListSize;\n\tunsigned int startingActiveSystemListIndex;\n"),
    ("RakPeer.h", "\tbool RunUpdateCycle( RakNet::TimeUS timeNS, RakNet::Time timeMS, BitStream &updateBitStream );\n",
     "\tvirtual bool RunUpdateCycle( RakNet::TimeUS timeNS, RakNet::Time timeMS, BitStream &updateBitStream );\n"),
    ("RakPeer.cpp", '#include "RakNetTypes.h"\n\n#ifdef _WIN32\n',
     '#include "RakNetTypes.h"\n\n#include "FastLog.h"\nLOGGROUP(RakNetDisconnect)\n\n#ifdef _WIN32\n'),
    ("RakPeer.cpp", "\tisMainLoopThreadActive = false;\n",
     "\tisMainLoopThreadActive = false;\n\tstartingActiveSystemListIndex = 0;\n"),
    ("RakPeer.cpp",
     "\tif (sendDisconnectionNotification==false && GetConnectionState(target)==IS_CONNECTED)\n\t{\n",
     "\tif (sendDisconnectionNotification==false && GetConnectionState(target)==IS_CONNECTED)\n\t{\n"
     "\t\tFASTLOG(FLog::RakNetDisconnect, \"RakNet: CloseConnection\");\n\n"),
    ("RakPeer.cpp",
     "\t\telse if (bcs->command==BufferedCommandStruct::BCS_CLOSE_CONNECTION)\n\t\t{\n",
     "\t\telse if (bcs->command==BufferedCommandStruct::BCS_CLOSE_CONNECTION)\n\t\t{\n"
     "\t\t\tFASTLOG(FLog::RakNetDisconnect, \"RakNet: Calling CloseConnectionInternal, BCS_CLOSE_CONNECTION\");\n"),
    ("RakPeer.cpp",
     "\tfor ( activeSystemListIndex = 0; activeSystemListIndex < activeSystemListSize; ++activeSystemListIndex )\n"
     "\t//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; ++remoteSystemIndex )\n\t{\n",
     "\tunsigned int count = 0;\n\tbool newStartingIndexAssigned = false;\n"
     "\tactiveSystemListIndex = startingActiveSystemListIndex;\n\n"
     "\twhile (count < activeSystemListSize)\n\t{\n"
     "\t\tif ((activeSystemListIndex+1) >= activeSystemListSize)\n\t\t\tactiveSystemListIndex = -1;\n\n"
     "\t\tcount++;\n\t\tactiveSystemListIndex++;\n\n"),
    ("RakPeer.cpp",
     "\t\t\tremoteSystem->reliabilityLayer.Update( remoteSystem->rakNetSocket->s,",
     "\t\t\tbool bufferEmpty = remoteSystem->reliabilityLayer.Update( remoteSystem->rakNetSocket->s,"),
    ("RakPeer.cpp",
     "\t\t\t\t\t|| remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP || remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ON_NO_ACK)\n\t\t\t\t{\n",
     "\t\t\t\t\t|| remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP || remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ON_NO_ACK)\n\t\t\t\t{\n"
     "\t\t\t\t\tFASTLOG(FLog::RakNetDisconnect, \"Raknet: connection lost\");\n"
     "\t\t\t\t\tFASTLOG3(FLog::RakNetDisconnect, \"Raknet: connection lost: connectMode: %d, timeMS:%u, connectionTime %u\", remoteSystem->connectMode, timeMS, remoteSystem->connectionTime);\n"
     "\t\t\t\t\tFASTLOG2(FLog::RakNetDisconnect, \"Raknet: connection lost: AckTimeout %d, IsOutgoingDataWaiting %d\", remoteSystem->reliabilityLayer.AckTimeout(timeMS) ? 1 : 0, remoteSystem->reliabilityLayer.IsOutgoingDataWaiting() ? 1 : 0);\n"),
    ("RakPeer.cpp",
     "\t\t\t\tCloseConnectionInternal( systemAddress, false, true, 0, LOW_PRIORITY );\n\t\t\t\tcontinue;\n\t\t\t}\n\n",
     "\t\t\t\tCloseConnectionInternal( systemAddress, false, true, 0, LOW_PRIORITY );\n\t\t\t\tcontinue;\n\t\t\t}\n\n"
     "\t\t\tif (!bufferEmpty && !newStartingIndexAssigned)\n\t\t\t{\n"
     "\t\t\t\tif (startingActiveSystemListIndex != activeSystemListIndex)\n\t\t\t\t{\n"
     "\t\t\t\t\tstartingActiveSystemListIndex = activeSystemListIndex;\n"
     "\t\t\t\t\tnewStartingIndexAssigned = true;\n\t\t\t\t}\n\t\t\t}\n\n"),
    ("ReliabilityLayer.h", "\tvoid Update( SOCKET s, SystemAddress &systemAddress, int MTUSize, CCTimeType time,\n",
     "\tbool Update( SOCKET s, SystemAddress &systemAddress, int MTUSize, CCTimeType time,\n"),
    ("ReliabilityLayer.cpp", "void ReliabilityLayer::Update( SOCKET s, SystemAddress &systemAddress, int MTUSize, CCTimeType time,\n",
     "bool ReliabilityLayer::Update( SOCKET s, SystemAddress &systemAddress, int MTUSize, CCTimeType time,\n"),
    ("ReliabilityLayer.cpp", "\t\tlastUpdateTime=time;\n\t\treturn;\n", "\t\tlastUpdateTime=time;\n\t\treturn true;\n"),
    ("ReliabilityLayer.cpp", "\t\tdeadConnection = true;\n\t\treturn;\n",
     "\t\tFASTLOG(FLog::RakNetDisconnect, \"Raknet: AckTimeout\");\n\n\t\tdeadConnection = true;\n\t\treturn true;\n"),
    ("ReliabilityLayer.cpp", "\t//DeleteOldUnreliableSplitPackets( time );\n}\n",
     "\t//DeleteOldUnreliableSplitPackets( time );\n\n\treturn outgoingPacketBuffer.Size() > 0 ? false : true;\n}\n"),
    # RakNetRandom::SeedMT 0x00605350 has no printf (upstream 4.067 still prints the seed)
    ("Rand.cpp", '\tprintf("%i\\n",seed);\n', ""),
    # RakNetSocket::~RakNetSocket 0x00608100 ends with cmp byte [FFlag], 0 / mov [esi], -1; the flag's
    # name string "MarkRakNetSocketOnDelete" is in PLAYER (registered outside RakNet).
    ("RakNetSocket.cpp", "using namespace RakNet;\n", '#include "FastLog.h"\nFASTFLAG(MarkRakNetSocketOnDelete)\n\nusing namespace RakNet;\n'),
    ("RakNetSocket.cpp", "\t\trecvEvent = INVALID_HANDLE_VALUE;\n\t}\n#endif\n}\n",
     "\t\trecvEvent = INVALID_HANDLE_VALUE;\n\t}\n#endif\n\n\tif (FFlag::MarkRakNetSocketOnDelete)\n\t\ts = (unsigned int)-1;\n}\n"),
    # Receive 0x005fcef0: the NTS OnReceive loop is skipped when a TS plugin consumed the packet
    ("RakPeer.cpp", "RECV_IFPACKET", None),
    ("ReliabilityLayer.cpp", "#include <math.h>\n\nusing namespace RakNet;\n",
     "#include <math.h>\n\n#include \"FastLog.h\"\nLOGGROUP(RakNetDisconnect)\n\nusing namespace RakNet;\n"),
]

SPECIAL = {
    "PUSHBACK": (
        "\tfor (i=0; i < messageHandlerList.Size(); i++)\n"
        "\t\tmessageHandlerList[i]->OnPushBackPacket((const char*) packet->data, packet->bitSize, packet->systemAddress);\n",
        "\tfor (i=0; i < pluginListTS.Size(); i++)\n"
        "\t\tpluginListTS[i]->OnPushBackPacket((const char*) packet->data, packet->bitSize, packet->systemAddress);\n"
        "\tfor (i=0; i < pluginListNTS.Size(); i++)\n"
        "\t\tpluginListNTS[i]->OnPushBackPacket((const char*) packet->data, packet->bitSize, packet->systemAddress);\n"),
    "ATTACH": (
        "\tif (messageHandlerList.GetIndexOf(plugin)==MAX_UNSIGNED_LONG)\n"
        "\t{\n\t\tplugin->SetRakPeerInterface(this);\n\t\tplugin->OnAttach();\n"
        "\t\tmessageHandlerList.Insert(plugin, _FILE_AND_LINE_);\n\t}\n",
        "\tbool isNotThreadsafe = plugin->UsesReliabilityLayer();\n"
        "\tif (isNotThreadsafe)\n\t{\n"
        "\t\tif (pluginListNTS.GetIndexOf(plugin)==MAX_UNSIGNED_LONG)\n"
        "\t\t{\n\t\t\tplugin->SetRakPeerInterface(this);\n\t\t\tplugin->OnAttach();\n"
        "\t\t\tpluginListNTS.Insert(plugin, _FILE_AND_LINE_);\n\t\t}\n\t}\n"
        "\telse\n\t{\n"
        "\t\tif (pluginListTS.GetIndexOf(plugin)==MAX_UNSIGNED_LONG)\n"
        "\t\t{\n\t\t\tplugin->SetRakPeerInterface(this);\n\t\t\tplugin->OnAttach();\n"
        "\t\t\tpluginListTS.Insert(plugin, _FILE_AND_LINE_);\n\t\t}\n\t}\n"),
    "DETACH": (
        "\tindex = messageHandlerList.GetIndexOf(plugin);\n"
        "\tif (index!=MAX_UNSIGNED_LONG)\n\t{\n"
        "\t\t// Unordered list so delete from end for speed\n"
        "\t\tmessageHandlerList[index]=messageHandlerList[messageHandlerList.Size()-1];\n"
        "\t\tmessageHandlerList.RemoveFromEnd();\n"
        "\t\tplugin->OnDetach();\n\t\tplugin->SetRakPeerInterface(0);\n\t}\n",
        "\tbool isNotThreadsafe = plugin->UsesReliabilityLayer();\n"
        "\tif (isNotThreadsafe)\n\t{\n"
        "\t\tindex = pluginListNTS.GetIndexOf(plugin);\n"
        "\t\tif (index!=MAX_UNSIGNED_LONG)\n\t\t{\n"
        "\t\t\tpluginListNTS[index]=pluginListNTS[pluginListNTS.Size()-1];\n"
        "\t\t\tpluginListNTS.RemoveFromEnd();\n\t\t}\n\t}\n"
        "\telse\n\t{\n"
        "\t\tindex = pluginListTS.GetIndexOf(plugin);\n"
        "\t\tif (index!=MAX_UNSIGNED_LONG)\n\t\t{\n"
        "\t\t\tpluginListTS[index]=pluginListTS[pluginListTS.Size()-1];\n"
        "\t\t\tpluginListTS.RemoveFromEnd();\n\t\t}\n\t}\n"
        "\tplugin->OnDetach();\n\tplugin->SetRakPeerInterface(0);\n"),
}


def rl_push1(s):
    """The three duplicate/hole-count drops after the orderingChannel check (as in upstream 4.067)."""
    a = s.index("// Check for corrupt orderingChannel")
    head, out = s[:a], s[a:]
    for _ in range(3):
        i = out.index("FreeInternalPacketData(internalPacket, _FILE_AND_LINE_ );\n", out.index("goto CONTINUE_SOCKET_DATA_PARSE_LOOP;"))
        ls = out.rindex("\n", 0, i) + 1
        ind = out[ls:i]
        old = (f"{ind}FreeInternalPacketData(internalPacket, _FILE_AND_LINE_ );\n{ind}ReleaseToInternalPacketPool( internalPacket );\n\n"
               f"{ind}bpsMetrics[(int) USER_MESSAGE_BYTES_RECEIVED_IGNORED].Push1(timeRead,BITS_TO_BYTES(internalPacket->dataBitLength));\n")
        new = (f"{ind}bpsMetrics[(int) USER_MESSAGE_BYTES_RECEIVED_IGNORED].Push1(timeRead,BITS_TO_BYTES(internalPacket->dataBitLength));\n\n"
               f"{ind}FreeInternalPacketData(internalPacket, _FILE_AND_LINE_ );\n{ind}ReleaseToInternalPacketPool( internalPacket );\n")
        assert out[ls:ls + len(old)] == old, out[ls:ls + 300]
        out = out[:ls] + new.replace("FreeInternalPacketData", "FreeInternalPacketData\x00", 1) + out[ls + len(old):]
    out = out.replace("FreeInternalPacketData\x00", "FreeInternalPacketData")
    s = head + out
    ind = "\t" * 6
    old = (f"{{\n\n{ind}FreeInternalPacketData(internalPacket, _FILE_AND_LINE_ );\n{ind}ReleaseToInternalPacketPool( internalPacket );\n\n"
           f"{ind}for (unsigned int messageHandlerIndex=0;")
    a = s.index(old)
    b = s.index(f"{ind}bpsMetrics", a)
    b = s.index("\n\n", b) + 2  # after the blank line that follows Push1, right before the goto
    seg = s[a:b].replace(f"\n{ind}FreeInternalPacketData(internalPacket, _FILE_AND_LINE_ );\n{ind}ReleaseToInternalPacketPool( internalPacket );\n\n", "", 1)
    seg += f"{ind}FreeInternalPacketData(internalPacket, _FILE_AND_LINE_ );\n{ind}ReleaseToInternalPacketPool( internalPacket );\n"
    return s[:a] + seg + s[b:]


def rs_utf8(s):
    a = s.index("void RakString::Replace(unsigned index, unsigned count, unsigned char c)")
    s = s[:a] + upstream("RakString.cpp", "// http://porg.es/", "void RakString::Replace(") + s[a:]
    old = "void RakString::Truncate(unsigned length)\n"
    assert s.count(old) == 1
    return s.replace(old, upstream("RakString.cpp", "void RakString::TruncateUTF8(", "void RakString::Truncate(unsigned int length)")
                     + "void RakString::Truncate(unsigned int length)\n")


def recv_ifpacket(s):
    a = s.index("\t\tfor (i=0; i < pluginListNTS.Size(); i++)\n\t\t{\n \t\t\tpluginResult=pluginListNTS[i]->OnReceive(packet);")
    b = s.index("\t} while(packet==0);", a)
    body = s[a:b].rstrip("\n")
    body = "\n".join("\t" + l if l.strip() else l for l in body.split("\n"))
    return s[:a] + "\t\tif (packet)\n\t\t{\n" + body + "\n\t\t}\n\n" + s[b:]


def receive_edit(s):
    """Receive(): the per-plugin switch moves into CallPluginCallbacks, called for TS then NTS; then
    OnReceive runs over TS, then over NTS."""
    a = s.index("\t\tfor (i=0; i < messageHandlerList.Size(); i++)\n\t\t{\n\t\t\tswitch (packet->data[0])")
    sw0 = s.index("\t\t\tswitch (packet->data[0])", a)
    sw1 = s.index("\t\t\t}\n", s.index("FCAR_IP_RECENTLY_CONNECTED", sw0)) + len("\t\t\t}\n")
    switch = s[sw0:sw1]
    body0 = s.index(" \t\t\tpluginResult=messageHandlerList[i]->OnReceive(packet);", sw1)
    end = s.index("\t} while(packet==0);", body0)
    loop_body = s[body0:end]  # OnReceive handling + closing brace of the for + blank
    onrecv = loop_body[:loop_body.rindex("\t\t}\n") + len("\t\t}\n")]
    new = ("\t\tCallPluginCallbacks(pluginListTS, packet);\n"
           "\t\tCallPluginCallbacks(pluginListNTS, packet);\n\n"
           "\t\tfor (i=0; i < pluginListTS.Size(); i++)\n\t\t{\n"
           + onrecv.replace("messageHandlerList", "pluginListTS") + "\n"
           "\t\tfor (i=0; i < pluginListNTS.Size(); i++)\n\t\t{\n"
           + onrecv.replace("messageHandlerList", "pluginListNTS") + "\n")
    s = s[:a] + new + s[end:]
    func = ("void RakPeer::CallPluginCallbacks(DataStructures::List<PluginInterface2*> &pluginList, Packet *packet)\n"
            "{\n\tfor (unsigned int i=0; i < pluginList.Size(); i++)\n\t{\n"
            + "\n".join(l[1:] if l.startswith("\t") else l for l in switch.replace("messageHandlerList", "pluginList").split("\n"))
            + "\t}\n}\n\n")
    tail = s.index("// #if defined(RMO_NEW_UNDEF_ALLOCATING_QUEUE)\n// #pragma pop_macro")
    return s[:tail] + func + s[tail:]


FUNCS = {"RECEIVE": receive_edit, "RL_PUSH1": rl_push1, "RS_UTF8": rs_utf8, "RECV_IFPACKET": recv_ifpacket}
GROUPS = [("0001-upstream-4.036-plugin-lists.patch", G1), ("0002-roblox-2013.patch", G2)]
HEADERS = {
    "0001-upstream-4.036-plugin-lists.patch":
        "RakNet 4.034 -> 4.036, the upstream changes the 2013 bytes need (no pristine 4.035/4.036 PC source\n"
        "archive is fetchable; text of upstream code is taken from RakNet_PC-4.067). Upstream revision log:\n"
        "  4.035 'removing DS_Multilist.h'                  -> ReliabilityLayer.h include gone (BPSTracker::Push1\n"
        "                                                      0x005ca3e0 pushes header line 0x5a, 4.034: 0x5b)\n"
        "  4.036 'Added PluginInterface::UsesReliabilityLayer()' -> pluginListTS/NTS, CallPluginCallbacks\n"
        "                                                      (PluginInterface2 vftable 0x00b27304: 17 slots;\n"
        "                                                      AttachPlugin 0x005fbc30, Receive 0x005fcef0)\n"
        "  4.036 'Fix duplicate datagram crash ... holeCount > typeRange/2' -> Push1 before Free/Release in\n"
        "                                                      HandleSocketReceiveFromConnectedPlayer 0x005cd5a0\n"
        "  4.036 'RakString: Added GetLengthUTF8 and TruncateUTF8' -> RakString.cpp __LINE__ +64 (Free 0x005c7a80)\n"
        "  plus hasSplitPacket=false (CreateInternalPacketFromBitStream 0x005ca9f0) and doSecurity=false\n"
        "  (ProcessOfflineNetworkPacket 0x005ffc90). Generated by tools/lib/raknet_patches.py.\n\n",
    "0002-roblox-2013.patch":
        "Roblox edits present in the October 2013 build: virtual RakPeer::RunUpdateCycle, round-robin\n"
        "startingActiveSystemListIndex, ReliabilityLayer::Update returns bool, FastLog RakNetDisconnect\n"
        "messages. Needs FastLog.h (Log library). Generated by tools/lib/raknet_patches.py (evidence per edit).\n\n",
}


def obj_strings(path):
    """symbol name -> NUL-terminated string at that symbol (string literals of one .obj)."""
    secs, syms = read_obj(path)
    out = {}
    for nm, value, secno, typ, cls in syms.values():
        if nm.startswith("??_C@") and secno > 0:
            data = secs[secno - 1]["data"]
            out[nm] = data[value:data.index(b"\0", value)].decode("latin1")
    return out


def line_pairs(state, log):
    """Compile `state`, locate it in PLAYER, and return {file: {our __LINE__: PLAYER __LINE__}} read from
    every aligned `push imm32; push <string>` pair (the _FILE_AND_LINE_ argument pair)."""
    d = tempfile.mkdtemp(prefix="ogre-rkl-", dir="/tmp/claude-1000")
    tus = rm.build(d, [], rm.FLAGS, src=state)
    allf, defs = rm.load(d, tus)
    tgt = rm.Target("PLAYER")
    funcs, loc, _ = rm.analyse(allf, defs, tus, tgt, [])
    strs = {tu: obj_strings(os.path.join(d, tu + ".obj")) for tu in tus}
    seen = {}
    for f in funcs:
        k = (f["tu"], f["name"])
        if k not in loc:
            continue
        a = loc[k][0]
        code = f["code"]
        mine = list(rm.MD.disasm(code, 0))
        theirs = list(rm.MD.disasm(tgt.bytes(a, max(tgt.size.get(a, 0), len(code))), 0))
        sm = difflib.SequenceMatcher(None, [i.mnemonic for i in mine], [i.mnemonic for i in theirs], autojunk=False)
        relat = {o: sym for o, sym, t in f["relocs"]}
        for i, j, n in sm.get_matching_blocks():
            for x in range(n):
                m, t = mine[i + x], theirs[j + x]
                if m.bytes[0] != t.bytes[0] or m.bytes[0] not in (0x68, 0x6A) or m.size != t.size or m.address + 1 in f["mask"]:
                    continue
                # the __FILE__ push is the next push (argument stores may sit in between)
                mn = next((q for q in mine[i + x + 1:i + x + 12] if q.mnemonic == "push"), None)
                sym = relat.get(mn.address + 1) if mn else None
                if sym not in strs[f["tu"]]:
                    continue
                path = strs[f["tu"]][sym]
                if not path.lower().endswith((".cpp", ".h")):
                    continue
                fn = re.split(r"[\\/]", path)[-1]
                ours, their = int.from_bytes(m.bytes[1:], "little", signed=True), int.from_bytes(t.bytes[1:], "little", signed=True)
                if ours <= 0 or their <= 0:
                    continue
                if their > 20000:
                    continue
                seen.setdefault(fn, {}).setdefault(ours, {}).setdefault(their, []).append(f"{f['cname']}@0x{a:08x}")
    shutil.rmtree(d)
    pairs = {}
    for fn, m in seen.items():
        for ours, th in m.items():
            if len(th) > 1:
                log.append(f"{fn}:{ours} maps to several PLAYER lines {sorted(th)}; skipped")
                continue
            pairs.setdefault(fn, {})[ours] = next(iter(th))
    return pairs


def line_edit(pairs):
    """#line directives so that every observed __LINE__ equals PLAYER's: one directive wherever the
    offset between our line and PLAYER's changes (placed right before the first line that needs it)."""
    def f(s, fn):
        lines = s.split("\n")
        ins, cur = [], 0
        for ours in sorted(pairs.get(fn, {})):
            delta = pairs[fn][ours] - ours
            if delta != cur:
                ins.append((ours, pairs[fn][ours]))
                cur = delta
        for ours, their in reversed(ins):
            lines.insert(ours - 1, f"#line {their}")
        return "\n".join(lines)
    return f


def apply(d, edits):
    for e in edits:
        fn, old, new = e[0], e[1], e[2]
        p = os.path.join(d, fn)
        s = open(p, newline="").read()
        crlf = "\r\n" in s
        s = s.replace("\r\n", "\n")
        if callable(old):
            s = old(s, fn)
        elif old in FUNCS:
            s = FUNCS[old](s)
        elif old in SPECIAL:
            o, n = SPECIAL[old]
            assert s.count(o) == 1, (fn, old, s.count(o))
            s = s.replace(o, n)
        elif len(e) > 3 and e[3] == "rest":  # every remaining occurrence outside comments
            lines = s.split("\n")
            for i, l in enumerate(lines):
                if old in l and not l.lstrip().startswith(("//", "/*", "*")) and "[j]" not in l and "j <" not in l:
                    lines[i] = l.replace(old, new)
            s = "\n".join(lines)
        else:
            n = e[3] if len(e) > 3 else 1
            assert s.count(old) == n, (fn, old[:60], s.count(old))
            s = s.replace(old, new)
        open(p, "w", newline="").write(s.replace("\n", "\r\n") if crlf else s)


def main():
    os.makedirs(OUT, exist_ok=True)
    for f in os.listdir(OUT):
        if f.endswith(".patch"):
            os.remove(os.path.join(OUT, f))
    tmp = tempfile.mkdtemp(prefix="ogre-rkp-", dir="/tmp/claude-1000")
    prev = os.path.join(tmp, "s0")
    shutil.copytree(SRC, prev)
    for i, (name, edits) in enumerate(GROUPS, 1):
        cur = os.path.join(tmp, f"s{i}")
        shutil.copytree(prev, cur)
        apply(cur, edits)
        r = subprocess.run(["diff", "-ruN", os.path.basename(prev), os.path.basename(cur)],
                           cwd=tmp, capture_output=True)  # bytes: the sources are CRLF
        open(os.path.join(OUT, name), "wb").write(HEADERS.get(name, "").encode() + r.stdout)
        print(f"{name}: {r.stdout.count(10)} lines")
        prev = cur
    # 0003: __LINE__ alignment, measured from PLAYER (see line_pairs)
    log = []
    pairs = line_pairs(prev, log)
    for line in log:
        print("note:", line)
    name = "0003-line-numbers.patch"
    cur = os.path.join(tmp, "s_lines")
    shutil.copytree(prev, cur)
    fl = line_edit(pairs)
    apply(cur, [(fn, fl, None) for fn in sorted(pairs) if os.path.exists(os.path.join(cur, fn))])
    r = subprocess.run(["diff", "-ruN", os.path.basename(prev), os.path.basename(cur)], cwd=tmp, capture_output=True)
    hdr = ("__LINE__ alignment: the 2013 files had other line counts than 4.034 + 0001 + 0002 (upstream\n"
           "4.035/4.036 and Roblox edits we do not reproduce line for line). Each #line sets the line\n"
           "PLAYER pushes with __FILE__ (_FILE_AND_LINE_) at that statement; generated by\n"
           "tools/lib/raknet_patches.py from the aligned push pairs of the located functions.\n\n")
    open(os.path.join(OUT, name), "wb").write(hdr.encode() + r.stdout)
    print(f"{name}: {r.stdout.count(10)} lines, files: " + ", ".join(f"{k} ({len(v)})" for k, v in sorted(pairs.items())))
    shutil.rmtree(tmp)


if __name__ == "__main__":
    sys.exit(main())
