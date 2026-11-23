"""Coverage of a DECOMP_PLAN Phase 5 row: every PLAYER function of a set of source files, with its score.

    python3 -P tools/row_coverage.py P.json [--row 5a|5b|5d|5h] [--files Client/App/util/Math.cpp ...] [--csv out.csv]

With no --files it measures --row (default 5a: math and G3D helpers in Client/App/util + include/Util;
5f: the Roblox code of the Network project; 5c: serialization (v8xml: XML DOM, text parser/writer,
SerializerV2 binders, the binary deserializer, WebParser/WebSerializer);
5b: DataModel and the core services; 5h: GUI, the Gui folder and the v8datamodel Gui classes; 5k: the client shell's LTCG block).
For 5h a function also counts as a file's own when it stores one of the class's vftables (ctors) or
sits only in the class's vftables (ida/exports/PLAYER-evidence/vtables.jsonl): most Gui classes have
few oracle names. Math.cpp's range is one 2013 .obj; an address counts once.
Per file, the original .obj range is [lowest PLAYER marker, highest marker + its size] widened over
neighbouring functions whose oracle name (ida/exports/PLAYER-names.csv) carries one of the file's
class qualifiers (`RBX::Math::`). Every IDA function in that range is listed with its oracle name and
its score in P.json (reccmp --json): "100" (effective), the ratio, or "-" (no body / not paired).
Coverage = functions at effective 100% / functions in range. A range is a hypothesis about .obj
bounds: /Gy keeps .obj order, but a function from another .obj can sit at a range edge.
"""
import argparse
import bisect
import csv
import json
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MARK = re.compile(r"// (?:STUB|FUNCTION): PLAYER 0x([0-9a-f]{8})")
ROW_5A = {
    "Client/App/util/Math.cpp": ["RBX::Math::"],
    "Client/App/util/Extents.cpp": ["RBX::Extents::"],
    "Client/App/util/Face.cpp": ["RBX::Face::"],
    "Client/App/util/NormalId.cpp": ["RBX::normalIdTo", "RBX::Vector3ToNormalId", "RBX::intToNormalId",
                                     "RBX::normalIdOpposite", "RBX::normalIdToMatrix3", "RBX::uvwToObject",
                                     "RBX::objectToUvw"],
    "Client/App/util/Region3.cpp": ["RBX::Region3::"],
    "Client/App/util/Region3int16.cpp": ["RBX::Region3int16::"],
    "Client/App/util/IndexedMesh.cpp": ["RBX::IndexedMesh::"],
    "Client/App/util/InterpolatedCFrame.cpp": ["RBX::InterpolatedCFrame::"],
    "Client/App/util/Quaternion.cpp": ["RBX::Quaternion::"],
    "Client/App/util/Velocity.cpp": ["RBX::Velocity::"],
    "Client/App/util/Units.cpp": ["RBX::Units::"],
    "Client/App/util/PV.cpp": ["RBX::PV::"],
    "Client/App/util/ExtentsInt32.cpp": ["RBX::ExtentsInt32::"],
    "Client/Rendering/RbxG3D/Frustum.cpp": ["RBX::Frustum::"],
    "Client/Rendering/RbxG3D/RbxRay.cpp": ["RBX::RbxRay::"],
    "Client/Rendering/RbxG3D/RbxCamera.cpp": ["RBX::RbxCamera::"],
}

