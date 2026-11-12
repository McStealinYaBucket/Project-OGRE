/* gui/ChatOutput.h -- October 2013 C++-only widget (NO reflection).
   Evidence: C2(void) 0x5BF2DC (6833B: 5 maps + billboard callbacks);
     D2 0x5C2308 (828B).  Member map from decompile:
       +0   GuiItem base
       +28  Network::Players* players
       +116 std::map (ChatLine::BubbleColor -> ScalingInfo) bubbles
       +140 std::map (unknown)
       +164 std::map (unknown)
       +188 std::map (unknown)
       +212 std::map (unknown)
       +280 std::map/vector (unknown)
       +304 rbx::signals::connection heartbeatConn
       +308 rbx::signals::connection chatMessageConn
       +312 rbx::signals::connection chattedConn
   TODO: full decompile (member map + init/render/process bodies).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatOutput.h */

#pragma once

#include "V8DataModel/Item.h"
#include "Network/Players.h"
#include "V8DataModel/RunService.h"
#include "V8DataModel/ChatService.h"
#include "Gui/ChatLine.h"
#include "V8DataModel/PartInstance.h"
#include "G3D/Vector3.h"
#include <boost/weak_ptr.hpp>
#include <map>
#include <vector>
#include <deque>

namespace RBX {

	class GuiObject;  // the template-map value type (shared_ptr<GuiObject>); lands with GuiObject.h via the cpp include chain.

	class ChatOutput
		: public GuiItem
	{
	public:
		// C2(void) 0x5BF2DC (6833B): GuiItem base ctor + explicit zero of
		// players @+112 (C2 line 245) and clock @+300 (line 302) + five
		// empty bubble-template maps + deque _M_initialize_map + the
		// chats-map header; body landed in gui/ChatOutput.cpp.
		ChatOutput();
		virtual ~ChatOutput() {}

		// mac: 0x5C2774: onServiceProvider (body landed in ChatOutput.cpp;
		// rarc28 decomp 60-136 + raw 139-299 -- the three-connection
		// refresh + the NULL-provider players-null arm).
		virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);

		// mac: 0x5C298C: onHeartbeat -- real signature from the mangle
		// __ZN3RBX10ChatOutput11onHeartbeatERKNS_9HeartbeatE (Heartbeat =
		// RBX::Heartbeat, 5 doubles; body reads heartbeat.wallStep @+8; stubbed).
		virtual void onHeartbeat(const Heartbeat& heartbeat);

		// mac: 0x5C2A96: onPlayerChatMessage -- real signature from the Sanitize
		// xref: __ZN3RBX10ChatOutput19onPlayerChatMessageERKNS_7Network11ChatMessageE
		// (body decoded; needs PlayerChatLine ctor + deque machinery to land).
		void onPlayerChatMessage(const Network::ChatMessage& message);

		// mac: 0x5C2DB8: onGameChatMessage -- mangle
		// __ZN3RBX10ChatOutput17onGameChatMessageEN5boost10shared_ptrINS_8InstanceEEERKSsNS_11ChatService9ChatColorE
		// (shared_ptr BY VALUE, const string&, ChatService::ChatColor).
		void onGameChatMessage(shared_ptr<Instance> origin, const std::string& text, ChatService::ChatColor color);

		// mac: 0x5C2678 (252B): SanitizeChatLine -- non-static member, string
		// return (sret shape in the raw asm: first slot = sret, second =
		// this, unread).  Body landed in ChatOutput.cpp.
		std::string SanitizeChatLine(const std::string& text);

		// mac: 0x5C2FC6: removeExpired (stubbed).
		bool removeExpired();

		// mac: 0x5C316C createBillboardGuiHelper (710B): the raw ends with a bare
		// retn (both call sites PCM 0x5C2CD3 / GCM 0x5C2EFB discard eax) --
		// void return.
		void createBillboardGuiHelper(class RBX::Instance* inst, bool a3);

		// mac: 0x5C3432: render2d (stubbed).
		void render2d(class RBX::Adorn* adorn);

		// mac: 0x5C3498: render2d_bubbleStyle (stubbed).
		void render2d_bubbleStyle(class RBX::Adorn* adorn, bool v);

