/* Gui/ChatLine.h -- October 2013 (chat era; C++-only, no reflection).
   Evidence: ChatLine ctor 0x5BEF3C (187B, mangle
     __ZN3RBX8ChatLineC2ENS0_8ChatTypeERKSsfNS0_11BubbleColorEb);
     PlayerChatLine ctor 0x5BEFFA (510B; new(0x38) in onPlayerChatMessage
     0x5C2A96); GameChatLine ctor 0x5BF1FA (215B); ChatLine::getOrigin
     0x5C5E88 (54B); D1 0x5C6902 / D0 0x5C690C; partial dtor 0x5C5D0C
     (99B).  ELIPSES static @0xF80C34 (mangle __ZN3RBX8ChatLine7ELIPSESE)
     holds a pointer to the literal "..." @0xB20CA6.
   Member layout from the ctor member writes -- GCC offsets; MSVC layout
   DELTA (std::string is a 4-byte cow pointer in the binary, larger under
   MSVC -- member ORDER is the invariant):
     +0 vptr | +4 std::string text | +8 BubbleColor | +12 ChatType |
     +16 float clockAtInsert (the ChatOutput clock float, +300, passed as
     the ctor float from onPlayerChatMessage) | +20 float countdown (ctor:
     flag ? fminf(1.0f, len/75.0f)*7.0f+8.0f : fminf(1.0f,
     len/75.0f)*8.0f+12.0f) | +24 bool | +28 weak_ptr<Instance> origin
     (px@+28, weak count@+32 -- probe6; the nothrow weak ctor is the
     consumer path).
   PlayerChatLine (56B): +36..+47 Color3 (team game -> team brick color,
     white when neutral, black when teamed-out; else hash(name)&7 ->
     colorFromIndex8; null player -> ROBLOXNAME static + default color),
     +48 std::string playerName (Instance::fw(player)+24), +52 float init
     60.0f (0x42700000).
   GameChatLine: stores its own origin; ChatType 4 when the origin isA
     ModelInstance else 5; player lines 0..3 pass through the identity
     remap table @0xC5EED0 in onPlayerChatMessage.
   DELTAs: ChatType/BubbleColor enumerator NAMES unresolved in dbq24;
     onHeartbeat countdown Decay uses flag@+24 (removeExpired archaeology).
   TODO: PlayerChatLine/GameChatLine ctor bodies + ROBLOXNAME static
      (ChatLine ctor body landed in ChatOutput.cpp);
     map<BubbleColor, shared_ptr<GuiObject>> surface.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/Gui/ChatLine.h */

#pragma once

#include "V8Tree/Instance.h"
#include "Network/Player.h"
#include "G3D/Color3.h"
#include "V8DataModel/BillboardGui.h"
#include <deque>

namespace RBX {

	class ChatLine
	{
	public:
		// DELTA: NAMES unresolved; values pinned (ctor 0x5BEF3C stores the
		// ordinal at +12; GameChatLine supplies 4/5; the 0..3 player domain
		// arrives via ChatMessage::ChatType through the identity remap).
		// RENDER EVIDENCE (classic painter sub_5C4419, landed in
		// ChatOutput.cpp -- gate ga0): TYPE_1 prepends "(TEAM) ", TYPE_2
		// prepends "(whisper) ", TYPE_3 recolors the line with the
		// PlayerChatLine color3 bytes at +36..47 (alpha stays 1.0f, no
		// prefix), TYPE_0/default renders plain white with no prefix.
		enum ChatType
		{
			CHATLINE_TYPE_0 = 0,
			CHATLINE_TYPE_1,
			CHATLINE_TYPE_2,
			CHATLINE_TYPE_3,
			CHATLINE_TYPE_GAME_MODEL = 4,
			CHATLINE_TYPE_GAME_OTHER = 5
		};

		// DELTA: value NAMES decode pending; the ORDINALS are pinned by
		// the consumers: onGameChatMessage 0x5C2E08-0x5C2E2D maps
		// ChatColor{0,1,2} -> {1,2,3} else 0, and the ctor copies
		// BubbleColor bytes into +8 -- so the ordinal range 0..3 spans
		// the map image (the render arcs will name them).
		// RENDER EVIDENCE (renderBubbles 0x5C4EC0, landed in ChatOutput.cpp
		// -- gate999z): the tail/backing color switch pins 0 = gray
		// 0.745098f x3, 1 = #2D8EF5 blue, 2 = green 0/0.745098f/0,
		// 3 = #FF584F red; names remain DELTA.
		enum BubbleColor
		{
			BUBBLECOLOR_0 = 0,
			BUBBLECOLOR_1,
			BUBBLECOLOR_2,
			BUBBLECOLOR_3
		};

		static const char* ELIPSES;	// 0xF80C34 -> "..." (InitFunc_414
			// mac: 0x5CAC50 sets 127 - strlen(ChatLine::ELIPSES) = 124).

		ChatLine(ChatType chatType, const std::string& text, float clock,
			BubbleColor color, bool flag);	// 0x5BEF3C
		// STUB: PLAYER 0x00970b70
		// STUB: STUDIO 0x00a1f750
		virtual ~ChatLine() {}	// D1 0x5C6902 / D0 0x5C690C