ROW_5B = {
    "Client/App/v8datamodel/DataModel.cpp": ["RBX::DataModel::"],
    "Client/App/v8datamodel/DataModelJob.cpp": ["RBX::DataModelJob::", "RBX::DataModelArbiter::"],
    "Client/App/v8datamodel/GuiBuilder.cpp": ["RBX::GuiBuilder::"],
    "Client/App/v8tree/Service.cpp": ["RBX::ServiceProvider::"],
    "Client/App/v8datamodel/Workspace.cpp": ["RBX::Workspace::"],
    "Client/App/v8datamodel/Lighting.cpp": ["RBX::Lighting::"],
    "Client/App/v8datamodel/Teams.cpp": ["RBX::Teams::"],
    "Client/App/v8datamodel/Team.cpp": ["RBX::Team::"],
    "Client/Network/Players.cpp": ["RBX::Network::Players::"],
    "Client/Network/Player.cpp": ["RBX::Network::Player::"],
    "Client/App/v8datamodel/RunService.cpp": ["RBX::RunService::"],
    "Client/App/v8datamodel/StarterGuiService.cpp": ["RBX::StarterGuiService::"],
    "Client/App/v8datamodel/StarterPackService.cpp": ["RBX::StarterPackService::"],
    "Client/App/v8datamodel/DebrisService.cpp": ["RBX::DebrisService::"],
    "Client/App/v8datamodel/PhysicsInstructions.cpp": ["RBX::PhysicsInstructions::"],
}
ROW_5H = {f"Client/App/v8datamodel/{c}.cpp": [f"RBX::{c}::"] for c in (
    "GuiBase", "GuiBase2d", "GuiBase3d", "GuiObject", "Frame", "ScreenGui", "GuiMain", "BillboardGui",
    "GuiLabel", "TextLabel", "ImageLabel", "GuiButton", "GuiTextButton", "GuiImageButton", "TextBox",
    "Scale9Frame", "GuiService", "GuiLayerCollector", "CoreGuiService", "BasePlayerGui", "PlayerGui",
    "TextService", "GuiRoot")}
ROW_5H.update({
    "Client/App/Gui/GUI.cpp": ["RBX::GuiItem::", "RBX::GuiTarget::", "RBX::UnifiedWidget::"],
    "Client/App/Gui/ChatOutput.cpp": ["RBX::ChatOutput::"],
    "Client/App/Gui/AdornBillboarder.cpp": ["RBX::AdornBillboarder::"],
    "Client/App/Gui/ViewportBillboarder.cpp": ["RBX::ViewportBillboarder::"],
    "Client/App/Gui/GuiDrawImage.cpp": ["RBX::GuiDrawImage::"],
    "Client/App/Gui/ProfanityFilter.cpp": ["RBX::ProfanityFilter::"],
})
# 5f: the Roblox code of the 2013 Network project (RakNet's own rows are library code, not counted).
# Files with asserts start from FIXED_5F, the first and last PLAYER function whose __FILE__ string
# names them (ida/exports/PLAYER-evidence/functions.jsonl); the others from oracle names. The
# NetworkSettings and Marker classes sit in the Network .text range but our tree keeps them in App.
N = "RBX::Network::"
ROW_5F = {
    "Client/Network/API.cpp": [N + "isTrustedContent"],
    "Client/Network/Player.cpp": [N + "Player::"],
    "Client/Network/Players.cpp": [N + "Players::", N + "ChatMessage::", N + "AbuseRep"],
    "Client/Network/ServerReplicator.cpp": [N + "ServerReplicator::"],
    "Client/Network/Replicator.cpp": [N + "Replicator::Replicator", N + "Replicator::~Replicator",
                                      N + "Replicator::on", N + "Replicator::process", N + "ReplicatorJob",
                                      N + "DescriptorReceiver", N + "Replicator::SendDataJob",
                                      N + "Replicator::PingJob", N + "Replicator::ProcessPacketsJob",
                                      N + "Replicator::JoinDataItem"],
    "Client/Network/Streaming.cpp": [N + "serializeEnum", N + "deserializeEnum", N + "IdSerializer",
                                     N + "readBrickVector", N + "SenderDictionary"],
    "Client/Network/Client.cpp": [N + "Client::"],
    "Client/Network/Server.cpp": [N + "Server::"],
    "Client/App/v8datamodel/NetworkSettings.cpp": ["RBX::NetworkSettings::"],
    "Client/Network/NetworkFilter.cpp": [N + "FilterResult"],
    "Client/Network/Peer.cpp": [N + "Peer::", N + "PacketReceiveJob", N + "PeerStatsItem", N + "ProfiledRakPeer"],
    "Client/Network/ClientReplicator.cpp": [N + "ClientReplicator::ClientReplicator", N + "ClientReplicator::Memory",
                                            N + "ClientReplicator::Client", N + "ClientReplicator::RequestCharacterItem",
                                            N + "ClientReplicator::OnReceive"],
    "Client/Network/ChatFilter.cpp": [N + "ChatFilter"],
    "Client/Network/ConcurrentRakPeer.cpp": [N + "ConcurrentRakPeer"],
    "Client/Network/Replicator.StreamJob.cpp": [N + "Replicator::StreamJob::"],
    "Client/Network/Item.cpp": [N + "ItemQueue::"],
    "Client/Network/NetworkPacketCache.cpp": [N + "PhysicsPacketCache", N + "InstancePacketCache"],
    "Client/Network/Dictionary.cpp": [N + "ReceiverStringDictionary"],
    "Client/Network/ClusterUpdateBuffer.cpp": [N + "ClusterUpdateBuffer::"],
    "Client/Network/NetworkClusterPacketCache.cpp": [N + "ClusterPacketCache"],
    "Client/Network/MechanismItem.cpp": [N + "MechanismItem"],
    "Client/Network/DirectPhysicsReceiver.cpp": [N + "DirectPhysicsReceiver::"],
    "Client/Network/InterpolatingPhysicsReceiver.cpp": [N + "InterpolatingPhysicsReceiver"],
    "Client/Network/PhysicsSender.cpp": [N + "PhysicsSender::"],
    "Client/Network/TopNErrorsPhysicsSender.cpp": [N + "TopNErrorsPhysicsSender"],
    "Client/Network/RoundRobinPhysicsSender.cpp": [N + "RoundRobinPhysicsSender"],
    "Client/Network/ErrorCompPhysicsSender2.cpp": [N + "ErrorCompPhysicsSender2"],
    "Client/Network/ErrorCompPhysicsSender.cpp": [N + "ErrorCompPhysicsSender::"],
    "Client/Network/PhysicsReceiver.cpp": [N + "PhysicsReceiver::"],
    "Client/Network/Compressor.cpp": [N + "Compressor::"],
    "Client/Network/NetworkOwnerJob.cpp": [N + "NetworkOwnerJob"],
    "Client/Network/Replicator.GCJob.cpp": [N + "ClientReplicator::GCJob"],
}
ROW_5F.update({f"Client/Network/Replicator.{c}.cpp": [f"{N}Replicator::{c}::"] for c in (
    "PingBackItem", "DeleteInstanceItem", "ReferencePropertyChangedItem", "NewInstanceItem",
    "EventInvocationItem", "PingItem", "MarkerItem", "ChangePropertyItem", "StatsItem", "ItemSender")})
