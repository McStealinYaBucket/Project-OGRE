/* App/v8datamodel/Game.cpp -- October 2013, partial: the Game constructor and STUBs for the rest.
   The 2013 ctor (PLAYER 0x00655e70, iOS callees) differs from 2016's: when the loading screen is
   asked for and FFlag::OverlayDataModelEnabled is set, it builds an OverlayDataModel first, hands it
   the game DataModel, and sets up the overlay's services under a write LegacyLock. */

#include "stdafx.h"

#include "V8DataModel/Game.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/OverlayDataModel.h"
#include "V8DataModel/ScriptInformationProvider.h"
#include "V8DataModel/ContentProvider.h"
#include "V8DataModel/UserInputService.h"
#include "V8DataModel/UserController.h"
#include "V8DataModel/RunService.h"
#include "V8Tree/Verb.h"

FASTFLAG(OverlayDataModelEnabled)

#include "decomp.h"

namespace RBX {

	static void unk0x30Init() {}

	// __declspec(noinline) is a stand-in until the real bodies land (VC9 inlines a same-TU STUB).
	// STUB: PLAYER 0x00653790
	// STUB: STUDIO 0x007cc250
	__declspec(noinline) Game::Unk0x30::Unk0x30()
		: m_unk0x00(0)
	{
		STUB(0x00653790);
		// Also a stand-in: one opaque call (the real body makes a call_once too) so VC9 does not
		// prove this stub nothrow and drop the Game ctor's EH state 3.
		static boost::once_flag flag = BOOST_ONCE_INIT;
		boost::call_once(&unk0x30Init, flag);
	}

	// STUB: PLAYER 0x00654e80
	// STUB: STUDIO 0x007cd5c0
	__declspec(noinline) Game::Unk0x30::~Unk0x30()
	{
		STUB(0x00654e80);
	}

	// Not written yet (1446 bytes). STUB body so the bind target pairs.
	// STUB: PLAYER 0x00655630
	// STUB: STUDIO 0x007cd6d0
	void Game::setupDataModel(const std::string& baseUrl)
	{
		STUB(0x00655630);
	}

	// Not written yet (shutdown 0x00654360, then the members). STUB body.
	// STUB: PLAYER 0x00655d30
	// STUB: STUDIO 0x007cdc90
	Game::~Game()
	{
		STUB(0x00655d30);
	}

	// FUNCTION: PLAYER 0x00655e70
	// STUB: STUDIO 0x007cddd0
	Game::Game(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen)
		: hasShutdown(false)
	{
		if (shouldShowLoadingScreen && FFlag::OverlayDataModelEnabled)
			overlayDataModel = OverlayDataModel::createOverlayDataModel(true, new NullVerb(NULL, ""));

		dataModel = DataModel::createDataModel(true, lockVerb, overlayDataModel.get());

		if (shouldShowLoadingScreen && FFlag::OverlayDataModelEnabled)
		{
			// 2013 copies dataModel into a shared_ptr temporary, then converts it to the weak_ptr
			// argument (use count +1, then weak count +1); this spelling is byte-identical.
			overlayDataModel->FUN_00652f00(shared_ptr<DataModel>(dataModel));

			DataModel::LegacyLock lock(overlayDataModel, DataModelJob::Write);
			overlayDataModel->game = this;
			overlayDataModel->create<ScriptInformationProvider>()->setAssetUrl(std::string(baseUrl) + "/asset/");
			overlayDataModel->create<ContentProvider>()->setBaseUrl(baseUrl);
			if (UserInputService* userInputService = ServiceProvider::create<UserInputService>(overlayDataModel.get()))
				userInputService->setProcessToolCommandsSeparately(true);
			ServiceProvider::create<ControllerService>(overlayDataModel.get());
			ServiceProvider::create<RunService>(overlayDataModel.get())->setRunState(RS_RUNNING);
		}

		dataModel->submitTask(boost::bind(&Game::setupDataModel, this, std::string(baseUrl)), DataModelJob::Write);
	}

} // namespace RBX