		// mac: 0x5C426C: render2d_classicStyle (body landed in ChatOutput.cpp;
		// the file-static renderClassicLineList wing = sub_5C4419,
		// DELTA-named -- the per-line "PlayerName; text" pass).
		void render2d_classicStyle(class RBX::Adorn* adorn, bool v);

		// mac: 0x5C4B88: renderBubbleImposters -- the mf3 bind from the
		// mac: 0x5C3B04 imposters arm (the bound list = this, _2, weak-inst,
		// weak-part): the Adorn arrives at the bg-set time via _2.
		void renderBubbleImposters(class RBX::Adorn* adorn,
			boost::weak_ptr<Instance const> origin,
			boost::weak_ptr<PartInstance> part);

		// mac: 0x5C4EC0: renderBubbles -- the mf6 bind from the bubbles arm
		// (this, _2, weak-inst, weak-part, a4, V(0, yShift, 0), V(0, 0,
		// fade)): six call-arg params after the object.
		void renderBubbles(class RBX::Adorn* adorn,
			boost::weak_ptr<Instance const> origin,
			boost::weak_ptr<PartInstance> part, bool a4,
			G3D::Vector3 offset, G3D::Vector3 fade);

		// Member map (from C2 0x5BF2DC decompile + onHeartbeat/
		// removeExpired raw arithmetic; GCC offsets -- MSVC layout DELTA,
		// member ORDER is the invariant):
		// players @+112 (dword index 28): C2 zeroes it (line 245
		// `*(_DWORD*)this+28 = 0`); onServiceProvider 0x5C2774 stores
		// ServiceProvider::create<Network::Players>().  Read sites:
		// onPlayerChatMessage 0x5C2B69 reads *(players+188) = the local
		// player; onGameChatMessage 0x5C2DF1-2 reads *( *(players+188)
		// + 0x5C ) = the local player's character (landed localPlayer /
		// Player::character).
		Network::Players* players;                             // +112.

		// C2 0x5BF2DC bubble-template maps -- one constructed template per
		// BubbleColor at C2 time (arm store order: A/B/C = keys 0,1,3,2;
		// E = keys 0,1,2,3).  Empty-map headers proven inline per map
		// (C2 lines 246-280/295-299): 6 dwords = 24B (compare slot,
		// color, parent, left/right -> the color slot, node count).
		std::map<ChatLine::BubbleColor, shared_ptr<GuiObject>> bubbleBackings;       // +116 (C2 A-arms 0x5BF5CB.., fac1 e.g. "chatBubble_white_bkg")
		std::map<ChatLine::BubbleColor, shared_ptr<GuiObject>> bubbleTails;          // +140 (C2 B-arms 0x5BF6.., fac2 tail-image Scale9Frames)
		std::map<ChatLine::BubbleColor, shared_ptr<GuiObject>> bubbleTailFrames;     // +164 (C2 C-arms 0x5BF7xx.., fac3 TailFrame + tail)

		// ChatOutput::ScalingInfo -- the +188 map's value type (name from
		// the D2 0x5C2308 _M_erase mangle
		// map<ChatLine::BubbleColor, RBX::ChatOutput::ScalingInfo>).
		// DELTA: field layout undecoded -- no decompiled code path touches
		// its fields yet; values arrive with the render2d_* bubble arcs.
		// (Declared before use: MSVC wants the map's value type complete
		// at the member declaration.)
		struct ScalingInfo
		{
			ScalingInfo() {}
		};

		std::map<ChatLine::BubbleColor, ScalingInfo> bubbleScaling;                  // +188 (runtime-populated)
		std::map<ChatLine::BubbleColor, shared_ptr<GuiObject>> bubbleNotifications;  // +212 (C2 E-arms 0x5BFFA4.., fac4 DialogPlaceholder templates)

		std::deque<shared_ptr<ChatLine>> activeLines;          // +236 (C2 _M_initialize_map(that,0) line 294; 40B = +236..275)
		std::map<Instance const*, CharacterChats> chats;       // +276 (header color@+280, left/right -> this+280, node count @+296; operator[] 0x5C6128)
		float clock;                                           // +300 (C2 zeroes dword 75, line 302; += heartbeat.wallStep at 0x5C29AB)
		rbx::signals::connection heartbeatConn;  // +304
		rbx::signals::connection chatMessageConn;  // +308
		rbx::signals::connection chattedConn;  // +312
	};

} // namespace RBX