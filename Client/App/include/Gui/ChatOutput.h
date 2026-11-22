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

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x0096da50
// TEMPLATE: STUDIO 0x00a1c560
// ??1CharacterChats@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x0096dad0
// TEMPLATE: STUDIO 0x00a1c5e0
// ??$bind@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@PAV12@U?$arg@$01@5@V45@V65@@boost@@YA?AV?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@0@P8ChatOutput@RBX@@AEXPAVAdorn@4@V?$weak_ptr@$$CBVInstance@RBX@@@0@V?$weak_ptr@VPartInstance@RBX@@@0@@ZPAV34@U?$arg@$01@0@12@Z
// TEMPLATE: PLAYER 0x0096dbf0
// TEMPLATE: STUDIO 0x00a1c700
// ??0?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@QAE@ABV012@@Z
// TEMPLATE: PLAYER 0x0096dc50
// TEMPLATE: STUDIO 0x00a1c760
// ??0?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@boost@@QAE@V?$value@PAVChatOutput@RBX@@@12@U?$arg@$01@2@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@12@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@12@V?$value@_N@12@V?$value@VVector3@G3D@@@12@5@Z
// TEMPLATE: PLAYER 0x0096dde0
// TEMPLATE: STUDIO 0x00a1c8f0
// ?push_front@?$deque@V?$shared_ptr@VChatLine@RBX@@@boost@@V?$allocator@V?$shared_ptr@VChatLine@RBX@@@boost@@@std@@@std@@QAEXABV?$shared_ptr@VChatLine@RBX@@@boost@@@Z
// TEMPLATE: PLAYER 0x0096df30
// TEMPLATE: STUDIO 0x00a1ca40
// ?manager@?$functor_manager@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@@function@detail@boost@@CAXABTfunction_buffer@234@AAT5234@W4functor_manager_operation_type@234@U?$bool_@$0A@@mpl@4@@Z
// TEMPLATE: PLAYER 0x0096dff0
// TEMPLATE: STUDIO 0x00a1cb00
// ??$assign_functor@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@@?$basic_vtable2@XPAVBillboardGui@RBX@@PAVAdorn@2@@function@detail@boost@@ABEXV?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@3@AATfunction_buffer@123@U?$bool_@$0A@@mpl@3@@Z
// TEMPLATE: PLAYER 0x0096f0e0
// TEMPLATE: STUDIO 0x00a1de30
// ??0?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@QAE@ABV012@@Z
// TEMPLATE: PLAYER 0x0096f1e0
// TEMPLATE: STUDIO 0x00a1df30
// ?manager@?$functor_manager@V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@@function@detail@boost@@CAXABTfunction_buffer@234@AAT5234@W4functor_manager_operation_type@234@U?$bool_@$0A@@mpl@4@@Z
// TEMPLATE: PLAYER 0x0096f2a0
// TEMPLATE: STUDIO 0x00a1dff0
// ??$assign_to@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@@?$basic_vtable2@XPAVBillboardGui@RBX@@PAVAdorn@2@@function@detail@boost@@QBE_NV?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@3@AATfunction_buffer@123@@Z
// TEMPLATE: PLAYER 0x0096f310
// TEMPLATE: STUDIO 0x00a1e060
// ??$assign_functor@V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@@?$basic_vtable2@XPAVBillboardGui@RBX@@PAVAdorn@2@@function@detail@boost@@ABEXV?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@3@AATfunction_buffer@123@U?$bool_@$0A@@mpl@3@@Z
// TEMPLATE: PLAYER 0x0096f3a0
// TEMPLATE: STUDIO 0x00a1e0f0
// ??0?$deque@V?$shared_ptr@VChatLine@RBX@@@boost@@V?$allocator@V?$shared_ptr@VChatLine@RBX@@@boost@@@std@@@std@@QAE@ABV01@@Z
// TEMPLATE: PLAYER 0x0096f440
// TEMPLATE: STUDIO 0x00a1e190
// ?_Erase@?$_Tree@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@IAEXPAU_Node@?$_Tree_nod@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@2@@Z
// TEMPLATE: PLAYER 0x0096f860
// TEMPLATE: STUDIO 0x00a1e440
// ??$assign_to@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@@?$function2@XPAVBillboardGui@RBX@@PAVAdorn@2@@boost@@AAEXV?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@1@@Z
// TEMPLATE: PLAYER 0x0096fc90
// TEMPLATE: STUDIO 0x00a1e870
// ??$connect@V?$bind_t@XV?$mf1@XVChatOutput@RBX@@ABVHeartbeat@2@@_mfi@boost@@V?$list2@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@@?$signal@$$A6AXABVHeartbeat@RBX@@@Z@signals@rbx@@QAE?AVconnection@12@ABV?$bind_t@XV?$mf1@XVChatOutput@RBX@@ABVHeartbeat@2@@_mfi@boost@@V?$list2@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@@Z
// TEMPLATE: PLAYER 0x0096fd20
// TEMPLATE: STUDIO 0x00a1e900
// ??$connect@V?$bind_t@XV?$mf1@XVChatOutput@RBX@@ABVChatMessage@Network@2@@_mfi@boost@@V?$list2@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@@?$signal@$$A6AXABVChatMessage@Network@RBX@@@Z@signals@rbx@@QAE?AVconnection@12@ABV?$bind_t@XV?$mf1@XVChatOutput@RBX@@ABVChatMessage@Network@2@@_mfi@boost@@V?$list2@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$00@3@@_bi@3@@_bi@boost@@@Z
// TEMPLATE: PLAYER 0x0096fdb0
// TEMPLATE: STUDIO 0x00a1e990
// ??$connect@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@V?$shared_ptr@VInstance@RBX@@@boost@@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4ChatColor@ChatService@2@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$00@3@U?$arg@$01@3@U?$arg@$02@3@@_bi@3@@_bi@boost@@@?$signal@$$A6AXV?$shared_ptr@VInstance@RBX@@@boost@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4ChatColor@ChatService@RBX@@@Z@signals@rbx@@QAE?AVconnection@12@ABV?$bind_t@XV?$mf3@XVChatOutput@RBX@@V?$shared_ptr@VInstance@RBX@@@boost@@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@W4ChatColor@ChatService@2@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$00@3@U?$arg@$01@3@U?$arg@$02@3@@_bi@3@@_bi@boost@@@Z
// TEMPLATE: PLAYER 0x0096fe40
// TEMPLATE: STUDIO 0x00a1ea20
// ??$?0V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@@?$function2@XPAVBillboardGui@RBX@@PAVAdorn@2@@boost@@QAE@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@1@H@Z
// TEMPLATE: PLAYER 0x0096fef0
// TEMPLATE: STUDIO 0x00a1ead0
// ??$assign_to@V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@@?$basic_vtable2@XPAVBillboardGui@RBX@@PAVAdorn@2@@function@detail@boost@@QBE_NV?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@3@AATfunction_buffer@123@@Z
// TEMPLATE: PLAYER 0x00970310
// TEMPLATE: STUDIO 0x00a1eef0
// ?erase@?$_Tree@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@0@Z
// TEMPLATE: PLAYER 0x009703e0
// TEMPLATE: STUDIO 0x00a1efc0
// ??0_Node@?$_Tree_nod@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@QAE@PAU012@00ABU?$pair@QBVInstance@RBX@@UCharacterChats@2@@2@D@Z
// TEMPLATE: PLAYER 0x00970480
// TEMPLATE: STUDIO 0x00a1f060
// ??$?0V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@boost@@@?$function@$$A6AXPAVBillboardGui@RBX@@PAVAdorn@2@@Z@boost@@QAE@V?$bind_t@XV?$mf3@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@@_mfi@boost@@V?$list4@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@@_bi@3@@_bi@1@H@Z
// TEMPLATE: PLAYER 0x009704f0
// TEMPLATE: STUDIO 0x00a1f0d0
// ??$assign_to@V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@@?$function2@XPAVBillboardGui@RBX@@PAVAdorn@2@@boost@@AAEXV?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@1@@Z
// TEMPLATE: PLAYER 0x009705a0
// TEMPLATE: STUDIO 0x00a1f180
// ?_Buynode@?$_Tree@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@IAEPAU_Node@?$_Tree_nod@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@2@PAU342@00ABU?$pair@QBVInstance@RBX@@UCharacterChats@2@@2@D@Z
// TEMPLATE: PLAYER 0x00970630
// TEMPLATE: STUDIO 0x00a1f210
// ??$?0V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@@?$function2@XPAVBillboardGui@RBX@@PAVAdorn@2@@boost@@QAE@V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@1@H@Z
// TEMPLATE: PLAYER 0x009708d0
// TEMPLATE: STUDIO 0x00a1f4b0
// ??$?0V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@boost@@@?$function@$$A6AXPAVBillboardGui@RBX@@PAVAdorn@2@@Z@boost@@QAE@V?$bind_t@XV?$mf6@XVChatOutput@RBX@@PAVAdorn@2@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@V?$weak_ptr@VPartInstance@RBX@@@5@_NVVector3@G3D@@V78@@_mfi@boost@@V?$list7@V?$value@PAVChatOutput@RBX@@@_bi@boost@@U?$arg@$01@3@V?$value@V?$weak_ptr@$$CBVInstance@RBX@@@boost@@@23@V?$value@V?$weak_ptr@VPartInstance@RBX@@@boost@@@23@V?$value@_N@23@V?$value@VVector3@G3D@@@23@V823@@_bi@3@@_bi@1@H@Z
// TEMPLATE: PLAYER 0x00970f80
// TEMPLATE: STUDIO 0x00a1fb80
// ?insert@?$_Tree@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@QAE?AU?$pair@Viterator@?$_Tree@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@_N@2@ABU?$pair@QBVInstance@RBX@@UCharacterChats@2@@2@@Z
// TEMPLATE: PLAYER 0x00971e60
// TEMPLATE: STUDIO 0x00a20a60
// ?insert@?$_Tree@V?$_Tmap_traits@PBVInstance@RBX@@UCharacterChats@2@U?$less@PBVInstance@RBX@@@std@@V?$allocator@U?$pair@QBVInstance@RBX@@UCharacterChats@2@@std@@@5@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@ABU?$pair@QBVInstance@RBX@@UCharacterChats@2@@2@@Z
// clang-format on
