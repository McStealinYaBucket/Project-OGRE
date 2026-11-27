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
     +0x30 rbx::signal<void(DataModel*)>: its out-of-line ctor 0x00653790 makes the per-signature
           SAFE_HEAP_STATIC mutex (call_once 0x00426660, getter 0x004265c0, both the /GL shell's);
           iOS 10-29 Game::Game calls signal<void(DataModel*)>::safe_static_do_get_mutex and ~Game
           signal<void(DataModel*)>::disconnectAll. Its dtor is 0x00654e80.
   getDataModel() is inline: the /GL shell keeps an out-of-line copy (PLAYER 0x0041ce40, this in ecx,
   return slot in eax). */

#pragma once

#include "decomp.h"
#include "rbx/signal.h"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <string>
#include <vector>

namespace RBX {

	class CommonVerbs;
	class DataModel;
	class OverlayDataModel;
	class ProfanityFilter;
	class Verb;

	// VTABLE: PLAYER 0x00b31044
	// VTABLE: STUDIO 0x00c0a920
	// SIZE 0x34
	class Game : boost::noncopyable
	{
	protected:
		Game(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen);

	public:
		virtual ~Game();	// vtable+0x00

		// Its scalar deleting destructor folds with SecurePlayerGame's (PLAYER 0x00656230) and
		// UnsecuredStudioGame's (STUDIO 0x007ce200); the PDB keeps the subclass name, so they carry the markers.

		boost::shared_ptr<DataModel> getDataModel() const { return dataModel; }

		static void globalExit();

		void setupDataModel(const std::string& baseUrl);
		void setDataModel(boost::shared_ptr<DataModel> dataModel);
		void clearVerbs(bool useLock);
		void shutdownGameDataModel();

	protected:
		void doClearVerbs();
		void shutdown();

		boost::shared_ptr<DataModel> dataModel;					// 0x04
		boost::shared_ptr<OverlayDataModel> overlayDataModel;	// 0x0c
		bool hasShutdown;										// 0x14
	public:
		std::vector<Verb*> verbs;								// 0x18
	protected:
		boost::shared_ptr<CommonVerbs> commonVerbs;				// 0x28 (2016 name; the ctor only zeroes it)
		rbx::signal<void(DataModel*)> m_unk0x30;				// 0x30 (no 2016 counterpart; unnamed)
	};

	// PLAYER only: its deleting destructor folds with Game's (both vftable slots are 0x00656230).
	// VTABLE: PLAYER 0x00b3104c
	// SIZE 0x34
	class SecurePlayerGame : public Game
	{
	public:
		SecurePlayerGame(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen);

		// SYNTHETIC: PLAYER 0x00656230
		// RBX::SecurePlayerGame::`scalar deleting destructor'
	};

	// STUDIO only (RTTI .?AVUnsecuredStudioGame@RBX@@; iOS 10-29 has it too). Its deleting destructor
	// folds with Game's, as SecurePlayerGame's does in PLAYER.
	// VTABLE: STUDIO 0x00c0a928
	// SIZE 0x34
	class UnsecuredStudioGame : public Game
	{
	public:
		UnsecuredStudioGame(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen);

		// SYNTHETIC: STUDIO 0x007ce200
		// RBX::UnsecuredStudioGame::`scalar deleting destructor'
	};

	// Game::m_unk0x30 (rbx::signal<void(DataModel*)>): its out-of-line members. PLAYER keeps the /GL shell's
	// mutex helpers (0x004265c0/0x00426660), STUDIO Game.obj's; the rest are Game.obj copies in both.
	// TEMPLATE: PLAYER 0x004265c0
	// TEMPLATE: STUDIO 0x007cb980
	// ?safe_static_do_get_mutex@?$signal@$$A6AXPAVDataModel@RBX@@@Z@signals@rbx@@CAPAVmutex@boost@@XZ

	// TEMPLATE: PLAYER 0x00426660
	// TEMPLATE: STUDIO 0x007cba10
	// ?safe_static_init_mutex@?$signal@$$A6AXPAVDataModel@RBX@@@Z@signals@rbx@@CAXXZ

	// TEMPLATE: PLAYER 0x00653790
	// TEMPLATE: STUDIO 0x007cc250
	// ??0?$signal@$$A6AXPAVDataModel@RBX@@@Z@signals@rbx@@QAE@XZ

	// TEMPLATE: PLAYER 0x00654790
	// TEMPLATE: STUDIO 0x007cd150
	// ?disconnectAll@?$signal@$$A6AXPAVDataModel@RBX@@@Z@signals@rbx@@QAEXXZ

	// TEMPLATE: PLAYER 0x00654dc0
	// ??R?$signal_with_args@$00$$A6AXPAVDataModel@RBX@@@Z@signals@rbx@@QAEXPAVDataModel@RBX@@@Z

	// TEMPLATE: PLAYER 0x00654e80
	// TEMPLATE: STUDIO 0x007cd5c0
	// ??1?$signal@$$A6AXPAVDataModel@RBX@@@Z@signals@rbx@@QAE@XZ

} // namespace RBX
