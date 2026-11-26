/* V8DataModel/Game.h -- October 2013.
   RBX::Game (RTTI .?AVGame@RBX@@, vftable PLAYER 0x00b31044: one slot, the deleting destructor
   0x00656230) and RBX::SecurePlayerGame (vftable 0x00b3104c, ctor 0x00656250). No 2013 __FILE__ names
   the header; its path follows the tree's V8DataModel include directory.

   Layout from Game::Game(Verb*, const char*, bool) (PLAYER 0x00655e70, ret 0xc; ios: name) and the
   shell's `new` (0x34 bytes, PLAYER 0x0041d990):
     +0x04 dataModel          DataModel::createDataModel(true, lockVerb, overlay) 0x0064fc30
     +0x0c overlayDataModel   OverlayDataModel::createOverlayDataModel 0x00693310 (only when the
                              bool argument and a flag at 0x00de98d0 are set)
     +0x14 bool, zeroed
     +0x18 std::vector<Verb*> verbs (three zeroed pointers at +0x1c)
     +0x28 shared_ptr, zeroed (2016: commonVerbs)
     +0x30 a 4-byte member built by 0x00653790 (zeroes itself, then call_once + 0x004265c0)
   getDataModel() is inline: the /GL shell keeps an out-of-line copy (PLAYER 0x0041ce40, this in ecx,
   return slot in eax). */

#pragma once

#include "decomp.h"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <string>
#include <vector>

namespace RBX {

	class CommonVerbs;
	class DataModel;
	class OverlayDataModel;
	class Verb;

	// VTABLE: PLAYER 0x00b31044
	// SIZE 0x34
	class Game : boost::noncopyable
	{
	protected:
		Game(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen);

	public:
		virtual ~Game();	// vtable+0x00

		// SYNTHETIC: PLAYER 0x00656230
		// RBX::Game::`scalar deleting destructor'

		boost::shared_ptr<DataModel> getDataModel() const { return dataModel; }

		void setupDataModel(const std::string& baseUrl);

	protected:
		// The member at +0x30: a 4-byte object whose ctor (PLAYER 0x00653790) zeroes it and touches a
		// SAFE_HEAP_STATIC (call_once 0x00426660, getter 0x004265c0); its dtor is 0x00654e80.
		struct Unk0x30
		{
			undefined4 m_unk0x00;
			Unk0x30();
			~Unk0x30();
		};

		boost::shared_ptr<DataModel> dataModel;					// 0x04
		boost::shared_ptr<OverlayDataModel> overlayDataModel;	// 0x0c
		bool hasShutdown;										// 0x14
	public:
		std::vector<Verb*> verbs;								// 0x18
	protected:
		boost::shared_ptr<CommonVerbs> commonVerbs;				// 0x28 (2016 name; the ctor only zeroes it)
		Unk0x30 m_unk0x30;										// 0x30
	};

	class SecurePlayerGame : public Game
	{
	public:
		SecurePlayerGame(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen);
	};

} // namespace RBX
