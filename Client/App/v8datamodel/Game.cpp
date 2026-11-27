/* App/v8datamodel/Game.cpp -- October 2013, partial (globalInit not written; CommonVerbs is opaque).
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
#include "V8DataModel/Commands.h"
#include "V8DataModel/CommonVerbs.h"
#include "V8DataModel/Workspace.h"

FASTFLAG(OverlayDataModelEnabled)

#include "security/SecurityContext.h"
#include "Network/API.h"

#include "decomp.h"

#ifndef OGRE_TARGET_STUDIO
// iOS keeps it in __MergedGlobals (a file static).
// GLOBAL: PLAYER 0x00de68b4
static boost::once_flag initWithPlayerSecurityFlag = BOOST_ONCE_INIT;
#else
// GLOBAL: STUDIO 0x00f89340
static boost::once_flag initWithoutSecurityFlag = BOOST_ONCE_INIT;
#endif

namespace RBX {



	// CommonVerbs' destructor, emitted here in 2013 (Game.obj): the members' destructors, last first.
	// FUNCTION: PLAYER 0x006531e0
	// FUNCTION: STUDIO 0x007cbbb0
	CommonVerbs::~CommonVerbs()
	{
	}

	// iOS 10-29: void RBX::shutdownDM<DataModel>(shared_ptr<DataModel>&) and <OverlayDataModel>; the two
	// PLAYER copies are byte-identical and fold (STUDIO keeps two: 0x007cc000, 0x007cc090).
	// FUNCTION: PLAYER 0x00653600
	template<class T>
	void shutdownDM(shared_ptr<T>& dm)
	{
		if (dm)
		{
			{
				DataModel::LegacyLock lock(dm.get(), DataModelJob::Write);
				DataModel::closeDataModel(dm, true);
			}
			dm.reset();
		}
	}

	// 2016 name; globalInit (PLAYER 0x00654ca0, not written) assigns ScopedSingleton<ProfanityFilter>::getInstance().
	// GLOBAL: PLAYER 0x00de68d0
	// GLOBAL: STUDIO 0x00f8935c
	static boost::shared_ptr<ProfanityFilter> s_profanityFilter;

	// FUNCTION: PLAYER 0x00653780
	// FUNCTION: STUDIO 0x007cc240
	void Game::globalExit()
	{
		s_profanityFilter.reset();
	}

	// FUNCTION: PLAYER 0x006542b0
	// FUNCTION: STUDIO 0x007ccca0
	void Game::doClearVerbs()
	{
		if (dataModel)
		{
			Security::Impersonator impersonate(Security::COM);
			commonVerbs.reset();
			for (std::vector<Verb*>::iterator iter = verbs.begin(), end = verbs.end(); iter != end; ++iter)
				delete *iter;
			verbs.clear();
		}
	}

	// FUNCTION: PLAYER 0x00654360
	// FUNCTION: STUDIO 0x007ccd50
	void Game::clearVerbs(bool useLock)
	{
		if (useLock)
		{
			DataModel::LegacyLock lock(dataModel, DataModelJob::Write);
			doClearVerbs();
		}
		else
			doClearVerbs();
	}

	// View::resetScheduler calls it (PLAYER 0x00466ed9); STUDIO has no copy. clearVerbs(false) inlines with its dead LegacyLock branch, which leaves the EH frame.
	// FUNCTION: PLAYER 0x00654400
	void Game::shutdownGameDataModel()
	{
		clearVerbs(false);
		if (dataModel)
		{
			DataModel::closeDataModel(dataModel, false);
			dataModel.reset();
		}
	}

	// iOS 10-29 setupDataModel(std::string const&); the camera verbs' ctors are inline (Commands.h).
	// FUNCTION: PLAYER 0x00655630
	// STUB: STUDIO 0x007cd6d0
	void Game::setupDataModel(const std::string& baseUrl)
	{
		dataModel->create<ScriptInformationProvider>()->setAssetUrl(baseUrl + "/asset/");
		dataModel->create<ContentProvider>()->setBaseUrl(baseUrl);
		dataModel->game = this;

		commonVerbs.reset(new CommonVerbs(dataModel.get()));

		verbs.push_back(new CameraPanLeftCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraPanRightCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraTiltUpCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraTiltDownCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraZoomInCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraZoomOutCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraCenterCommand(dataModel->getWorkspace()));
		verbs.push_back(new CameraZoomExtentsCommand(dataModel->getWorkspace()));
		verbs.push_back(new ToggleViewMode(dataModel.get()));
	}

	// View::resetScheduler calls it (PLAYER 0x00466d48); STUDIO has no copy.
	// FUNCTION: PLAYER 0x00655bf0
	void Game::setDataModel(shared_ptr<DataModel> dataModel)
	{
		if (dataModel)
		{
			this->dataModel = dataModel;
			if (overlayDataModel)
			{
				overlayDataModel->FUN_00652f00(dataModel);
				if (ContentProvider* contentProvider = ServiceProvider::create<ContentProvider>(overlayDataModel.get()))
					this->dataModel->submitTask(boost::bind(&Game::setupDataModel, this, contentProvider->getBaseUrl()), DataModelJob::Write);
			}
		}
		m_unk0x30(dataModel.get());
	}

	// Inline into ~Game in 2013 (iOS keeps it out of line).
	void Game::shutdown()
	{
		if (hasShutdown)
			return;
		hasShutdown = true;
		clearVerbs(true);
		shutdownDM(dataModel);
		if (overlayDataModel)
			shutdownDM(overlayDataModel);
	}

	// FUNCTION: PLAYER 0x00655d30
	// STUB: STUDIO 0x007cdc90
	Game::~Game()
	{
		shutdown();
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

#ifndef OGRE_TARGET_STUDIO
	// The shell's `new SecurePlayerGame` is the only caller.
	// FUNCTION: PLAYER 0x00656250
	SecurePlayerGame::SecurePlayerGame(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen)
		: Game(lockVerb, baseUrl, shouldShowLoadingScreen)
	{
		boost::call_once(&Network::initWithPlayerSecurity, initWithPlayerSecurityFlag);
	}
#else
	// FUNCTION: STUDIO 0x007ce190
	UnsecuredStudioGame::UnsecuredStudioGame(Verb* lockVerb, const char* baseUrl, bool shouldShowLoadingScreen)
		: Game(lockVerb, baseUrl, shouldShowLoadingScreen)
	{
		boost::call_once(&Network::initWithoutSecurity, initWithoutSecurityFlag);
	}
#endif

} // namespace RBX

#ifdef OGRE_TARGET_STUDIO
DECOMP_SIZE_ASSERT(RBX::UnsecuredStudioGame, 0x34);
#else
DECOMP_SIZE_ASSERT(RBX::SecurePlayerGame, 0x34);
#endif
