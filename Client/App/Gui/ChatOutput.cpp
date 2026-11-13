/* App/gui/ChatOutput.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "Gui/ChatOutput.h"
#include "Gui/ChatLine.h"
#include "Gui/GUI.h"
#include "V8DataModel/Item.h"
#include "V8Tree/Service.h"
#include "V8DataModel/RunService.h"
#include "Network/Players.h"
#include "V8DataModel/ChatService.h"
#include "V8DataModel/GameSettings.h"
#include "V8DataModel/ModelInstance.h"
#include "V8DataModel/PartInstance.h"
#include "V8DataModel/CoreGuiService.h"
#include "Util/Object.h"
#include "Util/FastFlags.h"
#include "Gui/ProfanityFilter.h"
#include "V8DataModel/Teams.h"
#include "V8DataModel/Team.h"
#include "Util/BrickColor.h"
#include "Util/Color.h"
#include "Util/Hash.h"
#include "V8DataModel/Scale9Frame.h"
#include "V8DataModel/ImageLabel.h"
#include "V8DataModel/Frame.h"
#include "Util/ContentId.h"
#include "G3D/Vector2int16.h"
#include "V8DataModel/Workspace.h"
#include "V8DataModel/Camera.h"
#include "Gui/AdornBillboarder.h"
#include "Gui/ViewportBillboarder.h"
#include "Util/UDim.h"
#include "V8DataModel/GuiObject.h"
#include "RbxG3D/Frustum.h"
#include "G3D/g3dmath.h"
#include "rbx/Debug.h"
#include <boost/bind.hpp>
#include <string.h>
#include <math.h>
#include <vector>
#include "V8DataModel/TextService.h"
#include "G3D/Color4.h"

namespace RBX {

	// ChatLine static from InitFunc_414 0x5CAC50 (file static init):
	// dword_FB87A8 = 127 - strlen(ChatLine::ELIPSES) -- the
	// SanitizeChatLine truncation cap; with the "..." (3 chars) appended,
	// the maximum model line is 127 chars.
	const char* ChatLine::ELIPSES = "...";

	// The 0xFB87A8 constant of SanitizeChatLine (raw asm:
	// mov ecx, [edi+0x9F6122] -> *(0xFB87A8); cmp text.length, ecx; jbe
	// returns the plain copy).  Mirrors the binary's strlen-at-static-init.
	static const size_t kChatLineLimit = 127 - strlen(ChatLine::ELIPSES);	// = 124

	// mac: 0x5BEF3C (187B; mangle __ZN3RBX8ChatLineC2ENS0_8ChatTypeERKSsfNS0_11-
	// BubbleColorEb).  Member order: text copy ctor (+4), BubbleColor
	// (+8), ChatType (+12), clockAtInsert (+16), countdown (+20), flag
	// (+24), origin zero-init (+28/+32).  countdown = flag ?
	// fminf(1.0f, len/75.0f)*7.0f+8.0f : fminf(1.0f, len/75.0f)*8.0f+12.0f
	// -- the decompile routed text.length() through the GCC int->double
	// COW conversion pipe; (float)text.length() is the same value.
	ChatLine::ChatLine(ChatType type, const std::string& text, float clock, BubbleColor color, bool flag)
	{
		ChatLine::text = text;
		bubbleColor = color;
		chatType = type;
		clockAtInsert = clock;
		float lenOver75 = (float)text.length() / 75.0f;
		// fminf(1.0f, len/75.0f) spelled via ternary per the VS2012
		// toolchain rule (GUI.h/Contact.h convention).
		float ratio = 1.0f < lenOver75 ? 1.0f : lenOver75;
		if (flag)
			countdown = ratio * 7.0f + 8.0f;
		else
			countdown = ratio * 8.0f + 12.0f;
		flag0x18 = flag;
	}

	// mac: 0x5BF1FA (215B; mangle __ZN3RBX12GameChatLineC2EN5boost10shared_ptr
	// INS_8InstanceEEERKSsfbNS_8ChatLine11BubbleColorE).  ChatType =
	// 5 unless origin.px != 0 AND the instance's cached ClassDescriptor
	// (Instance+0x30, probed at 0x5BF21D) isA(ModelInstance descriptor
	// 0xEAF1A8, ClassDescriptor::isA 0x157C0E at 0x5BF23B) -> 4 (0x5BF24A
	// `or ebx, 4`).  Then the base ctor (type, text, clock, color, flag)
	// at 0x5BF282, then the origin px copy @+28 (0x5BF29A) and the count
	// assign (+32) at 0x5BF2AA -- the landed init list keeps that order.
	GameChatLine::GameChatLine(shared_ptr<Instance> origin, const std::string& text,
		float clock, bool flag, BubbleColor color)
		: ChatLine(origin != NULL
			&& origin->getDescriptor().isA(ModelInstance::classDescriptor())
				? CHATLINE_TYPE_GAME_MODEL : CHATLINE_TYPE_GAME_OTHER,
			text, clock, color, flag)
	{
		// Binary 0x5BF29A/0x5BF2AA: px copy @+28, count assign @+32 --
		// after the base ctor (inherited members are not init-listable
		// from a derived ctor in C++).
		this->origin = origin;
	}

	// mac: 0x5C4419 (1653B .. 0x5C4A70 ret + the 0x5C4A7B unwind pads;
	// unnamed in the binary = the render2d_classicStyle wingman sub_,
	// DELTA-named renderClassicLineList here): the whole
	// "PlayerName; text" per-line classic pass.  __userpurge decode
	// (rarc28 raw 2052-2455): ecx = the Adorn*, edx = &region (only
	// low.x/low.y are read -- the high pair carries caller junk),
	// [esp] = &std::deque<shared_ptr<ChatLine>>, [esp+4] = &const
	// G3D::Color4 backdrop (the translucentBackdrop() singleton), bare
	// retn (the arg slots live in the caller's 0x6C frame,
	// mac: 0x5C4387-93/0x5C43FA-45; the shared call = 0x5C440C).
	// Per line (the loop head 0x5C44E7-0x5C4518 re-folds size() every
	// iteration = the source expression `lines.size()`; exit i >= size;
	// the element = the fresh advance 0x5C4537):
	//   a) name = std::string(+0x30).append("; ", 2) (0x5C4450/0x4567).
	//      The +0x30 read is UNCONDITIONAL -- the CC/NULL-key deques can
	//      also carry GameChatLine (36B) elements, and the binary still
	//      reads their +0x30 bytes (shipped shape, garbage or not).
	//   b) the two +0x48 measures (0x5C45A7/0x5C45E7): sret V2 extents,
	//      12.0f, font 0, &Vector2::zero() -- name first, then the RAW
	//      line->text (whose .y stanza is never read).
	//   c) colX = max(20.0f, nameExtent.x + 4.0f) (0x5C45F1-0x5C4605;
	//      C408B8 = 20.0f column floor, CA2400 = 4.0f pad).
	//   d) the backdrop band @0x5C460D-0x5C46A6: a Rect pair
	//      canonicalization over {x0-2.0f, textExtent.x+colX+x0+2.0f} x
	//      {y0+1.0f, nameExtent.y+y0-1.0f} (C1E5B0/C1E54C/C1C918/CA245C),
	//      then fillRectangle (vtbl+0x40) with the backdrop&.
	//   e) row color default = the white Color4, alpha 1.0f
	//      (0x5C46A8-0x46D1), text2 = line->text (the 0x5C46D3 copy);
	//      the +12 switch @0x5C46E2: TYPE_1 -> "(TEAM) " + text2
	//      (0x5C4753), TYPE_2 -> "(whisper) " + text2 (0x5C470A), TYPE_3
	//      -> write the PlayerChatLine color3 r/g/b (+24/28/2C) into the
	//      color, alpha stays 1.0f (0x5C47BC-DB); 4/5 + 0 = untouched.
	//   f) two drawFont2D (+0x4C; the 12-slot stage: sret V2, this,
	//      text&, position&, 10.0f, &color, &Color4::clear(), font 0,
	//      xa 1, ya 0, &zero, &{-1,-1,-1,-1} clip): the name at
	//      (x0, y0) @0x5C48C7, text2 at (colX + x0, y0) @0x5C49E2.
	//   g) row advance += 14.0f (0x5C49E7-0x5C49F4; C57EB8).
	// STUB: PLAYER 0x0096d500
	// STUB: STUDIO 0x00a1c060
	static void renderClassicLineList(RBX::Adorn* adorned, const Rect& region,
		const std::deque<shared_ptr<ChatLine>>& lines, const G3D::Color4& backdrop)
	{
		Rect row = region;  // the binary advances its &region copy in place

		for (size_t i = 0; i < lines.size(); ++i)
		{
			const shared_ptr<ChatLine>& line = *(lines.begin() + i);

			// the unconditional +0x30 name read (see above): the binary
			// never gates the downcast, so neither does the landing.
			PlayerChatLine* pl = static_cast<PlayerChatLine*>(line.get());

			std::string name = pl->playerName;
			name.append("; ", 2);

			G3D::Vector2 nameExtent = adorned->get2DStringBounds(name, 12.0f,
				Text::font0, G3D::Vector2::zero());
			G3D::Vector2 textExtent = adorned->get2DStringBounds(line->text, 12.0f,
				Text::font0, G3D::Vector2::zero());

			float colX = 20.0f < nameExtent.x + 4.0f ? nameExtent.x + 4.0f : 20.0f;

			Rect band;
			{
				const float bandX0 = row.low.x - 2.0f;
				const float bandX1 = textExtent.x + colX + row.low.x + 2.0f;
				const float bandY0 = row.low.y + 1.0f;
				const float bandY1 = nameExtent.y + row.low.y - 1.0f;
				band.low.x = bandX0 < bandX1 ? bandX0 : bandX1;
				band.high.x = bandX0 < bandX1 ? bandX1 : bandX0;
				band.low.y = bandY0 < bandY1 ? bandY0 : bandY1;
				band.high.y = bandY0 < bandY1 ? bandY1 : bandY0;
			}
			adorned->fillRectangle(band, backdrop);   // vtbl +0x40 (0x5C46A6)

			G3D::Color4 color(G3D::Color3::white(), 1.0f);
			std::string text2 = line->text;

			switch (line->chatType)
			{
			case ChatLine::CHATLINE_TYPE_1:
				text2 = "(TEAM) " + text2;     // 0x5C4753
				break;
			case ChatLine::CHATLINE_TYPE_2:
				text2 = "(whisper) " + text2;  // 0x5C470A
				break;
			case ChatLine::CHATLINE_TYPE_3:
				color.r = pl->color3.r;        // 0x5C47BC-DB
				color.g = pl->color3.g;
				color.b = pl->color3.b;
				break;
			default:
				break;
			}

			adorned->drawFont2D(name, G3D::Vector2(row.low.x, row.low.y), 10.0f,
				color, G3D::Color4::clear(), Text::font0, Text::xaLeft,
				Text::yaInvariant, G3D::Vector2::zero(),
				G3D::Rect2D::xyxy(-1.0f, -1.0f, -1.0f, -1.0f));   // 0x5C48C7
			adorned->drawFont2D(text2, G3D::Vector2(colX + row.low.x, row.low.y),
				10.0f, color, G3D::Color4::clear(), Text::font0, Text::xaLeft,
				Text::yaInvariant, G3D::Vector2::zero(),
				G3D::Rect2D::xyxy(-1.0f, -1.0f, -1.0f, -1.0f));   // 0x5C49E2

			row.low.y += 14.0f;   // 0x5C49E7-0x5C49F4 (C57EB8 the stride)
		}
	}

	// mac: 0x5C298C (266B): onHeartbeat -- raw-asm verified.
	//   dt = (float)heartbeat.wallStep (cvtsd2ss from hb+8, a double);
	//   clock @+0x12C += dt; 3.0f*dt precomputed (float 3.0f at
	//   0xCA23FC); walk the +280 map from header._M_left (this+288) to
	//   &header (this+280); per CharacterChats deque element: ChatLine
	//   flag (+24) set && entry.flagDecay3 -> countdown (-= the entry
	//   flagDecay path) -= 3.0f*dt; else entry.flagDecay -> countdown
	//   -= dt; then removeExpired() repeatedly while it returns true.
	//   GCC deque-size arithmetic in the binary = the library size
	//   walk; the landed loop iterates the landed deque directly
	//   (element order identical).
	void ChatOutput::onHeartbeat(const Heartbeat& heartbeat)
	{
		float dt = (float)heartbeat.wallStep;
		clock += dt;
		float dt3 = 3.0f * dt;
		for (std::map<Instance const*, CharacterChats>::iterator it = chats.begin(); it != chats.end(); ++it)
		{
			CharacterChats& entry = it->second;
			for (size_t i = 0; i < entry.lines.size(); ++i)
			{
				ChatLine* line = entry.lines[i].get();
				if (line->flag0x18 != 0 && entry.flagDecay3)
					line->countdown -= dt3;
				else if (entry.flagDecay)
					line->countdown -= dt;
			}
		}
		while (removeExpired())
		{
		}
	}

	// mac: 0x5BEFFA (512B incl. the unwinder coda; mangle
	// __ZN3RBX14PlayerChatLineC2ENS_8ChatLine8ChatTypeEN5boost10shared_
	// ptrINS_7Network6PlayerEEERKSsfb): base ctor (type, text, clock,
	// BubbleColor_0 = the [esp+0x10] zero store at 0x5BF030, flag)
	// through the 0x5BF038 call; playerName (+48) string("") arm
	// mac: 0x5BF04E-62; lineLifetime (+52) = 0x42700000 = 60.0f (0x5BF06A);
	// a player arm 0x5BF07B: playerName.assign(*(fw(player) + 24) =
	// Instance+24 = landed getName) and origin-copy 0x5BF095-0x5BF0C5
	// (character = *(player.px+92)/+96 -> ChatLine +28 px / +32 pi via
	// the weak_count op= form -- landed shared_ptr::operator= writes
	// the same field pair); null player -> playerName.assign((const
	// char*)(*0xF80C30), strlen) -- the ROBLOXNAME static slot whose
	// data reads "(ROBLOX)" (0xB726DA) -- then Color::getColorByIndex
	// (0, 0x5BF139).  Color tree 0x5BF0D5-0x5BF199: find<Teams>(player),
	// isTeamGame (0x4BBB9A), neutral byte [player+104] -> getTeamFrom-
	// Player (0x4BBF28)/getTeamColor (0x4BA284 -> +96 BrickColor) ->
	// BrickColor::color3 (0x1E9E6C) -> +36/+40/+44, no-team players ->
	// Hash::hash(playerName) & 7 -> Color::colorFromIndex8 (0x1F39BE),
	// white()/black() statics (0x7560FE/0x755E6C) on the neutral/no-
	// team edges; null player exits via getColorByIndex(0).  All three
	// copy eax..eax+8 into +36..+47.
	const char* const PlayerChatLine::ROBLOXNAME = "(ROBLOX)";

	// mac: 0x5C0D8D (748B) -- ecx = the OUT shared_ptr slot, edx = &sliceBase.
	// The plain "ChatBubble" backing Scale9Frame; 9-slice edges (20, 20)
	// (packed dword 0x00140014 staged @0x5C0E05).
	// STUB: PLAYER 0x0096e420
	// STUB: STUDIO 0x00a1d0a0
	static shared_ptr<Scale9Frame> createChatBubbleFrame(const std::string& sliceBase)
	{
		shared_ptr<Scale9Frame> frame = Creatable<Instance>::create<Scale9Frame>();       // @0x5C0DAA -> create<Scale9Frame> 0x967B04
		frame->setName("ChatBubble");                                                    // vtbl+0x1C; lit @0xB729CE (ref 0x5C0DC6)
		frame->setScaleEdgeSize(G3D::Vector2int16(20, 20));                              // @0x5C0E15 -> Scale9Frame::setScaleEdgeSize 0x46FBCE
		frame->setSlicePrefix(std::string("rbxasset://textures/") + sliceBase + ".png");  // op+ @0x5C0E31 (lit 0xB72991), append(".png" lit 0xB4DEA5) -- callee 0x46FC18 @0x5C0E69
		frame->setBackgroundTransparency(1.0f);                                          // @0x5C0EA2 -> 0x3A854E
		frame->setBorderSizePixel(0);                                                    // @0x5C0EB2 -> 0x3A828E
		frame->setSize(UDim2(1.0f, 0, 1.0f, 0));                                         // @0x5C0EEA -> 0x3A811A
		frame->setPosition(UDim2(0.0f, 0, 0.0f, -30));                                   // @0x5C0F22 -> 0x3A81D4; offset word 0xFFE2 = -30
		return frame;
	}

	// mac: 0x5C1079 (1078B) -- ecx = &OUT (passed straight through to fac1),
	// edx = &base, [esp] = &tailPos, [esp+4] = &tailSize.  fac1(base +
	// "_bkg") + the tail ImageLabel parented onto the backing frame.
	// STUB: PLAYER 0x0096e5b0
	// STUB: STUDIO 0x00a1d230
	static shared_ptr<Scale9Frame> createChatBubbleTail(const std::string& base, const UDim2& tailPos, const UDim2& tailSize)
	{
		shared_ptr<Scale9Frame> out = createChatBubbleFrame(base + "_bkg");      // copy+append('_bkg' lit @0xB729B0, ref 0x5C109F) then fac1 call @0x5C10BE
		shared_ptr<ImageLabel> tail = Creatable<Instance>::create<ImageLabel>(); // @0x5C10DE -> create<ImageLabel> 0x967AFE
		tail->setName("ChatBubbleTail");                                          // vtbl+0x1C; lit @0xB729B5 (ref 0x5C10FB)
		tail->setImage(TextureId(std::string("rbxasset://textures/") + base + "_tail.png"));  // op+ @0x5C1157, append('_tail.png' lit @0xB729C4, ref 0x5C116E), ContentIdC2 @0x5C1192 -> 0x95F0BC, vcall vtbl+0xC4
		tail->setBackgroundTransparency(1.0f);
		tail->setBorderSizePixel(0);
		tail->setPosition(tailPos);                                               // *a3
		tail->setSize(tailSize);                                                  // *a4
		tail->setParent(out.get());                                               // setParentInternal(tail, *a1 = the backing frame, false) -- @0x5C1251 -> Instance::setParentInternal 0x542D16
		return out;
	}

	// mac: 0x5C14AF -- edx = a1 = &base, ecx = a2 = &OUT, xmm0 = a3 = frame
	// scale, [esp] = a4 = &tailPos (verified __usercall listing).  fac1 +
	// a transparent ChatBubbleTailFrame container (scale-sized) + the
	// tail ImageLabel inside it.
	// STUB: PLAYER 0x0096e7a0
	// STUB: STUDIO 0x00a1d420
	static shared_ptr<Scale9Frame> createChatBubbleTailFrame(const std::string& base, float frameScale, const UDim2& tailPos)
	{
		shared_ptr<Scale9Frame> out = createChatBubbleFrame(base + "_bkg");       // fac1 call @0x5C14FF
		shared_ptr<Frame> tailFrame = Creatable<Instance>::create<Frame>();
		tailFrame->setName("ChatBubbleTailFrame");                                // block 0x5C14AF..0x5C1B0C
		tailFrame->setBackgroundTransparency(1.0f);
		tailFrame->setSizeConstraint(GuiObject::RelativeXX);                      // 1 (fac3 decompile: setSizeConstraint(v8, 1))
		tailFrame->setPosition(UDim2(0.5f, 0, 1.0f, 0));                          // staging = {0.5f, 0, 1.0f, 0}
		tailFrame->setSize(UDim2(frameScale, 0, frameScale, 0));                  // xmm0 (white 0.5f @0xC1E4BC / bots 0.7f @0xC68368)
		tailFrame->setParent(out.get());                                          // setParentInternal(v8, *a2 = the backing frame, false)
		shared_ptr<ImageLabel> tail = Creatable<Instance>::create<ImageLabel>();
		tail->setName("ChatBubbleTail");
		tail->setImage(TextureId(std::string("rbxasset://textures/") + base + "_tail.png"));  // op+ @0x5C16C6 + ContentIdC2 @0x5C1707 + vcall vtbl+0xC4
		tail->setBackgroundTransparency(1.0f);
		tail->setBorderSizePixel(0);
		tail->setPosition(tailPos);                                               // *a4
		tail->setSize(UDim2(1.0f, 0, 1.0f, 0));                                   // setSize(v15, 1065353216)
		tail->setParent(tailFrame.get());                                         // setParentInternal(v15, v37 = the tail frame, false)
		return out;
	}

	// mac: 0x5C2678 (252B; mangle __ZN3RBX10ChatOutput16SanitizeChatLineERKSs).
	// Raw asm: cmp text.length, [0xFB87A8] (= 124, InitFunc_414); jbe ->
	// plain copy ctor of text; else basic_string(text, 0, LIMIT) + append
	// (ChatLine::ELIPSES, strlen(ChatLine::ELIPSES)) => first 124 chars +
	// "...".
	std::string ChatOutput::SanitizeChatLine(const std::string& text)
	{
		if (text.length() <= kChatLineLimit)
			return text;
		std::string line(text, 0, kChatLineLimit);
		line.append(ChatLine::ELIPSES, strlen(ChatLine::ELIPSES));
		return line;
	}

	// mac: 0x5C2FC6 (421B): removeExpired -- raw-asm verified.
	//   max = min(GameSettingsItem +120, 10) (singleton read 0x5C2FCF,
	//   cmovle clamp 0x5C2FDC); landed GameSettings::chatLinesMax is a
	//   name DELTA.  +236 (0xEC) deque arm 0x5C2FE8-0x5C301F: with a
	//   front element present (start.cur +244 != finish.cur +260),
	//   expiry = lineLifetime(+0x34, the 60.0f seat) + clockAtInsert
	//   (+0x10); clock(+0x12C) > expiry -> pop_front, result true.
	//   Map arm (begin = header._M_left +288, end = &header +280): per
	//   CharacterChats (value @node+0x14; start.cur @cc+8, finish.cur
	//   @cc+24): empty -> straight to the erase check; front px =
	//   *start.cur; when clock > front->clockAtInsert(+0x10) + front->
	//   countdown(+0x14) OR deque size > max -> pop_front (0x5C3091),
	//   result true.  After the pop the cursors are re-read
	//   (0x5C3099-C) and an emptied deque falls into the erase arm
	//   mac: 0x5C30A7: lock the weak BillboardGui (cc+44) with the no-throw
	//   weak->shared ctor, unparent via setParentInternal(px, NULL,
	//   false) (the landed setParent(NULL) inline body), release, then
	//   erase the map node (increment/rebalance/destroy/--count @+296
	//   -- the landed erase(it++) folds increment + erase).  Non-empty
	//   entries just advance (0x5C311F).  Returns result.
	bool ChatOutput::removeExpired()
	{
		int maxLines = GameSettings::singleton().chatLinesMax;
		if (maxLines > 10)
			maxLines = 10;

		bool result = false;

		if (!activeLines.empty())
		{
			// The +236 deque carries PlayerChatLine elements only (see
			// ChatLine.h); the binary reads the expiry seat straight
			// off the derived +0x34 float.
			PlayerChatLine* front = static_cast<PlayerChatLine*>(activeLines.front().get());
			if (clock > front->lineLifetime + front->clockAtInsert)
			{
				activeLines.pop_front();
				result = true;
			}
		}

		std::map<Instance const*, CharacterChats>::iterator it = chats.begin();
		while (it != chats.end())
		{
			CharacterChats& entry = it->second;
			if (!entry.lines.empty())
			{
				ChatLine* front = entry.lines.front().get();
				if (clock > front->clockAtInsert + front->countdown
					|| (int)entry.lines.size() > maxLines)
				{
					entry.lines.pop_front();
					result = true;
				}
			}
			if (entry.lines.empty())
			{
				shared_ptr<BillboardGui> billboard = entry.billboard.lock();
				if (billboard != NULL)
					billboard->setParent(NULL);
				chats.erase(it++);
			}
			else
				++it;
		}
		return result;
	}

	// mac: 0x5C1B0C..0x5C229A -- ecx = &OUT, edx = &outer, xmm0 = dotY,
	// [esp] = &inner (verified from the raw).  The notification
	// placeholder: a fullscreen DialogPlaceholder ImageLabel showing the
	// notify background texture + a 0.7-high DotDotDot strip.
	// STUB: PLAYER 0x0096eb20
	// STUB: STUDIO 0x00a1d7a0
	static shared_ptr<ImageLabel> createChatBubbleNotifyFrame(const std::string& outer, float dotY, const std::string& inner)
	{
		shared_ptr<ImageLabel> placeholder = Creatable<Instance>::create<ImageLabel>();
		placeholder->setName("DialogPlaceholder");
		placeholder->setImage(TextureId(std::string("rbxasset://textures/") + outer + ".png"));  // op+ @0x5C1BBD (var_10C then overwritten with the rbxasset literal) + ContentIdC2 @0x5C1C01 + vcall vtbl+0xC4
		placeholder->setBackgroundTransparency(1.0f);
		placeholder->setBorderSizePixel(0);
		placeholder->setPosition(UDim2(0.0f, 0, -1.25f, 0));                       // 0xBFA00000 = -1.25f
		placeholder->setSize(UDim2(1.0f, 0, 1.0f, 0));
		shared_ptr<ImageLabel> dot = Creatable<Instance>::create<ImageLabel>();
		dot->setName("DotDotDot");
		dot->setImage(TextureId(std::string("rbxasset://textures/") + inner + ".png"));  // op+ @0x5C1D9A ([esp+4] = rbxasset lit, [esp+8] = arg inner) + ContentIdC2 @0x5C1DDB + vcall
		dot->setBackgroundTransparency(1.0f);
		dot->setBorderSizePixel(0);
		dot->setPosition(UDim2(0.001f, 0, dotY, 0));                               // 0x3A83126F = 0.001f; dotY: white -0.05f @0xC84670 / bots -0.12f @0xC8593C
		dot->setSize(UDim2(1.0f, 0, 0.7f, 0));                                     // 0x3F333333 = 0.7f
		dot->setParent(placeholder.get());                                         // setParentInternal(dot, placeholder, false)
		return placeholder;
	}

	// ===== Bubble-template factories (file statics) =====
	// GCC __usercall signatures decoded from dbq24; C2 0x5BF2DC is the
	// only caller set (xref run C:\tmp\lits.txt -- no other callers of
	// mac: 0x5C0D8D/0x5C1079/0x5C14AF/0x5C1B0C).  Names are behavior-pinned
	// placeholders (DELTAs: linkonce-local GCC functions emit no symbol
	// names).

	// STUB: PLAYER 0x00970970
	// STUB: STUDIO 0x00a1f550
	PlayerChatLine::PlayerChatLine(ChatType type, shared_ptr<Network::Player> player,
		const std::string& text, float clock, bool flag)
		: ChatLine(type, text, clock, ChatLine::BUBBLECOLOR_0, flag)
	{
		lineLifetime = 60.0f;

		if (player == NULL)
		{
			playerName = ROBLOXNAME;
			color3 = *Color::getColorByIndex(0);
			return;
		}

		playerName = player->getName();
		origin = player->character;

		Teams* teams = ServiceProvider::find<Teams>(player.get());
		if (teams != NULL && teams->isTeamGame())
		{
			if (player->neutral == false)
			{
				Team* team = teams->getTeamFromPlayer(player.get());
				color3 = (team != NULL) ? team->getTeamColor().color3()
					: G3D::Color3::black();
			}
			else
				color3 = G3D::Color3::white();
		}
		else
			color3 = *Color::colorFromIndex8(Hash::hash(playerName) & 7);
	}

	// ===== ChatOutput(void) 0x5BF2DC (6833B) =====
	// Members the binary touches beyond the compiler-managed defaults:
	// players dword 28 (+112) = 0 (C2 line 245) and clock dword 75
	// (+300) = 0 (line 302).  The five template maps (empty 6-dword
	// headers), the deque (_M_initialize_map @+236, line 294) and the
	// chats map are default-constructed (lines 246-305).  The 16 smith
	// template arms build one bubble per BubbleColor; statement order
	// A0,B0,C0 / A1,B1,C1 / A3,B3,C3 / A2,B2,C2 / E0,E1,E2,E3.
	// STUB: PLAYER 0x00971040
	// STUB: STUDIO 0x00a1fc40
	ChatOutput::ChatOutput()
	{
		players = NULL;
		clock = 0.0f;

		// A-arms -> bubbleBackings; the full "_bkg" slice names pass direct.
		bubbleBackings[ChatLine::BUBBLECOLOR_0] = createChatBubbleFrame("chatBubble_white_bkg");    // A0 string ctor 0x5BF5F5, fac1 call 0x5BF60A
		bubbleBackings[ChatLine::BUBBLECOLOR_1] = createChatBubbleFrame("chatBubble_botBlue_bkg");  // A1
		bubbleBackings[ChatLine::BUBBLECOLOR_3] = createChatBubbleFrame("chatBubble_botRed_bkg");   // A3
		bubbleBackings[ChatLine::BUBBLECOLOR_2] = createChatBubbleFrame("chatBubble_botGreen_bkg"); // A2

		// B-arms -> bubbleTails via fac2: white tail pos (0.5,-15)/size (0,30,0,30); bots pos (0.5,-21)/size (0,42,0,42).
		bubbleTails[ChatLine::BUBBLECOLOR_0] = createChatBubbleTail("chatBubble_white", UDim2(0.5f, -15, 1.0f, -10), UDim2(0, 30, 0, 30));    // B0, fac2 call 0x5BF70B
		bubbleTails[ChatLine::BUBBLECOLOR_1] = createChatBubbleTail("chatBubble_botBlue", UDim2(0.5f, -21, 1.0f, -10), UDim2(0, 42, 0, 42));  // B1
		bubbleTails[ChatLine::BUBBLECOLOR_3] = createChatBubbleTail("chatBubble_botRed", UDim2(0.5f, -21, 1.0f, -10), UDim2(0, 42, 0, 42));   // B3
		bubbleTails[ChatLine::BUBBLECOLOR_2] = createChatBubbleTail("chatBubble_botGreen", UDim2(0.5f, -21, 1.0f, -10), UDim2(0, 42, 0, 42)); // B2

		// C-arms -> bubbleTailFrames via fac3: white frameScale 0.5 + tailPos (-0.5,0,-0.35,0); bots 0.7 + (-0.5,0,-0.23,0).
		bubbleTailFrames[ChatLine::BUBBLECOLOR_0] = createChatBubbleTailFrame("chatBubble_white", 0.5f, UDim2(-0.5f, 0, -0.35f, 0));   // C0, fac3 call 0x5BF7DE
		bubbleTailFrames[ChatLine::BUBBLECOLOR_1] = createChatBubbleTailFrame("chatBubble_botBlue", 0.7f, UDim2(-0.5f, 0, -0.23f, 0));  // C1
		bubbleTailFrames[ChatLine::BUBBLECOLOR_3] = createChatBubbleTailFrame("chatBubble_botRed", 0.7f, UDim2(-0.5f, 0, -0.23f, 0));   // C3
		bubbleTailFrames[ChatLine::BUBBLECOLOR_2] = createChatBubbleTailFrame("chatBubble_botGreen", 0.7f, UDim2(-0.5f, 0, -0.23f, 0)); // C2

		// E-arms -> bubbleNotifications via fac4: white dotY -0.05 and the Gray dot texture; all bots dotY -0.12 and the bot dot texture.
		bubbleNotifications[ChatLine::BUBBLECOLOR_0] = createChatBubbleNotifyFrame("chatBubble_white_notify_bkg", -0.05f, "chatBubble_bot_notifyGray_dotDotDot");  // E0, fac4 call 0x5BFFE5
		bubbleNotifications[ChatLine::BUBBLECOLOR_1] = createChatBubbleNotifyFrame("chatBubble_botBlue_notify_bkg", -0.12f, "chatBubble_bot_notify_dotDotDot");    // E1
		bubbleNotifications[ChatLine::BUBBLECOLOR_2] = createChatBubbleNotifyFrame("chatBubble_botGreen_notify_bkg", -0.12f, "chatBubble_bot_notify_dotDotDot");   // E2
		bubbleNotifications[ChatLine::BUBBLECOLOR_3] = createChatBubbleNotifyFrame("chatBubble_botRed_notify_bkg", -0.12f, "chatBubble_bot_notify_dotDotDot");     // E3
	}

	// mac: 0x5C316C (710B): createBillboardGuiHelper -- decompile + raw verified.
	//   mac: 0x5C3188  cc = chats[inst] (the +276 map impl; key = the inst px);
	//   mac: 0x5C31A0  shared_ptr<BillboardGui>(cc+44 weak, sp_nothrow) = the
	//             landed lock(); 0x5C31C5 `test old.px; jz` skips the whole
	//             body when a live billboard survives (no re-creation, no
	//             null gate on the entry read side).
	//   mac: 0x5C31CE  cgs = ServiceProvider::create<CoreGuiService>(inst);
	//             NULL -> return without touching anything.
	//   mac: 0x5C31E2  the !a3 arm runs first and falls through: for !a3 the
	//             inst descriptor is probed against PartInstance
	//             (`*(a2[12 = shadow+12])` = landed getDescriptor()); a
	//             PartInstance adornees the inst DIRECTLY (the GCM chat
	//             origin on a workspace piece).
	//   mac: 0x5C3292  the model arm (a3==true, or !a3 with the Part probe
	//             failed) probes ModelInstance, findConstFirstChildByName
	//             ("Head" -- the string literal built at 0x5C32D8, its
	//             COW release at 0x5C32F0) and a second probe against
	//             PartInstance promotes the head; only a confirmed Part
	//             head adornees.
	//   Both success arms: Creatable<Instance>::create<BillboardGui>() ->
	//   setAdornee(px) -> setRobloxLocked(true) -> setParentInternal(cgs,
	//   NULL-arg==false) -- the landed setParent(cgs) -- -> the weak store
	//   (cc+44 = px / weak_count(+48) = landed entry.billboard = bg) ->
	//   the temp release folded into the landed shared dtor.
	// STUB: PLAYER 0x00972110
	// STUB: STUDIO 0x00a20d10
	void ChatOutput::createBillboardGuiHelper(class RBX::Instance* inst, bool a3)
	{
		shared_ptr<BillboardGui> live = chats[inst].billboard.lock();
		if (live != NULL)
			return;

		CoreGuiService* cgs = ServiceProvider::create<CoreGuiService>(inst);
		if (cgs == NULL)
			return;

		if (!a3
			&& inst != NULL
			&& inst->getDescriptor().isA(PartInstance::classDescriptor()))
		{
			shared_ptr<BillboardGui> bg = Creatable<Instance>::create<BillboardGui>();
			bg->setAdornee(inst);
			bg->setRobloxLocked(true);
			bg->setParent(cgs);
			chats[inst].billboard = bg;
			return;
		}

		if (inst != NULL
			&& inst->getDescriptor().isA(ModelInstance::classDescriptor()))
		{
			const Instance* head = inst->findConstFirstChildByName("Head");
			Instance* headAdornee = NULL;
			if (head != NULL
				&& head->getDescriptor().isA(PartInstance::classDescriptor()))
				headAdornee = const_cast<Instance*>(head);
			if (headAdornee != NULL)
			{
				shared_ptr<BillboardGui> bg = Creatable<Instance>::create<BillboardGui>();
				bg->setAdornee(headAdornee);
				bg->setRobloxLocked(true);
				bg->setParent(cgs);
				chats[inst].billboard = bg;
			}
		}
	}

	// mac: 0x5C2DB8 (269B): onGameChatMessage -- raw-asm verified.
	//   Profanity gate 0x5C2DC6-E8: FFlag::LocalTextFiltering
	//   (off_EB8418 -> 0xF88527) && ProfanityFilter::ContainsProfanity
	//   (0x5D2104) -> drop before anything else.  own byte 0x5C2DEF-E08:
	//   players(+112) -> *(players+188) -> *( that + 92 ) = landed
	//   localPlayer->character.px; own = (origin.px == that), null
	//   chain leaves own = 1.  BubbleColor map 0x5C2E08-0x5C2E2D: ChatColor
	//   0/1/2 -> 1/2/3, anything else -> 0.  SanitizeChatLine (0x5C2E35)
	//   -> new GameChatLine (0x24 @0x5C2E3D; ctor 0x5BF1FA via a stack
	//   copy of origin bumped at 0x5C2E5D, sanitized text, clock float
	//   +0x12C @0x5C2E69, own byte, mapped BubbleColor) -> sp
	//   <ChatLine> wrap (0x5C2EA6) + copy release -> getOrigin
	//   (0x5C2EC0) -> map operator[] key = origin px (0x5C2EDB, +276
	//   impl) -> push_back into the CC deque (0x5C2EE7) ->
	//   createBillboardGuiHelper(origin.px, false) (0x5C2EFB).  NO +236
	//   pop loop and NO +236 push in this body (the +236 deque carries
	//   PlayerChatLine elements only).
	// STUB: PLAYER 0x00972310
	// STUB: STUDIO 0x00a20f10
	void ChatOutput::onGameChatMessage(shared_ptr<Instance> origin, const std::string& text, ChatService::ChatColor color)
	{
		if (FFlag::LocalTextFiltering
			&& ProfanityFilter::ContainsProfanity(text))
			return;

		bool own = true;
		Network::Player* local = players->localPlayer.get();
		if (local != NULL)
			own = (origin.get() == local->character.get());

		GameChatLine::BubbleColor mapped = ChatLine::BUBBLECOLOR_0;
		switch (color)
		{
			case ChatService::Blue: mapped = ChatLine::BUBBLECOLOR_1; break;
			case ChatService::Green: mapped = ChatLine::BUBBLECOLOR_2; break;
			case ChatService::Red: mapped = ChatLine::BUBBLECOLOR_3; break;
			default: break;
		}

		GameChatLine* line = new GameChatLine(origin, SanitizeChatLine(text),
			clock, own, mapped);
		shared_ptr<ChatLine> lineSp(line);
		chats[line->getOrigin().get()].lines.push_back(lineSp);

		createBillboardGuiHelper(origin.get(), false);
	}

	// mac: 0x5C2A96: onPlayerChatMessage -- raw-asm verified.
	//   FilterEmoteChat gate 0x5C2AA8: the "/e " walk (3 chars, literal
	//   0xB72841, 0x5C2ABE-0x5C2AE5) and the "/emote " walk (7 chars,
	//   literal 0xB72845, 0x5C2AEF-0x5C2B13) both carry the per-pass
	//   (len-1)==i overrun guard, i.e. exact starts-with walks; a full
	//   match drops the line (exit 0x5C2CF5 before any construction).
	//   +236 pop loop 0x5C2B24-67: while size > GameSettings +104
	//   (0x68, unclamped; landed chatLinesShown) -> pop_front.
	//   own byte 0x5C2B69-B1: players(+112) -> *(players+188) (landed
	//   localPlayer px; no NULL gate on the deref chain in the binary),
	//   own = (message.player.px == that), null intermediates leave
	//   own = 1.  ChatType remap = the table @0xC5EED0 (delivered
	//   identity).  SanitizeChatLine(message.message, 0x5C2678) ->
	//   new PlayerChatLine (0x38 @0x5C2BB9; ctor args = mapped type,
	//   a stack copy of message.player bumped at 0x5C2BCE, sanitized
	//   text, clock float +0x12C, own byte @0x5C2BF1) -> sp wrap
	//   (0x5C2C0D) + temp release -> push_back into the +236 deque
	//   (0x5C2C54) -> getOrigin (0x5C2C6E) -> map<Instance const*,
	//   CharacterChats>::operator[](origin px) on the +276 impl
	//   (0x5C2C7F) -> push_back into the CC deque (0x5C2CA3) -> helper
	//   call 0x5C2CD3 = createBillboardGuiHelper(*(player+92) =
	//   character px, true) when message.player.px != 0.
	// STUB: PLAYER 0x00972500
	// STUB: STUDIO 0x00a21100
	void ChatOutput::onPlayerChatMessage(const Network::ChatMessage& message)
	{
		if (FFlag::FilterEmoteChat
			&& (message.message.compare(0, 3, "/e ") == 0
				|| message.message.compare(0, 7, "/emote ") == 0))
			return;

		int shownCap = GameSettings::singleton().chatLinesShown;
		while (activeLines.size() > shownCap)
			activeLines.pop_front();

		bool own = true;
		Network::Player* local = players->localPlayer.get();
		if (local != NULL)
			own = (message.player.get() == local);

		PlayerChatLine* line = new PlayerChatLine(
			(ChatLine::ChatType)message.chatType, message.player,
			SanitizeChatLine(message.message), clock, own);
		shared_ptr<ChatLine> lineSp(line);
		activeLines.push_back(lineSp);
		chats[line->getOrigin().get()].lines.push_back(lineSp);

		if (message.player != NULL)
			createBillboardGuiHelper(message.player->character.get(), true);
	}

	// mac: 0x5C426C (1096B .. the 0x5C4418 retn): the classic-chat renderer.
	//   mac: 0x5C427B-84: FFlag::NativeChatRendering (the off_EB5C98 slot
	//      deref, byte test @0xFA9A58) -> OFF exits before anything.
	//   mac: 0x5C428A-9F: the vtbl+0x20 get2DExtents (sret float4) --
	//      Canvas{f2 - f0, f3 - f1} (0x5C42A2-BB).
	//   mac: 0x5C42C0-D1: getMyRect(Canvas) (sret var_30; the Canvas rides
	//      the 8-byte var_38/34 pair).
	//   a3 arm 0x5C42ED-94: chats[(Instance const*)NULL] op[] #1 (the
	//      NULL key staged at var_3C = 0) -> the CC `.lines` size() fold
	//      (the deque head/tail subtraction = the size() source
	//      expression; the imul 0Ch + 0xFFFFFD00h add runs on the 32-bit
	//      eax, so the landed spell keeps the int wrap), then the region
	//      low = {f0, f1 - (float)(12*size - 768)} (the qword_C1C908
	//      2^52 magic = the int->float pass) -- op[] #2 with the same
	//      NULL key (0x5C437B, var_4C = 0) = the painter's deque&;
	//      translucentBackdrop() 0x5C4382; the staging 0x5C4387-93.
	//   non-a3 arm 0x5C4396-45: the +236 deque fold (the +0xF4/FC/100/
	//      104/108/110 slot walk = the same size() shape -- the decomp
	//      elides it, the raw carries it), the same shift formula on
	//      &activeLines, color 0x5C43F5, the staging 0x5C43FA-45; the
	//      shared call 0x5C440C.
	// DELTA: the staged region Rect's high pair carries ABI junk in
	// the binary (only var_48/var_44 exist); every consumer reads the
	// low pair only, so the landed whole-Rect copy is order-invariant.
	// STUB: PLAYER 0x009727e0
	// STUB: STUDIO 0x00a213e0
	void ChatOutput::render2d_classicStyle(class RBX::Adorn* adorn, bool v)
	{
		if (!FFlag::NativeChatRendering)
			return;

		Rect extents = adorn->get2DExtents();       // vtbl +0x20 (sret)
		Canvas canvas;
		canvas.x = extents.high.x - extents.low.x;  // f2 - f0 (0x5C42A2-B6)
		canvas.y = extents.high.y - extents.low.y;  // f3 - f1 (0x5C42A7-BB)
		Rect region = getMyRect(canvas);            // sret var_30

		if (v)
		{
			// the double chats[NULL] op[] (0x5C430E = the size input,
			// mac: 0x5C437B = the painter input -- the binary looks the key
			// up twice; the landing spells both).
			region.low.y -= (float)(12 * (int)chats[(Instance const*)NULL].lines.size() - 768);
			renderClassicLineList(adorn, region, chats[(Instance const*)NULL].lines,
				translucentBackdrop());
		}
		else
		{
			region.low.y -= (float)(12 * (int)activeLines.size() - 768);
			renderClassicLineList(adorn, region, activeLines,
				translucentBackdrop());
		}
	}

	// mac: 0x5C4EC0 (3633B, rarc18.txt full raw+decompile): renderBubbles --
	// the far-slot bubble render (mf6 bind route).  ORDER (all pools
	// re-landed as literals; base 0x5C4ED1):
	// (a) the two weak locks -- part (5C4F06) then Instance const
	// origin (5C4F23), either NULL = the release-tail return;
	// (b) ServiceProvider::find<Workspace> off the PART (5C4F45, the
	// receiver = the part px), NULL guard;
	// (c) the ViewportBillboarder temp: V3 offset/V3 fade + G3D::
	// Vector2::zero() (5C4F69 calls __ZN3G3D7Vector24zeroEv -- NOT the
	// inline V2(0,0) the imposter arm spells) + UDim2(22.0f,25,3.0f,0)
	// (5C4F7B..5C4F91) + pivot = (const Vector2*) NULL;
	// (d) camera = the workspace-vptr+0xC8 vcall (5C4FA3..5C4FB3);
	// (e) partSize = the landed 28B getPartSizeXml copy (5C4FBC, the
	// fresh Vector3 local), frame = the Matrix3 copy ctor + the
	// translation words (5C4FF0..5C505C, the one-copy staging --
	// mirror the imposter arm); AdornBillboarder impostor(adorn,
	// camera, partSize, frame, billboarder);
	// (f) NO isVisibleAndValid gate here (the imposter arm has one;
	// renderBubbles does not): chats[origin px] op[] (5C506C, the map
	// at a1+276 => the RBX::CharacterChats&);
	// (g) the setOBM slot call = a2's vptr+88 (decompile 503, the 3rd
	// slot of the +0x50.. pure band per the ZTV map) staged with the
	// default G3D::CoordinateFrame ctor (5C50AC = identity);
	// (h) rect = impostor.getViewport() (5C50DD), then the center-
	// preserving width clamp to 336.0f (5C50E2..5C516B): clipped =
	// min(336, w); excess = w - clipped; x0' = excess*0.5f + x0;
	// x1' = clipped + x0'; the x pair rewrites min/max-sorted and the
	// y pair = min/max of (y1-y0)+y0 vs y0 (identity y0<=y1, kept);
	// (i) fontSize = (x1'-x0')/21.0f (5C517C; the 336 cap bounds it at
	// 16.0f); scale = {(double)span - (double)fs (5C5190..5C51A7, the
	// -0.5d*2 addsd fold), 5.0d*(1.5d*fs)+0.01d (5C51B3..5C51D3)} --
	// the cvtss2sd/subsd/mulsd/addsd/cvtsd2ss double-math chain kept;
	// (j) count = the GCC deque-fold == cc.lines.size() (5C51DB..
	// 5C5209); rows = the 16B/row arena (stack alloca 5C521E -- the
	// landed std::vector: identical {x0,y0,x1,y1} float layout,
	// DELTA in the allocation spell), slot = 0; count == 0 = the
	// render-entry jump with the empty rows;
	// (k) the size pass (5C529B..5C54BF), newest-first (dec/count-1
	// .. 0; the skip path decs+cmps -1, the pass path tests the loop
	// bottom): gate = a4 (arg bool) OR chatType@+0xC >=
	// CHATLINE_TYPE_3; per pass line: service =
	// ServiceProvider::create<TextService>(this) (5C52D7, the
	// Creatable chain), measure = getTypesetter(font 0)->measure(
	// text, fs, scale, 0) as ONE full expression (the registry
	// refcount releases right after the call, before the row math);
	// the row math (5C53B1..5C54AA): sh = min(eY/60, min(eX/60, 1))
	// *13+7; xa = (x0+x1)*0.5f - eX*0.5f - sh; xb = xa + (eX+sh+sh);
	// bottom = (float)(cursor - (double)eY) - sh (the subsd double
	// math, then cvtsd2ss, then subss); top = (sh+sh + eY) + bottom;
	// the stores min/max-sorted (x0=min(xb,xa), y0=min(top,bottom),
	// x1=max, y1=max); rowHeight = stored y1-y0; cursor -=
	// (double)rowHeight + 2.0d; ++slot;
	// (l) the render pass (5C54C2..0x5C5CF1 chunk), oldest-first (the
	// gate/text advances each copy a fresh iter -- the extern +=
	// cannot CSE): slot = rowCount; per good line: the bubbleColor
	//@+8 read (fresh advance 2: the map keys), prevSlot = slot-1,
	// the row bind = rows[slot-1] (v40 = 16*(slot-1) byte offset),
	// color = black staged; slot == 1 = the NEWEST row (rows[0]):
	// 60.0f <= width && 60.0f <= height => bubbleTails[...].get()
	// ->legacyRender2d(&impostor, row); else the ChatBubbleTailFrame
	// child (findConstFirstChildByName, the ClassDescriptor::isA
	// GuiObject gate) gets setSizeConstraint((height <= width)?
	// RelativeXX : RelativeYY) (the (bool)+1 fold = 1|2) and
	// bubbleTailFrames[...] re-keyed -> legacyRender2d; then the
	// newest-bubble text color: bubbleColor != 0 ? white : black;
	// the non-last rows: bubbleBackings[...].get()->legacyRender2d
	// row + the color switch -- 0 = 0.745098f gray x3 (@C55D44, one
	// pool float), 1 = #2D8EF5 (@C85958-60), 2 = 0/0.745098/0 green
	// (@C8594C g), 3 = #FF584F (@C1C918/C85950-54), default keeps
	// black; common finisher: anchor = ((x1+x0)*0.5f) + 0.0f,
	// ((y1+y0)*0.5f) + (-30.0f) (0.5f@C1C91C, -30.0f@C85964 -- the
	// +0.0f literal kept: -0 + 0 flips to +0 in RN mode); the text =
	// fresh advance 3; fill = (color.r, g, b, 1.0); outline =
	// (black, 0.0) (v90[6..9]); clipRect = -1.0f x4 (v90[2..5]);
	// drawFont2D(text, anchor, fs, fill, outline, font0, xaCenter,
	// yaCenter, scale, clipRect) (5CD756/75E witnesses the enum
	// pack); restore slot = prevSlot on hits (unchanged on skips);
	// ++walkIdx; scope death = the AB's Adorn D2 (5C4E48 shape) + the
	// two lock releases.
	// STUB: PLAYER 0x00972d30
	// STUB: STUDIO 0x00a21930
	void ChatOutput::renderBubbles(class RBX::Adorn* adorn,
		boost::weak_ptr<Instance const> origin,
		boost::weak_ptr<PartInstance> part, bool a4,
		G3D::Vector3 offset, G3D::Vector3 fade)
	{
		boost::shared_ptr<PartInstance> partLocked = part.lock();
		if (partLocked.get() != NULL)
		{
			boost::shared_ptr<Instance const> originLocked = origin.lock();
			if (originLocked.get() != NULL)
			{
				Workspace* workspace =
					ServiceProvider::find<Workspace>(partLocked.get());
				if (workspace != NULL)
				{
					ViewportBillboarder billboarder(offset, fade,
						G3D::Vector2::zero(),
						UDim2(22.0f, 25, 3.0f, 0), NULL);

					Camera* camera = workspace->getCamera();
					G3D::Vector3 partSize = partLocked->getPartSizeXml();
					G3D::CoordinateFrame frame =
						partLocked->getCoordinateFrame();

					AdornBillboarder impostor(adorn, *camera, partSize,
						frame, billboarder);

					CharacterChats& cc = chats[originLocked.get()];

					// the +0x58 setOBM slot with the identity frame.
					adorn->setObjectToWorldMatrix(G3D::CoordinateFrame());

					G3D::Rect2D rect = impostor.getViewport();

					// the center-preserving width clamp to 336.0f; the
					// y pair = its own min/max re-sort (identity for a
					// sorted vp rect, kept for faith).
					float rectX0 = rect.x0();
					float rectY0 = rect.y0();
					float rectX1 = rect.x1();
					float rectY1 = rect.y1();
					float spanW = rectX1 - rectX0;
					// fminf(fmaxf) spelled via ternary per the VS2012
					// toolchain rule (the ChatLine ctor precedent).
					float clipped = 336.0f < spanW ? 336.0f : spanW;
					float excess = spanW - clipped;
					float xA = excess * 0.5f + rectX0;
					float xB = clipped + xA;
					float yWide = (rectY1 - rectY0) + rectY0;

					// the binary rewrites the four rect floats in place;
					// the landed rebuilds via the xyxy factory (the
					// internal min/max idempotent, float-identical).
					rect = G3D::Rect2D::xyxy(xB < xA ? xB : xA,
						yWide < rectY0 ? yWide : rectY0,
						xA > xB ? xA : xB,
						rectY0 > yWide ? rectY0 : yWide);

					// fontSize = the clamped span / 21 (<= 16.0f via the
					// cap); scale carries the double-math chain; rows =
					// the identical-float arena.
					float clampedSpan = rect.x1() - rect.x0();
					float fontSize = clampedSpan / 21.0f;
					G3D::Vector2 scale(
						(float)((double)clampedSpan - (double)fontSize),
						(float)(5.0 * (1.5 * fontSize) + 0.01));

					int count = (int)cc.lines.size();
					std::vector<G3D::Rect2D> rows(count);
					int slot = 0;

					if (count != 0)
					{
						// the cursor double = -(1.5d*fs) -- the -0.0d
						// sign flip of the mulsd.
						double cursor = -(1.5 * fontSize);

						// the size pass, newest-first.
						for (int walk = count - 1; walk >= 0; --walk)
						{
							if (a4 || (*(cc.lines.begin() + walk))->chatType
								>= ChatLine::CHATLINE_TYPE_3)
							{
								TextService* service =
									ServiceProvider::create<TextService>(this);

								if (service != NULL)
								{
									// the registry refcount release rides
									// at this full expression's end.
									G3D::Vector2 extents =
										service->getTypesetter(
											(TextService::Font)0)->measure(
												(*(cc.lines.begin()
													+ walk))->text,
												fontSize, scale, 0);

									float xOver60 = extents.x / 60.0f;
									float yOver60 = extents.y / 60.0f;
									float rCap = xOver60 < 1.0f
										? xOver60 : 1.0f;
									float yCap = yOver60 < rCap
										? yOver60 : rCap;
									float sh = yCap * 13.0f + 7.0f;
									float xa = (rect.x0() + rect.x1()) * 0.5f
										- extents.x * 0.5f - sh;
									float xb = xa
										+ (extents.x + (sh + sh));
									float rowBottom = (float)(cursor
										- (double)extents.y) - sh;
									float rowTop = ((sh + sh) + extents.y)
										+ rowBottom;

									rows[slot] = G3D::Rect2D::xyxy(
										xb < xa ? xb : xa,
										rowTop < rowBottom
											? rowTop : rowBottom,
										xa > xb ? xa : xb,
										rowBottom > rowTop
											? rowBottom : rowTop);

									cursor -= (double)rows[slot].height()
										+ 2.0;
									++slot;
								}
							}
						}
					}

					// the render pass, oldest-first.
					int walkIdx = 0;
					while (walkIdx < count)
					{
						if (a4 || (*(cc.lines.begin() + walkIdx))->chatType
							>= ChatLine::CHATLINE_TYPE_3)
						{
							// advance 2: the arm/map reads off the probe.
							shared_ptr<ChatLine>& probe =
								*(cc.lines.begin() + walkIdx);

							int prevSlot = slot - 1;
							G3D::Rect2D& row = rows[slot - 1];
							G3D::Color3 color = G3D::Color3::black();

							if (slot == 1)
							{
								// the newest bubble (rows[0]): the tail arm.
								if (60.0f <= row.width()
									&& 60.0f <= row.height())
								{
									bubbleTails[probe->bubbleColor].get()
										->legacyRender2d(&impostor, row);
								}
								else
								{
									// small rows size the ChatBubbleTailFrame
									// child: (height<=width)+1 folds into
									// RelativeXX (1) / RelativeYY (2).
									Instance* child = bubbleTailFrames
										[probe->bubbleColor].get()
										->findFirstChildByName(
											"ChatBubbleTailFrame");
									if (child != NULL
										&& child->getDescriptor().isA(
											GuiObject::classDescriptor()))
									{
										GuiObject* gui = (GuiObject*)child;
										gui->setSizeConstraint(
											row.height() <= row.width()
												? GuiObject::RelativeXX
												: GuiObject::RelativeYY);

										// the second fresh op[] insert.
										bubbleTailFrames[probe->bubbleColor]
											.get()->legacyRender2d(&impostor,
												row);
									}
								}

								if (probe->bubbleColor != 0)
									color = G3D::Color3::white();
								else
									color = G3D::Color3::black();
							}
							else
							{
								bubbleBackings[probe->bubbleColor].get()
									->legacyRender2d(&impostor, row);
								switch (probe->bubbleColor)
								{
								case ChatLine::BUBBLECOLOR_0:
									color.r = 0.745098f;
									color.g = 0.745098f;
									color.b = 0.745098f;
									break;
								case ChatLine::BUBBLECOLOR_1:
									color.r = 0.176471f;
									color.g = 0.556863f;
									color.b = 0.960784f;
									break;
								case ChatLine::BUBBLECOLOR_2:
									color.r = 0.0f;
									color.g = 0.745098f;
									color.b = 0.0f;
									break;
								case ChatLine::BUBBLECOLOR_3:
									color.r = 1.0f;
									color.g = 0.345098f;
									color.b = 0.309804f;
									break;
								default:
									break;
								}
							}

							slot = prevSlot;

							// advance 3: the drawFont2D string ref.
							shared_ptr<ChatLine>& textLine =
								*(cc.lines.begin() + walkIdx);
							G3D::Vector2 anchor(
								(row.x1() + row.x0()) * 0.5f + 0.0f,
								(row.y1() + row.y0()) * 0.5f - 30.0f);
							G3D::Color4 fill(color.r, color.g, color.b,
								1.0f);
							G3D::Color4 outline(G3D::Color3::black(), 0.0f);
							G3D::Rect2D clipRect =
								G3D::Rect2D::xyxy(-1.0f, -1.0f, -1.0f,
									-1.0f);
							impostor.drawFont2D(textLine->text, anchor,
								fontSize, fill, outline, Text::font0,
								Text::xaCenter, Text::yaCenter, scale,
								clipRect);
						}
						++walkIdx;
					}
				}
			}
		}
	}

	// mac: 0x5C3498 (3540B, PIC anchor 0x5C34A9): render2d_bubbleStyle -- the
	// per-node chats walk: (1) the node billboard's render function is
	// cleared only when the weak lock hits; (2) the flagDecay pair
	// zeroes; (3) the front line's chatType arm runs (model-Head /
	// part-Head / the :771/:775 flashed asserts); (4) LABEL_49 = the
	// camera depth gate routing the imposters/bubbles mf3/mf6 binds.
	// Stage locals are FUNCTION-scope (NOT per-node resets): the
	// workspace cache (v52) persists the find across nodes, and the
	// case-3/skip arms inherit the previous arm's v51/v53 -- harmless
	// because v54 stays NULL on those paths.  DELTAs: the deque front
	// cursor arithmetic (operator+=(0) walks) collapsed to the front()
	// reads; the a4=false front re-walk collapsed to the
	// `v || front->chatType >= 3` head gate (raw 0x5C3A2C); the bubbles
	// arm's second Workspace::getCamera vcall (0x5C3C1D) landed as the
	// one camera local reuse; the GCC bind-fn-pointer staging locals
	// (var_1C4/var_1C8) = the natural boost::bind temps here.
	// STUB: PLAYER 0x009736a0
	// STUB: STUDIO 0x00a222a0
	void ChatOutput::render2d_bubbleStyle(class RBX::Adorn* adorn, bool v)
	{
		Workspace* workspace = NULL;      // v52 -- FUNCTION-scope cache
		float yShift = 0.0f;              // v51 -- FUNCTION-scope stage
		float fade = 0.0f;                // v53 -- FUNCTION-scope stage

		for (std::map<Instance const*, CharacterChats>::iterator it = chats.begin();
			it != chats.end(); ++it)
		{
			CharacterChats& cc = it->second;

			// 0x5C35xx: the re-prime (the empty-function visit).
			{
				boost::shared_ptr<BillboardGui> billboard = cc.billboard.lock();
				if (billboard.get() != NULL)
					billboard->setRenderFunction(boost::function<void (BillboardGui*, Adorn*)>());
			}

			// +40/+41 zero each node.
			cc.flagDecay = false;
			cc.flagDecay3 = false;

			// the front-line empty gate.
			if (cc.lines.empty())
				continue;

			ChatLine* front = cc.lines.front().get();   // DELTA: cursor collapse
			PartInstance* part = NULL;                  // v54 per-node null
			boost::shared_ptr<Instance> origin = front->getOrigin();
			if (origin == NULL)
				continue;

			switch (front->chatType)
			{
			case ChatLine::CHATLINE_TYPE_0:
			case ChatLine::CHATLINE_TYPE_1:
			case ChatLine::CHATLINE_TYPE_2:
			case ChatLine::CHATLINE_TYPE_GAME_MODEL:
				// the model-Head arm (0x5C36xx-0x5C39xx): the origin must
				// be a ModelInstance (else the :771 flashed assert); the
				// Head part child pins v54; the workspace finds once.
				if (origin != NULL
					&& origin->getDescriptor().isA(ModelInstance::classDescriptor()))
				{
					const Instance* head = origin->findConstFirstChildByName("Head");
					part = NULL;
					if (head != NULL
						&& head->getDescriptor().isA(PartInstance::classDescriptor()))
						part = static_cast<PartInstance*>(const_cast<Instance*>(head));
					yShift = 0.0f;
					if (part != NULL && workspace == NULL)
						workspace = ServiceProvider::find<Workspace>(part);
				}
				else
				{
					yShift = 0.0f;
					if (FLog::Asserts != 0)
					{
						if (RBX::_internal::_debugHook == 0
							|| RBX::_internal::_debugHook("0",
								"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatOutput.cpp",
								771) == 0)
						{
							ReleaseAssert(FLog::Asserts,
								"0 file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatOutput.cpp line: 771");
						}
					}
				}
				fade = 2.0f;
				break;

			case ChatLine::CHATLINE_TYPE_3:
				// legacy line kind: only the :775 flashed assert.
				if (FLog::Asserts != 0)
				{
					if (RBX::_internal::_debugHook == 0
						|| RBX::_internal::_debugHook("0",
							"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatOutput.cpp",
							775) == 0)
					{
						ReleaseAssert(FLog::Asserts,
							"0 file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/ChatOutput.cpp line: 775");
					}
				}
				break;

			case ChatLine::CHATLINE_TYPE_GAME_OTHER:
				// the part-Head arm (0x5C3C62-0x5C3C9x decode): the origin
				// IS the Head part of a model (isA PartInstance + the name
				// + the parent-model gates); the workspace finds once.
				if (origin->getDescriptor().isA(PartInstance::classDescriptor())
					&& origin->getName() == "Head"
					&& origin->getParent() != NULL
					&& origin->getParent()->getDescriptor().isA(ModelInstance::classDescriptor()))
				{
					fade = 0.0f;
					if (workspace == NULL)
						workspace = ServiceProvider::find<Workspace>(origin.get());
					yShift = 1.0f;
					part = static_cast<PartInstance*>(origin.get());
				}
				else
				{
					yShift = 1.0f;
				}
				break;

			default:
				break;
			}

			// the common v52/v54 pair gates the render head; the a4=false
			// path reaches it only when the front line's chatType >= 3
			// (raw 0x5C3A2C).
			if (!(workspace != NULL && part != NULL))
				continue;
			if (!(v || front->chatType >= 3))
				continue;

			// LABEL_49 (0x5C3A3C-0x5C3C1x): the camera depth gate.  The
			// binary v-calls Workspace::getCamera twice (the head + the
			// bubbles arm 0x5C3C1D); landed reuses one local.
			Camera* camera = workspace->getCamera();
			const G3D::CoordinateFrame& cameraCF = camera->coordinateFrame();
			const G3D::CoordinateFrame& partCF = part->getCoordinateFrame();

			// the raw product order (0x5C3A5B-0x5C3A8C): xmm1 = dy*m12,
			// xmm0 = dx*m02, xmm0 += xmm1; then xmm1 = dz*m22 and the
			// FINAL sum = dz*m22 + (dx*m02 + dy*m12) -- the reversed add
			// tree, spelled exactly.  (The deeper summary grouping was
			// corrected against the 0x5C3A43+ raw read.)
			float depth =
				(partCF.translation.z - cameraCF.translation.z) * cameraCF.rotation[2][2]
				+ ((partCF.translation.x - cameraCF.translation.x) * cameraCF.rotation[0][2]
					+ (partCF.translation.y - cameraCF.translation.y) * cameraCF.rotation[1][2]);

			// mac: 0x5C3A93/0x5C3AAB jbe: the unordered NaN cases TAKE the
			// branch, so the landed predicates are the !-flipped compares.
			if (!(depth > 0.0f))
			{
				if (!(-90.0f > depth))
				{
					// the bubbles arm (0x5C3C1D-0x5C3F46): the frustum
					// build (the rect pin {0,0,1216,612}) + the render-CF
					// containment + the velocity max-speed gate, then the
					// mf6 bind install (0x5C3EAB).
					RBX::Frustum frus = camera->frustum(G3D::Rect2D::xyxy(
						0.0f, 0.0f, 1216.0f, 612.0f));
					const G3D::CoordinateFrame renderCF = part->getRenderingCoordinateFrame();
					cc.flagDecay = frus.containsPoint(renderCF.translation);

					const G3D::Vector3& velocity = part->getVelocity();
					// the raw multiply/add order (0x5C3CD4-0x5C3CEE):
					// xmm1 = y*y; xmm0 = x*x; xmm0 += xmm1; xmm1 = z*z;
					// xmm1 += xmm0 -- z*z + (x*x + y*y).
					float speed2 = velocity.z * velocity.z
						+ (velocity.x * velocity.x + velocity.y * velocity.y);

					// the eps-dance (0x5C3CFA-0x5C3D60): atMax starts
					// true; the |speed2| route rescales the 1e-5f limit
					// (the double promotions live; the raw's 0x5C3D02
					// jnz/jnp pair = the != route).
					bool atMax = true;
					if (speed2 != 0.0f)
					{
						double d = fabsf(speed2);
						double limit = 1e-5f;
						if (d + 1.0 != (double)G3D::inf())
							limit = (d + 1.0) * 1e-5f;
						atMax = limit >= d;
					}
					cc.flagDecay3 = !atMax;

					// the mf6 bind install (the 0x5C3D6x-0x5C3EAB staging):
					// (this, _2, wk-inst, wk-part, a4, V(0,v51,0),
					// V(0,0,v53)); the billboard must re-lock non-null.
					{
						boost::shared_ptr<BillboardGui> billboard = cc.billboard.lock();
						if (billboard.get() != NULL)
							billboard->setRenderFunction(boost::bind(
								&ChatOutput::renderBubbles, this, _2,
								RBX::weak_from<Instance const>(it->first),
								RBX::weak_from<PartInstance>(part),
								v,
								G3D::Vector3(0.0f, yShift, 0.0f),
								G3D::Vector3(0.0f, 0.0f, fade)));
					}
				}
				else
				{
					// the imposters arm (0x5C3AB1-0x5C3B9x): the billboard
					// re-locks non-null, then the mf3 bind install
					// (this, _2, wk-inst, wk-part).
					{
						boost::shared_ptr<BillboardGui> billboard = cc.billboard.lock();
						if (billboard.get() != NULL)
							billboard->setRenderFunction(boost::bind(
								&ChatOutput::renderBubbleImposters, this, _2,
								RBX::weak_from<Instance const>(it->first),
								RBX::weak_from<PartInstance>(part)));
					}
				}
			}
		}   // the ++it walk (_Rb_tree_increment in the binary)
	}

	// mac: 0x5C2774 (538B .. 0x5C2989; rarc28 decomp 60-136 + raw 139-299):
	// onServiceProvider -- the three cached connections refresh.
	//   mac: 0x5C2789-0x5C27AB: heartbeatConn (+304 = 0x130) /
	//   chatMessageConn (+308 = 0x134) / chattedConn (+312 = 0x138)
	//   all disconnect FIRST.
	//   mac: 0x5C27B9-BB: newProvider == NULL -> 0x5C2936 = players (+112 =
	//   0x70) = 0 + the bare epilogue.
	//   mac: 0x5C27C4-0x5C2837: findServiceProvider(create)<RunService> --
	//   the signal storage = RunService+0x9C (the landed
	//   heartbeatSignal); the connect runs bind(mf1 onHeartbeat); the
	//   landed rbx::remote_signal trick doesn't apply here, this is
	//   the plain signal.  The rebind pattern:
	//   connection::operator!= (0x5C2808) -> disconnect (0x5C2814) +
	//   connection::operator= (0x5C2820); the temp islot release
	//   (0x5C2832) folds into the connection copy.
	//   mac: 0x5C283D-0x5C28B0: findServiceProvider(create)<Network::Players>
	//   -> the store @+112 (0x70); the +0xF8 message signal (the landed
	//   Players::messageReceivedSignal) <- bind(mf1 onPlayerChatMessage)
	//   with the same rebind dance (0x5C2884/0x5C2890/0x5C289C).
	//   mac: 0x5C28B6-0x5C2934: findServiceProvider(create)<ChatService>;
	//   mac: 0x5C28DA-E0 = the remote_signal's connectionSignal probe
	//   (+0x64) firing BEFORE the connect (the landed
	//   rbx::remote_signal::connect runs the same order internally) +
	//   the +0x60 signal <- bind(mf3 onGameChatMessage) (0x5C28F9) and
	//   the rebind dance (0x5C2908/0x5C2914/0x5C2920).
	// STUB: PLAYER 0x00973ea0
	// STUB: STUDIO 0x00a22aa0
	void ChatOutput::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		(void)oldProvider;
		heartbeatConn.disconnect();
		chatMessageConn.disconnect();
		chattedConn.disconnect();

		if (newProvider == NULL)
		{
			players = NULL;   // 0x5C2936
			return;
		}

		RunService* runService = ServiceProvider::create<RunService>(newProvider);   // 0x5C27C4
		if (runService != NULL)
		{
			rbx::signals::connection newHeartbeat = runService->heartbeatSignal.connect(
				boost::bind(&ChatOutput::onHeartbeat, this, _1));   // 0x5C27F9
			if (heartbeatConn != newHeartbeat)
			{
				heartbeatConn.disconnect();
				heartbeatConn = newHeartbeat;
			}
		}

		players = ServiceProvider::create<Network::Players>(newProvider);   // 0x5C283D
		if (players != NULL)
		{
			rbx::signals::connection newChatMessage = players->messageReceivedSignal.connect(
				boost::bind(&ChatOutput::onPlayerChatMessage, this, _1));   // 0x5C2875
			if (chatMessageConn != newChatMessage)
			{
				chatMessageConn.disconnect();
				chatMessageConn = newChatMessage;
			}
		}

		ChatService* chatService = ServiceProvider::create<ChatService>(newProvider);   // 0x5C28B6
		if (chatService != NULL)
		{
			// the remote_signal's connectionSignal probe fires inside the
			// landed connect (the binary spells it as the separate
			// signal_with_args<0>::operator() call at 0x5C28DA-E0).
			rbx::signals::connection newChatted = chatService->chattedSignal.connect(
				boost::bind(&ChatOutput::onGameChatMessage, this, _1, _2, _3));   // 0x5C28F9
			if (chattedConn != newChatted)
			{
				chattedConn.disconnect();
				chattedConn = newChatted;
			}
		}
	}

	// mac: 0x5C4B88 (824B, rarc18.txt full raw+decompile; the mf3 bound
	// route = the far-slot imposter render).  ORDER:
	// (a) the two sp_nothrow locks -- part (5C4BA1) then Instance
	// const origin (5C4BBE -- the weak_ptr.lock() boot), either NULL
	// = the release-tail return;
	// (b) ServiceProvider::find<Workspace> off the PART (5C4BD7, the
	// receiver = the part px);
	// (c) the ViewportBillboarder temp: V3/V3/V2 all zero
	// (5C4BE6..5C4C2C), the UDim2 dwords {40400000h,0,40666666h,0} =
	// UDim2(3.0f,0,3.6f,0) (5C4C36..5C4C53), pivot = (const Vector2*)
	// NULL (the 6-arg C1 0x8212CC);
	// (d) camera = the workspace-vptr+0xC8 vcall (5C4C9A..5C4CA5 =
	// the landed Workspace::getCamera witness, cf. the head :710);
	// (e) partSize = the landed 28B getPartSizeXml (5C4CAC -- the
	// movsd qword + dword copies = the Vector3 copy out of the
	// const&);
	// (f) the CoordinateFrame temp = the Matrix3 copy ctor from cf
	// (5C4CDD) + the translation words ([cf+0x24] movsd, [cf+0x2C],
	// 5C4CE2..5C4CF3) = a plain memberwise CF copy;
	// (g) AdornBillboarder impostor(adorn, camera, partSize, frame,
	// billboarder) (the landed 205B C2 = vp.update with the part's
	// extents+CF);
	// (h) gated on the landed 15B isVisibleAndValid (5C4D3A):
	// chats[origin.get()] = the insert-empty operator[] (5C4D69);
	// idx = the GCC-inlined deque distance over the 48B GCC deque
	// internals (5C4D70..5C4D8D: 16*(fin.node - sta.node) +
	// ((sta.last - sta.cur) >> 3) + ((fin.cur - fin.first) >> 3) --
	// the <<4 folds the node-diff in slots x16 chunks) minus 65 =
	// the size()+64 fold then -64-1 -- so the landed spell = size()-1,
	// the LAST-LINE index (state-checked: empty -> 64 - 65 = -1 exits
	// the loop untouched; one line -> 0 = its index; the start-node
	// tail term carries front pops);
	// (i) the backwalk while idx != -1 (5C4D94): probe =
	// *(lines.begin() + idx) (the extern _Deque_iterator::+=
	// 5C4DBC), the gate = chatType@+0xC != ChatLine::CHATLINE_TYPE_3
	// (5C4DC6), the walk step idx-- (5C4DC3) only on the miss;
	// (j) the pass branch RE-ADVANCES from a fresh begin() at the
	// SAME idx (5C4E4 inc undoes the dec; the extern += can't CSE --
	// two fresh iterator copies spell the two advances); the render
	// = the +212 bubbleNotifications map operator[] keyed on
	// bubbleColor@+8 (5C4E07 -- insert-or-find; the C2 seeds all four
	// colors), the px taken RAW via .get() (zero refcount touched;
	// a NULL px reaches legacyRender2d whose +0x34 parent guard =
	// the silent no-op -- the binary shape), viewport =
	// impostor.getViewport() (5C4E21, the 55B memberwise vp->rect
	// copy), then gui->legacyRender2d(impostor, viewport) (5C4E3A:
	// __ZN3RBX9GuiObject14legacyRender2dEPNS_5AdornERKN3G3D6Rect2DE
	// -- the AdornBillboarder IS-A Gui/Adorn.h's Adorn);
	// (k) scope death: the AB's Adorn D2 (5C4E48) + the two lock
	// releases (5C4E57/5C4E66) = the locals' implicit dtors.
	void ChatOutput::renderBubbleImposters(class RBX::Adorn* adorn,
		boost::weak_ptr<Instance const> origin,
		boost::weak_ptr<PartInstance> part)
	{
		boost::shared_ptr<PartInstance> partLocked = part.lock();
		if (partLocked.get() != NULL)
		{
			boost::shared_ptr<Instance const> originLocked = origin.lock();
			if (originLocked.get() != NULL)
			{
				Workspace* workspace =
					ServiceProvider::find<Workspace>(partLocked.get());
				if (workspace != NULL)
				{
					ViewportBillboarder billboarder(
						G3D::Vector3(0.0f, 0.0f, 0.0f),
						G3D::Vector3(0.0f, 0.0f, 0.0f),
						G3D::Vector2(0.0f, 0.0f),
						UDim2(3.0f, 0, 3.6f, 0),
						NULL);

					Camera* camera = workspace->getCamera();
					G3D::Vector3 partSize = partLocked->getPartSizeXml();
					G3D::CoordinateFrame frame =
						partLocked->getCoordinateFrame();

					AdornBillboarder impostor(adorn, *camera, partSize,
						frame, billboarder);

					if (impostor.isVisibleAndValid())
					{
						CharacterChats& cc = chats[originLocked.get()];
						int idx = (int)cc.lines.size() - 1;
						while (idx != -1)
						{
							shared_ptr<ChatLine>& probe =
								*(cc.lines.begin() + idx);
							if (probe->chatType != ChatLine::CHATLINE_TYPE_3)
							{
								// the pass branch re-advances from a
								// fresh begin() at the same idx (the
								// extern += cannot CSE).
								shared_ptr<ChatLine>& render =
									*(cc.lines.begin() + idx);
								GuiObject* gui =
									bubbleNotifications[render->bubbleColor].get();
								G3D::Rect2D viewport = impostor.getViewport();
								gui->legacyRender2d(&impostor, viewport);
								break;
							}
							--idx;
						}
					}
				}
			}
		}
	}

	// mac: 0x5C3432: render2d.
	// STUB: PLAYER 0x00974080
	// STUB: STUDIO 0x00a22c80
	void ChatOutput::render2d(class RBX::Adorn* adorn)
	{
		bool v4 = false;
		if (players != NULL)
			v4 = (unsigned int)(*(int*)((char*)players + 220) - 1) < 2;
		render2d_bubbleStyle(adorn, v4);
		bool v3 = false;
		if (players != NULL)
			v3 = (*(int*)((char*)players + 220) & 0xFFFFFFFD) != 0;
		render2d_classicStyle(adorn, v3);
	}

} // namespace RBX