FIXED_5F = {
    "Client/Network/Player.cpp": (0x005276b0, 0x005561e0),
    "Client/Network/Players.cpp": (0x00568900, 0x005689c0),
    "Client/Network/ServerReplicator.cpp": (0x005720c0, 0x0057cbd0),
    "Client/Network/Replicator.cpp": (0x00581c10, 0x005a55a0),
    "Client/Network/Streaming.cpp": (0x005a6670, 0x005aaa30),
    "Client/Network/Client.cpp": (0x005ad5f0, 0x005b0880),  # ~Client 0x005ad760 .. Client() 0x005b0880
    "Client/Network/Server.cpp": (0x005b2de0, 0x005b4fd0),  # Server::OnReceive .. Server()
    "Client/Network/Peer.cpp": (0x005b8bf0, 0x005ba1d0),  # ~Peer .. Peer::onServiceProvider
    "Client/Network/ClientReplicator.cpp": (0x005bd170, 0x005c0c50),
    "Client/Network/ConcurrentRakPeer.cpp": (0x005c3a10, 0x005c6580),
    "Client/Network/Replicator.StreamJob.cpp": (0x005ceb60, 0x005d3100),
    "Client/Network/Item.cpp": (0x005d3e20, 0x005d42a0),
    "Client/Network/NetworkPacketCache.cpp": (0x005d5c80, 0x005d7ea0),
    "Client/Network/Dictionary.cpp": (0x005d84c0, 0x005d84c0),
    "Client/Network/ClusterUpdateBuffer.cpp": (0x005d8eb0, 0x005d8eb0),
    "Client/Network/MechanismItem.cpp": (0x005da940, 0x005db010),
    "Client/Network/InterpolatingPhysicsReceiver.cpp": (0x005dd460, 0x005dd460),
    "Client/Network/PhysicsSender.cpp": (0x005de670, 0x005e0f30),
    "Client/Network/TopNErrorsPhysicsSender.cpp": (0x005e1580, 0x005e34c0),
    "Client/Network/RoundRobinPhysicsSender.cpp": (0x005e3a20, 0x005e3a20),
    "Client/Network/ErrorCompPhysicsSender2.cpp": (0x005e4390, 0x005e6940),
    "Client/Network/ErrorCompPhysicsSender.cpp": (0x005e7130, 0x005e99d0),
    "Client/Network/PhysicsReceiver.cpp": (0x005eabd0, 0x005ec510),
    "Client/Network/Compressor.cpp": (0x005ece30, 0x005f3ef0),  # readCompressionType .. writeRotation
    "Client/Network/NetworkOwnerJob.cpp": (0x005f6c80, 0x005f7b20),
    "Client/Network/Replicator.GCJob.cpp": (0x006063b0, 0x00607320),
}
# 5d: physics. One file per 2013/2016 v8world and v8kernel .cpp; qualifiers are the classes whose methods
# that file defines (ours or the 2016 file of the same name) and that carry an oracle name.
ROW_5D = {f"Client/App/{k}": v for k, v in {
    'v8world/Assembly.cpp': ['RBX::Assembly::'],
    'v8world/AssemblyHistory.cpp': ['RBX::AssemblyHistory::'],
    'v8world/AssemblyStage.cpp': ['RBX::AssemblyStage::'],
    'v8world/Ball.cpp': ['RBX::Ball::'],
    'v8world/BallCellContact.cpp': ['RBX::BallCellContact::'],
    'v8world/BallPolyContact.cpp': ['RBX::BallPolyContact::'],
    'v8world/Block.cpp': ['RBX::Block::'],
    'v8world/Buoyancy.cpp': ['RBX::BuoyancyBallContact::', 'RBX::BuoyancyBoxContact::', 'RBX::BuoyancyContact::', 'RBX::BuoyancyCornerWedgeContact::', 'RBX::BuoyancyCylinderContact::', 'RBX::BuoyancyWedgeContact::'],
    'v8world/CleanStage.cpp': ['RBX::CleanStage::'],
    'v8world/Clump.cpp': ['RBX::Clump::'],
    'v8world/Contact.cpp': ['RBX::BallBallContact::', 'RBX::BallBlockContact::', 'RBX::BlockBlockContact::', 'RBX::BlockBlockContactData::', 'RBX::Contact::'],
    'v8world/ContactManager.cpp': ['RBX::ContactManager::'],
    'v8world/ContactManagerSpatialHash.cpp': ['RBX::ContactManagerSpatialHash::'],
    'v8world/ContactStage.cpp': ['RBX::ContactStage::'],
    'v8world/CornerWedgePoly.cpp': ['RBX::CornerWedgePoly::'],
    'v8world/Cylinder.cpp': ['RBX::Cylinder::'],
    'v8world/Edge.cpp': ['RBX::Edge::'],
    'v8world/EdgeBuffer.cpp': ['RBX::EdgeBuffer::'],
    'v8world/EdgeStage.cpp': ['RBX::EdgeStage::'],
    'v8world/GlueJoint.cpp': ['RBX::GlueJoint::', 'RBX::ManualGlueJoint::'],
    'v8world/GroundStage.cpp': ['RBX::GroundStage::'],
    'v8world/HumanoidStage.cpp': ['RBX::HumanoidStage::'],
    'v8world/IMoving.cpp': ['RBX::IMoving::', 'RBX::IMovingManager::'],
    'v8world/IPipelined.cpp': ['RBX::IPipelined::'],
    'v8world/IWorldStage.cpp': ['RBX::IWorldStage::'],
    'v8world/Joint.cpp': ['RBX::AnchorJoint::', 'RBX::FreeJoint::', 'RBX::Joint::'],
    'v8world/JointBuilder.cpp': ['RBX::JointBuilder::'],
    'v8world/JointStage.cpp': ['RBX::JointStage::'],
    'v8world/KDTree.cpp': ['RBX::KDTree::'],
    'v8world/KernelJoint.cpp': ['RBX::KernelJoint::'],
    'v8world/MaterialProperties.cpp': ['RBX::MaterialProperties::'],
    'v8world/MechToAssemblyStage.cpp': ['RBX::MechToAssemblyStage::'],
    'v8world/Mechanism.cpp': ['RBX::Mechanism::'],
    'v8world/MegaClusterPoly.cpp': ['RBX::MegaClusterPoly::'],
    'v8world/Mesh.cpp': ['RBX::Mesh::'],
    'v8world/Motor6DJoint.cpp': ['RBX::Motor6DJoint::'],
    'v8world/MotorJoint.cpp': ['RBX::MotorJoint::'],
    'v8world/MovingAssemblyStage.cpp': ['RBX::MovingAssemblyStage::'],
    'v8world/MovingStage.cpp': ['RBX::MovingStage::'],
    'v8world/MutilJoint.cpp': ['RBX::MultiJoint::'],
    'v8world/ParallelRampPoly.cpp': ['RBX::ParallelRampPoly::'],
    'v8world/Poly.cpp': ['RBX::Poly::'],
    'v8world/PolyCellContact.cpp': ['RBX::CellEdgeEdgePair::', 'RBX::CellFaceFacePair::', 'RBX::PolyCellContact::', 'RBX::PolyCellPair::'],
    'v8world/PolyContact.cpp': ['RBX::PolyContact::'],
    'v8world/PolyPolyContact.cpp': ['RBX::EdgeEdgePair::', 'RBX::FaceFacePair::', 'RBX::PolyPolyContact::'],
    'v8world/Primitive.cpp': ['RBX::EdgeList::', 'RBX::Primitive::'],
    'v8world/PrismPoly.cpp': ['RBX::PrismPoly::'],
    'v8world/PyramidPoly.cpp': ['RBX::PyramidPoly::'],
    'v8world/RightAngleRampPoly.cpp': ['RBX::RightAngleRampPoly::'],
    'v8world/RigidJoint.cpp': ['RBX::RigidJoint::'],
    'v8world/RotateConnector.cpp': ['RBX::RotateConnector::'],
    'v8world/RotateJoint.cpp': ['RBX::DynamicRotateJoint::', 'RBX::RotateJoint::', 'RBX::RotatePJoint::', 'RBX::RotateVJoint::'],
    'v8world/SendPhysics.cpp': ['RBX::SendPhysics::', 'RBX::SimJobTracker::'],
    'v8world/SimJob.cpp': ['RBX::SimJob::', 'RBX::SimJobTracker::'],
    'v8world/SimulateStage.cpp': ['RBX::SimulateStage::'],
    'v8world/SleepStage.cpp': ['RBX::SleepStage::'],
    'v8world/SmoothClusterGeometry.cpp': ['RBX::SmoothClusterGeometry::'],
    'v8world/SnapJoint.cpp': ['RBX::SnapJoint::'],
    'v8world/SpatialFilter.cpp': ['RBX::SpatialFilter::'],
    'v8world/SpatialHashMultiRes.cpp': ['RBX::SpatialHashStatic::'],
    'v8world/StepJointsStage.cpp': ['RBX::StepJointsStage::'],
    'v8world/TreeStage.cpp': ['RBX::TreeStage::'],
    'v8world/TriangleMesh.cpp': ['RBX::TriangleMesh::'],
    'v8world/WedgePoly.cpp': ['RBX::WedgePoly::'],
    'v8world/WeldJoint.cpp': ['RBX::ManualWeldJoint::', 'RBX::WeldJoint::'],
    'v8world/World.cpp': ['RBX::World::'],
    'v8kernel/Body.cpp': ['RBX::Body::'],
    'v8kernel/BuoyancyConnector.cpp': ['RBX::BuoyancyConnector::'],
    'v8kernel/Cofm.cpp': ['RBX::Cofm::'],
    'v8kernel/Connector.cpp': ['RBX::Connector::', 'RBX::NormalBreakConnector::', 'RBX::PointToPointBreakConnector::'],
    'v8kernel/Constants.cpp': ['RBX::Constants::'],
    'v8kernel/ContactConnector.cpp': ['RBX::BallBallConnector::', 'RBX::BallBlockConnector::', 'RBX::ContactConnector::', 'RBX::GeoPairConnector::'],
    'v8kernel/Kernel.cpp': ['RBX::Kernel::', 'RBX::KernelData::'],
    'v8kernel/Link.cpp': ['RBX::D6Link::', 'RBX::Link::', 'RBX::RevoluteLink::'],
    'v8kernel/Pair.cpp': ['RBX::GeoPair::'],
    'v8kernel/Point.cpp': ['RBX::Point::'],
    'v8kernel/PolyConnectors.cpp': ['RBX::BallEdgeConnector::', 'RBX::BallPlaneConnector::', 'RBX::BallVertexConnector::', 'RBX::EdgeEdgeConnector::', 'RBX::FaceEdgeConnector::', 'RBX::FaceVertexConnector::'],
    'v8kernel/SimBody.cpp': ['RBX::SimBody::'],
}.items()}
# 5k: the client shell. PLAYER's shell is one LTCG block (all 40 /GL objects, program_options
# included): 0x00401000 up to the linker's import thunks for LTCG code at 0x00480418; RbxOgre follows.
# LTCG codegen does not keep .obj order, so the row is the block, not per-file ranges.
ROW_5K = {"Client/WindowsClient": []}
# 5c: serialization. .obj ranges from the 2013 __FILE__ strings (XmlElement.cpp 328/437,
# XmlSerializer.cpp 186-310, SerializerV2.cpp 50-142) and the binary deserializer's strings
# ("Malformed data (%d != %d)" 0x008cf720, "Unexpected end of file" 0x008d20b0).
ROW_5C = {
    "Client/App/v8xml/XmlElement.cpp": ["XmlElement::", "XmlNameValuePair::", "XmlAttribute::"],
    "Client/App/v8xml/XmlSerializer.cpp": ["TextXmlParser::", "TextXmlWriter::", "decodeString"],
    "Client/App/v8xml/SerializerV2.cpp": ["SerializerV2::", "ArchiveBinder::", "MemoryBinder::"],
    "Client/App/v8xml/SerializerBinary.cpp": ["RBX::readFormatExpected", "RBX::readPropertyValues", "RBX::MemoryInputStream::"],
    "Client/App/v8xml/WebParser.cpp": ["RBX::WebParser::"],
    "Client/App/v8xml/WebSerializer.cpp": ["RBX::WebSerializer::"],
}
ROWS = {"5a": ROW_5A, "5c": ROW_5C, "5b": ROW_5B, "5h": ROW_5H, "5f": ROW_5F, "5d": ROW_5D, "5k": ROW_5K}
# Largest gap between two markers of one .obj. DataModel.obj spans 0x0062d1e0-0x00652800 with
# 0x9000-byte runs of template code between its own functions.
SPAN = {"5a": 0x2000, "5c": 0x2000, "5b": 0xa000, "5h": 0x4000, "5f": 0x4000, "5d": 0x4000, "5k": 0x80000}
VT_OWN = {"5h"}
# File -> (first, last) PLAYER function address, for files whose .obj range is known from the binary
# but whose markers sit in a header (template bodies) or have no oracle names (rows register these).
FIXED = {"Client/WindowsClient": (0x00401000, 0x004803b0),
         "Client/App/v8xml/XmlElement.cpp": (0x00613c70, 0x00614a30),
         "Client/App/v8xml/XmlSerializer.cpp": (0x00627b80, 0x00629630),
         "Client/App/v8xml/SerializerV2.cpp": (0x006fa580, 0x006fd510),
         "Client/App/v8xml/SerializerBinary.cpp": (0x008cec50, 0x008d2510),
         "Client/App/v8xml/WebParser.cpp": (0x008b0f20, 0x008b1500),
         "Client/App/v8xml/WebSerializer.cpp": (0x0081f410, 0x0081f410)}