		std::string text;                  // +4
		BubbleColor bubbleColor;           // +8
		ChatType chatType;                 // +12
		float clockAtInsert;               // +16
		float countdown;                   // +20 (onHeartbeat decay target)
		bool flag0x18;                     // +24
		weak_ptr<Instance> origin;         // +28 px / +32 weak count
		// (probe6 ground truth: the +28/+32 pair = the WEAK pair -- the
		// binary's line consumers lock it through the nothrow weak ctor;
		// ChatOutput's ctor/shared writes here ride the implicit
		// shared->weak operator= -- this->origin = origin.)

		// mac: 0x5C5E88 (54B): the sp_nothrow weak lock of the +28 pair --
		// locked shape per the probe6 + the render-arc decode.
		shared_ptr<Instance> getOrigin() const { return origin.lock(); }
	};

	// PlayerChatLine 0x5BEFFA (510B; alloc new(0x38) in onPlayerChatMessage
	// mac: 0x5C2A96).  Base 36B + :
	//   +36..+47 G3D::Color3 bubble color (team game -> Team/BrickColor
	//     arm; white when player->neutral; black when teamed-out; else
	//     hash(name)&7 color arm; null player -> ROBLOXNAME static arm)
	//   +48 std::string playerName (Instance::fw(player.px)+24 -- the
	//     landed Instance::getName() reads the same FWInstance+24 string)
	//   +52 float ctor init 60.0f (0x42700000; removeExpired 0x5C2FFA/
	//     mac: 0x5C2FFF reads it for the +236-deque front expiry -- the +236
	//     deque carries only PlayerChatLine: onPlayerChatMessage pushes
	//     both deques, onGameChatMessage pushes only the map's (raw
	//     mac: 0x5C2EF1..0x5C2F28 tail has no +236 push)).
	// DELTAs: ctor BODY parked (Teams::isTeamGame/getTeamFromPlayer are
	// still a shell; Hash::hash / Color::colorFromIndex8 /
	// Color::getColorByIndex unported; ROBLOXNAME static unresolved).
	// Member names unresolved except as behavior-pinned.
	class PlayerChatLine : public ChatLine
	{
	public:
		PlayerChatLine(ChatType type, shared_ptr<Network::Player> player,
			const std::string& text, float clock, bool flag);	// 0x5BEFFA

		// The null-player name: the static pointer slot @0xF80C30 reads
		// the "(ROBLOX)" data @0xB726DA (ctor 0x5BF118 + strlen +
		// assign(char const*, len)); landed defines it in ChatOutput.cpp.
		static const char* const ROBLOXNAME;

		G3D::Color3 color3;       // +36..+47
		std::string playerName;   // +48
		float lineLifetime;       // +52 (expiry seat read by removeExpired)
	};

	// GameChatLine 0x5BF1FA (215B; alloc new(0x24) in onGameChatMessage
	// raw 0x5C2E3D -- 36 bytes, exactly the base).  Body parked: the
	// Reflection isA(ModelInstance) probe supplies ChatType
	// 4|5 (0x5BE...); origin/text/flag/color route to the base fields.
	class GameChatLine : public ChatLine
	{
	public:
		GameChatLine(shared_ptr<Instance> origin, const std::string& text,
			float clock, bool flag, BubbleColor color);	// 0x5BF1FA
	};

	// Value of ChatOutput's per-character map (map<Instance const*,
	// CharacterChats>, operator[] 0x5C6128 -- the empty pair insert
	// zero-fills beyond the deque; base = RBX::CharacterChats).
	// Layout pinned by onHeartbeat 0x5C298C raw arithmetic (node value
	// starts node+0x14, so CC offsets = node - 0x14):
	//   +0  std::deque<shared_ptr<ChatLine>> lines (GCC 40B; MSVC DELTA
	//       -- order invariant)
	//   +40 bool decay flag: set -> every ChatLine countdown (+20)
	//       decays by dt each heartbeat (read 0x5C2A49)
	//   +41 bool decay-triple flag: with ChatLine flag (+24) set, that
	//       line decays by 3.0*dt instead (read 0x5C2A42/5C2A47)
	//   +44 weak_ptr<BillboardGui> billboard (the removeExpired erase
	//       arm 0x5C30AD-C4 locks it with the no-throw weak->shared
	//       ctor and setParentInternal(NULL,false) when the entry's
	//       deque empties; landed weak_ptr is MSVC-sized -- DELTA,
	//       order invariant)
	// DELTAs: member names unresolved; behavior-pinned.  The operator[]
	// insert zero-fills the pair value, so fields past +51 exist in the
	// binary but their reads/consumers land with the render arcs.
	struct CharacterChats
	{
		std::deque<shared_ptr<ChatLine>> lines;
		bool flagDecay;      // +40
		bool flagDecay3;     // +41 (triples the decay for flagged lines)
		weak_ptr<BillboardGui> billboard;   // +44
	};

} // namespace RBX