def vtable_owned():
    """{class: functions that store one of its vftables or sit only in its vftables}. A vftable of
    RBX::FactoryProduct<class RBX::X,...> (and the other templates with X first) counts for RBX::X."""
    ev = os.path.join(ROOT, "ida/exports/PLAYER-evidence")
    owner = lambda c: (re.match(r"RBX::\w+(?:::\w+)*<class (RBX::\w+),", c) or re.match(r"(.*)", c)).group(1)
    vt, slots = {}, {}
    for line in open(os.path.join(ev, "vtables.jsonl")):
        v = json.loads(line)
        for g in v["groups"]:
            vt[g["address_point"]] = owner(v["class"])
            for sl in g["slots"]:
                slots.setdefault(sl["ea"], set()).add(owner(v["class"]))
    own = {}
    for ea, cs in slots.items():
        if len(cs) == 1:
            own.setdefault(next(iter(cs)), set()).add(ea)
    for line in open(os.path.join(ev, "functions.jsonl")):
        f = json.loads(line)
        for g in f.get("globals", []):
            if g[0] in vt:
                own.setdefault(vt[g[0]], set()).add(f["address"])
    return own


GAP = 8


def densest(addrs, span=0x2000):
    """The largest run of addresses with consecutive gaps under span (inline copies sit elsewhere)."""
    best, cur = [], []
    for ad in addrs:
        cur = cur + [ad] if cur and ad - cur[-1] < span else [ad]
        if len(cur) > len(best):
            best = cur
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("json")
    ap.add_argument("--row", default="5a", choices=sorted(ROWS))
    ap.add_argument("--files", nargs="*")
    ap.add_argument("--csv")
    a = ap.parse_args()
    funcs = sorted((int(r["address"], 16), int(r["size"])) for r in
                   csv.DictReader(open(os.path.join(ROOT, "ida/exports/PLAYER-functions.csv"))))
    starts = [f[0] for f in funcs]
    names = {}
    for r in csv.DictReader(open(os.path.join(ROOT, "ida/exports/PLAYER-names.csv"), encoding="utf-8")):
        if r["kind"] == "func":
            names[int(r["address"], 16)] = r["demangled"] or r["name"]
    score = {int(e["address"], 16): e for e in json.load(open(a.json))["data"]}
    row = ROWS[a.row]
    vtown = vtable_owned() if a.row in VT_OWN else {}
    files = {f: row.get(f, []) for f in a.files} if a.files else row
    rows, tot, ok, seen = [], 0, 0, set()
    if a.row == "5f":  # RakNet, boost and the other library rows are not the row's own code
        seen = {int(r["address"], 16) for r in
                csv.DictReader(open(os.path.join(ROOT, "ida/exports/PLAYER-lib-resolved.csv"), encoding="utf-8"))}
    for f, quals in files.items():
        p = os.path.join(ROOT, f)
        marks = sorted({int(m, 16) for m in MARK.findall(open(p, encoding="utf-8", errors="replace").read())}) \
            if os.path.isfile(p) else []
        vto = set().union(*[vtown.get(q[:-2], set()) for q in quals]) if vtown else set()
        own = lambda ad: ad in vto or any(q in names.get(ad, "") for q in quals)
        fixed = FIXED_5F if a.row == "5f" else FIXED
        seeds = list(fixed[f]) if f in fixed else (densest(marks, SPAN[a.row]) or densest(
            sorted(ad for ad in set(names) | vto if own(ad)), SPAN[a.row] if vtown else 0x2000))
        if not seeds:
            print(f"{f}: no marker and no oracle name")
            continue
        lo, hi = bisect.bisect_left(starts, min(seeds)), bisect.bisect_left(starts, max(seeds))
        # widen over neighbours named for the file's classes, allowing GAP unnamed functions between
        while True:
            d = next((i for i in range(1, GAP + 1) if lo - i >= 0 and own(starts[lo - i])), 0)
            if not d:
                break
            lo -= d
        while True:
            d = next((i for i in range(1, GAP + 1) if hi + i < len(starts) and own(starts[hi + i])), 0)
            if not d:
                break
            hi += d
        n = k = 0
        for ad, sz in funcs[lo:hi + 1]:
            if ad in seen:
                continue
            seen.add(ad)
            e = score.get(ad)
            s = "-" if e is None else ("100" if e["matching"] >= 1 or e.get("effective") else f"{e['matching']*100:.1f}")
            rows.append((f, f"0x{ad:08x}", sz, s, "M" if ad in marks else "", names.get(ad, "")))
            n += 1
            k += s == "100"
        tot += n
        ok += k
        out = [m for m in marks if not starts[lo] <= m <= starts[hi]]
        if out:
            print(f"  {f}: markers outside the .obj cluster: " + " ".join(f"0x{m:08x}" for m in out))
        print(f"{f}: 0x{starts[lo]:08x}-0x{starts[hi]:08x}  {k}/{n} at 100%, {len(marks)} markers")
    print(f"TOTAL {ok}/{tot} = {100*ok/max(tot,1):.1f}%")
    if a.csv:
        with open(a.csv, "w", newline="") as o:
            w = csv.writer(o)
            w.writerow(["file", "address", "size", "score", "marker", "oracle_name"])
            w.writerows(rows)


if __name__ == "__main__":
    main()
