/* DataModel.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     - ctor 0x2D41E0 (F:\2013Decompile\exports\decomp\DataModel_ctor.txt)
     - createDataModel 0x2D3742: (bool, Verb*, DataModel*) — Creatable<Instance>::create
     - getGameTime 0x2D83C4: asserts "runService" (+736 member), reads gameTime at runService+116;
       original file /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DataModel.cpp line 1485
     - Scalar setters below from F:\2013Decompile\exports\decomp\Refl_DataModel.cpp.txt:
       setRemoteBuildMode 0x2CDEC2 (*(this+3440)=a2, Mach-O); getRemoteBuildMode 0x2CDED4;
       setServerSaveUrl 0x2CDEE4 (assign string at +3444);
       setScreenshotSEOInfo 0x2D1664 (+3448); setVideoSEOInfo 0x2D1686 (+3452);
       setPlaceVersion 0x2D1442 (+3464, FastLog "Setting place version");
       setPlaceID 0x2D14A4 (+3456 + ScriptContext::setRobloxPlace wiring — TODO);
       setCreatorID 0x2D153C (+3468/+3472); setGenre 0x2D159E (+3476);
       setGear 0x2D15DA (+3480/+3481 + allowedGearTypeChanged signal at +2784);
       isGearTypeAllowed 0x2D164A (bittest on +3484);
       activateExperimentalFeatures 0x2D152C (*(this+3460)=1);
       canSaveLocal 0x2D01B2 (returns 0 — 2016 returns true on Windows);
       getSyncronizationArbiter 0x2D2E70 (parentDM null ? this+184 : parentDM+184);
       setUiMessage 0x2D04F2 / clearUiMessage 0x2D0514 (string at +2988);
       getPlaceID 0x2E425A (int at +3456).
   TODO(decompile, needs Visit/Workspace/ScriptContext/GuiBuilder + reflection block):
     canSave 0x2D257A (static, find<Visit> + uploadUrl), close 0x2CFF76 (getVerb "Exit"),
     toggleToolsOff 0x2D00E4 (getVerb "TogglePlayMode"), completeShutdown 0x2D038C,
     requestShutdown 0x2D2CBA, setPlaceID ScriptContext/RbxDbgInfo wiring,
     raisePropertyChanged descriptor wiring for all setters (reflection block pending),
     addCustomStat/removeCustomStat/writeStatsSettings (GuiBuilder at +3488),
     httpGet/httpPost/doHttpGet/doHttpPost + async (0x2CDF06/0x2CE086/0x2CE20E/0x2CE4F2),
     loadWorld 0x2CD928 / loadGame 0x2CDA8C / loadContent 0x2CDBAA / save 0x2CDDFE,
     getJobsInfo 0x2CE8BE / getJobsExtendedStats 0x2D0548 / peak fractions / stats window,
     saveToRoblox 0x2D01BA, serverSavePlace 0x2D25EA,
     doDataModelSetup 0x2D2EB2 (GenericJobs wiring). */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/DataModel.h"
#include "V8DataModel/DataModelJob.h"
#include "V8DataModel/Visit.h"
#include "V8DataModel/Workspace.h"
#include "V8DataModel/Lighting.h"
#include "V8DataModel/GuiRoot.h"
#include "Gui/GuiEvent.h"
#include "V8DataModel/ContentProvider.h"
#include "V8DataModel/ContentFilter.h"
#include "V8DataModel/KeyframeSequenceProvider.h"
#include "V8DataModel/GuiService.h"
#include "V8DataModel/ChatService.h"
#include "V8DataModel/MarketplaceService.h"
#include "V8DataModel/LocalBackpack.h"
#include "V8DataModel/PlayerHUD.h"
#include "V8DataModel/StarterPackService.h"
#include "V8DataModel/StarterGuiService.h"
#include "V8DataModel/CoreGuiService.h"
#include "V8DataModel/RunService.h"
#include "V8DataModel/JointsService.h"
#include "V8DataModel/CollectionService.h"
#include "V8DataModel/PhysicsService.h"
#include "V8DataModel/BadgeService.h"
#include "V8DataModel/GeometryService.h"
#include "V8DataModel/FriendService.h"
#include "V8DataModel/RenderHooksService.h"
#include "V8DataModel/InsertService.h"
#include "V8DataModel/SocialService.h"
#include "V8DataModel/GamePassService.h"
#include "V8DataModel/DebrisService.h"
#include "V8DataModel/ScriptInformationProvider.h"
#include "V8DataModel/CookiesService.h"
#include "V8DataModel/TeleportService.h"
#include "V8DataModel/PersonalServerService.h"
#include "V8DataModel/FWService.h"
#include "V8DataModel/UserInputService.h"
#include "V8DataModel/ContextActionService.h"
#include "V8DataModel/ScriptService.h"
#include "V8DataModel/AssetService.h"
#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/Teams.h"
#include "V8DataModel/SpawnerService.h"
#include "V8DataModel/ChangeHistoryService.h"
#include "V8DataModel/TestService.h"
#include "V8DataModel/ServerScriptService.h"
#include "V8DataModel/ReplicatedStorage.h"
#include "V8DataModel/ServerStorage.h"
#include "V8DataModel/PartInstance.h"
#include "V8DataModel/BaseScript.h"
#include "Network/Players.h"
#include "Util/SoundService.h"
#include "Util/Http.h"
#include "rbx/Thread.hpp"
#include "rbx/Profiler.h"
#include "rbx/TaskScheduler.h"
#include "security/SecurityContext.h"
#include "Util/Utilities.h"
#include "V8Xml/SerializerV2.h"
#include "V8Xml/SerializerBinary.h"
#include "Util/standardout.h"
#include "Util/ContentId.h"
#include "Util/RbxStringTable.h"
#include "Util/ProtectedString.h"
#include "rbx/Log.h"
#include "rbx/RbxDbgInfo.h"

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>

LOGGROUP(CloseDataModel)
LOGGROUP(LegacyLock)

// Registered by DataModel.cpp's static initializers (PLAYER 0x00a665a0 / 0x00a665c0, between
// MachineIdUploader's flag and the "ItemChanged" event); both start false (.bss).
FASTFLAGVARIABLE(NewLightAPI, false)			// GLOBAL 0x00de4760 (STUDIO 0x00f83b31)
FASTFLAGVARIABLE(ImageRectEnabled, false)		// GLOBAL 0x00de4761

namespace RBX {

	//////////////////////////////////////////////////////////////////////////////////
	// Reflection (Lua surface)
	// Evidence: every bound Lua name below was verified present as a string in BOTH
	//   the 0.126 Player PE and the 0.125 Studio PE (PE string-table scan, 57/64
	//   candidate names; the 7 absent names — GetMessage, SetGameInstanceId,
	//   SetUniverseId, ReportInGoogleAnalytics, OnClose, ForceR15, VIPServerId — are
	//   2016 additions and are NOT bound here).
	// Method EAs from F:\2013Decompile\exports\decomp\Refl_DataModel.cpp.txt.
	// Only methods WITH reconstructed bodies are bound (taking a member pointer
	// odr-uses it, so binding a bodyless method would fail at link).
	// TODO(decompile): bind Load/LoadWorld/LoadGame/Save (ContentId bodies),
	//   HttpGet/HttpPost/Async (signature port), GetJobsInfo family, stats-window/
	//   peak-fraction bodies, savePlaceAsync/saveToRoblox/reportMeasurement bodies,
	//   serverSave, completeShutdown ("FinishShutdown"), ItemChanged event (needs
	//   itemChangedSignal member), Workspace/workspace/lighting props (need
	//   Workspace.h/Lighting.h), JobId prop (2016 says std::string; verify 2013).
	// Securities carried over from March 2016 source (TODO: verify per-entry from
	//   the 2013 ClassDescriptor tables).
	//////////////////////////////////////////////////////////////////////////////////
	REFLECTION_BEGIN();
	// TODO(decompile): bind LoadPlugins/LoadWorld/LoadGame once their bodies land
	// (Lua names verified in both 2013 PEs; no reconstructed bodies yet).
	static Reflection::BoundFuncDesc<DataModel, void(bool)> setRemoteBuildMode(&DataModel::setRemoteBuildMode, "SetRemoteBuildMode", "buildModeEnabled", Security::LocalUser);
	static Reflection::BoundFuncDesc<DataModel, bool()> getRemoteBuildMode(&DataModel::getRemoteBuildMode, "GetRemoteBuildMode", Security::None);
	static Reflection::BoundFuncDesc<DataModel, void(std::string)> setServerSaveUrlFunction(&DataModel::setServerSaveUrl, "SetServerSaveUrl", "url", Security::LocalUser);
	static Reflection::BoundFuncDesc<DataModel, void()> closeFunction(&DataModel::close, "Shutdown", Security::LocalUser);
	static Reflection::BoundFuncDesc<DataModel, void()> toggleFunction(&DataModel::toggleToolsOff, "ToggleTools", Security::LocalUser);

	// HTTP Lua surface (bodies 0x2CE20E/0x2CE4F2/0x2CDF06/0x2CE086; securities = 2016
	// kHttpPermission carry-over, TODO verify from 2013 descriptor tables)
	static Reflection::BoundYieldFuncDesc<DataModel, std::string(std::string)> httpGetAsyncFunction(&DataModel::httpGetAsync, "HttpGetAsync", "url", Security::RobloxScript);
	static Reflection::BoundYieldFuncDesc<DataModel, std::string(std::string, std::string)> httpPostAsyncFunction(&DataModel::httpPostAsync, "HttpPostAsync", "url", "data", Security::RobloxScript);
	static Reflection::BoundFuncDesc<DataModel, std::string(std::string, bool)> httpGetFunction(&DataModel::httpGet, "HttpGet", "url", "synchronous", false, Security::RobloxScript);
	static Reflection::BoundFuncDesc<DataModel, std::string(std::string, std::string, bool)> httpPostFunction(&DataModel::httpPost, "HttpPost", "url", "data", "synchronous", false, Security::RobloxScript);

	static Reflection::BoundFuncDesc<DataModel, shared_ptr<const Reflection::ValueArray>()> getJobsInfo(&DataModel::getJobsInfo, "GetJobsInfo", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, shared_ptr<const Reflection::ValueArray>()> getJobsExtendedStats(&DataModel::getJobsExtendedStats, "GetJobsExtendedStats", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, double(std::string, double)> getJobTimePeakFraction(&DataModel::getJobTimePeakFraction, "GetJobTimePeakFraction", "jobname", "greaterThan", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, double(std::string, double)> getJobIntervalPeakFraction(&DataModel::getJobIntervalPeakFraction, "GetJobIntervalPeakFraction", "jobname", "greaterThan", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(double)> setJobsExtendedStatsWindow(&DataModel::setJobsExtendedStatsWindow, "SetJobsExtendedStatsWindow", "seconds", Security::LocalUser);

	static Reflection::PropDescriptor<DataModel, bool> prop_isPersonalServer("IsPersonalServer", category_State, &DataModel::getIsPersonalServer, &DataModel::setIsPersonalServer, Reflection::PropertyDescriptor::SCRIPTING, Security::RobloxScript);
	static Reflection::PropDescriptor<DataModel, bool> prop_canSaveLocal("LocalSaveEnabled", category_State, &DataModel::canSaveLocal, NULL, Reflection::PropertyDescriptor::UI, Security::RobloxScript);

	static Reflection::BoundFuncDesc<DataModel, void(std::string)> func_SetUiMessage(&DataModel::setUiMessage, "SetMessage", "message", Security::LocalUser);
	static Reflection::BoundFuncDesc<DataModel, void()> func_ClearUIMessage(&DataModel::clearUiMessage, "ClearMessage", Security::LocalUser);
	static Reflection::BoundFuncDesc<DataModel, void()> func_SetUIMessageBrickCount(&DataModel::setUiMessageBrickCount, "SetMessageBrickCount", Security::LocalUser);

	static Reflection::BoundFuncDesc<DataModel, void(int)> setPlaceVersion(&DataModel::setPlaceVersion, "SetPlaceVersion", "placeId", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(int, bool)> setPlaceId(&DataModel::setPlaceID, "SetPlaceId", "placeId", "robloxPlace", false, Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(int, bool)> setPlaceIdDeprecated(&DataModel::setPlaceID, "SetPlaceID", "placeID", "robloxPlace", false, Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(int, DataModel::CreatorType)> setCreatorId(&DataModel::setCreatorID, "SetCreatorId", "creatorId", "creatorType", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(int, DataModel::CreatorType)> setCreatorIdDeprecated(&DataModel::setCreatorID, "SetCreatorID", "creatorID", "creatorType", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(DataModel::Genre)> func_setGenre(&DataModel::setGenre, "SetGenre", "genre", Security::Plugin);
	static Reflection::BoundFuncDesc<DataModel, void(DataModel::GearGenreSetting, int)> func_setGear(&DataModel::setGear, "SetGearSettings", "genreRestriction", "allowedGenres", Security::Plugin);

	static Reflection::PropDescriptor<DataModel, int> prop_placeId("PlaceId", category_State, &DataModel::getPlaceID, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::PropDescriptor<DataModel, int> prop_placeVersion("PlaceVersion", category_State, &DataModel::getPlaceVersion, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::PropDescriptor<DataModel, int> prop_creatorId("CreatorId", category_State, &DataModel::getCreatorID, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::EnumPropDescriptor<DataModel, DataModel::CreatorType> prop_creatorType("CreatorType", category_State, &DataModel::getCreatorType, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::EnumPropDescriptor<DataModel, DataModel::Genre> prop_genre("Genre", category_State, &DataModel::getGenre, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::EnumPropDescriptor<DataModel, DataModel::GearGenreSetting> prop_gearGenreSetting("GearGenreSetting", category_State, &DataModel::getGearGenreSetting, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::BoundFuncDesc<DataModel, bool(DataModel::GearType)> func_isGearTypeAllowed(&DataModel::isGearTypeAllowed, "IsGearTypeAllowed", "gearType", Security::None);
	static Reflection::EventDesc<DataModel, void()> event_allowedGearTypeChanged(&DataModel::allowedGearTypeChanged, "AllowedGearTypeChanged");

	static Reflection::BoundFuncDesc<DataModel, void(std::string)> setScreenshotSEOInfoFunction(&DataModel::setScreenshotSEOInfo, "SetScreenshotInfo", "info", Security::LocalUser);
	static Reflection::BoundFuncDesc<DataModel, void(std::string)> setVideoSEOInfoFunction(&DataModel::setVideoSEOInfo, "SetVideoInfo", "info", Security::LocalUser);

	static Reflection::EventDesc<DataModel, void()> event_GameLoaded(&DataModel::gameLoadedSignal, "Loaded");
	static Reflection::BoundFuncDesc<DataModel, bool()> gameLoadedFunction(&DataModel::getIsGameLoaded, "IsLoaded", Security::None);
	REFLECTION_END();

	const char* const sDataModel = "DataModel";

	// BSS-zero tamper bitmask (0xFA6E5C proven in the Oct 2013 binary).
	unsigned int DataModel::sendStats = 0;

	// Class registrar (see Instance.cpp/Service.cpp note).
	RBX_REGISTER_CLASS(DataModel);

	namespace Reflection
	{
		/* EnumDesc decompiles (Mach-O EAs): CreatorType 0x2D171A, Genre 0x2D1934,
		   GearGenreSetting 0x2D1C86, GearType 0x2D1EA0. Pair order verbatim. */
		// STUB: PLAYER 0x0063d6a0
		// STUB: STUDIO 0x0075b550
		template<>
		EnumDesc<DataModel::CreatorType>::EnumDesc()
			:EnumDescriptor("CreatorType", typeid(DataModel::CreatorType))
		{
			addPair(DataModel::CREATOR_USER, "User");
			addPair(DataModel::CREATOR_GROUP, "Group");
		}

		template<>
		DataModel::CreatorType& Variant::convert<DataModel::CreatorType>(void)
		{
			return genericConvert<DataModel::CreatorType>();
		}

		template<>
		EnumDesc<DataModel::Genre>::EnumDesc()
			:EnumDescriptor("Genre", typeid(DataModel::Genre))
		{
			addPair(DataModel::GENRE_ALL, "All");
			addPair(DataModel::GENRE_TOWN_AND_CITY, "TownAndCity");
			addPair(DataModel::GENRE_FANTASY, "Fantasy");
			addPair(DataModel::GENRE_SCI_FI, "SciFi");
			addPair(DataModel::GENRE_NINJA, "Ninja");
			addPair(DataModel::GENRE_SCARY, "Scary");
			addPair(DataModel::GENRE_PIRATE, "Pirate");
			addPair(DataModel::GENRE_ADVENTURE, "Adventure");
			addPair(DataModel::GENRE_SPORTS, "Sports");
			addPair(DataModel::GENRE_FUNNY, "Funny");
			addPair(DataModel::GENRE_WILD_WEST, "WildWest");
			addPair(DataModel::GENRE_WAR, "War");
			addPair(DataModel::GENRE_SKATE_PARK, "SkatePark");
			addPair(DataModel::GENRE_TUTORIAL, "Tutorial");
		}

		template<>
		DataModel::Genre& Variant::convert<DataModel::Genre>(void)
		{
			return genericConvert<DataModel::Genre>();
		}

		template<>
		EnumDesc<DataModel::GearGenreSetting>::EnumDesc()
			:EnumDescriptor("GearGenreSetting", typeid(DataModel::GearGenreSetting))
		{
			addPair(DataModel::GEAR_GENRE_ALL, "AllGenres");
			addPair(DataModel::GEAR_GENRE_MATCH, "MatchingGenreOnly");
		}

		template<>
		DataModel::GearGenreSetting& Variant::convert<DataModel::GearGenreSetting>(void)
		{
			return genericConvert<DataModel::GearGenreSetting>();
		}

		template<>
		EnumDesc<DataModel::GearType>::EnumDesc()
			:EnumDescriptor("GearType", typeid(DataModel::GearType))
		{
			addPair(DataModel::GEAR_TYPE_MELEE_WEAPONS, "MeleeWeapons");
			addPair(DataModel::GEAR_TYPE_RANGED_WEAPONS, "RangedWeapons");
			addPair(DataModel::GEAR_TYPE_EXPLOSIVES, "Explosives");
			addPair(DataModel::GEAR_TYPE_POWER_UPS, "PowerUps");
			addPair(DataModel::GEAR_TYPE_NAVIGATION_ENHANCERS, "NavigationEnhancers");
			addPair(DataModel::GEAR_TYPE_MUSICAL_INSTRUMENTS, "MusicalInstruments");
			addPair(DataModel::GEAR_TYPE_SOCIAL_ITEMS, "SocialItems");
			addPair(DataModel::GEAR_TYPE_BUILDING_TOOLS, "BuildingTools");
			addPair(DataModel::GEAR_TYPE_PERSONAL_TRANSPORT, "Transport");
		}

		template<>
		DataModel::GearType& Variant::convert<DataModel::GearType>(void)
		{
			return genericConvert<DataModel::GearType>();
		}
	}

	// RBX_REGISTER_ENUM content expanded manually: the macro re-opens
	// `namespace RBX { namespace Reflection {`, but this TU is already inside
	// namespace RBX, so invoking it would nest RBX::RBX. Same registrations.
	namespace Reflection {
		template<>
		const Type& Type::getSingleton<DataModel::CreatorType>()
		{
			return EnumDesc<DataModel::CreatorType>::singleton();
		}
		template<> EnumRegistrar<DataModel::CreatorType> EnumRegistrar<DataModel::CreatorType>::registrar(0);
		template<> TypeRegistrar<DataModel::CreatorType> TypeRegistrar<DataModel::CreatorType>::registrar(0);

		template<>
		const Type& Type::getSingleton<DataModel::Genre>()
		{
			return EnumDesc<DataModel::Genre>::singleton();
		}
		template<> EnumRegistrar<DataModel::Genre> EnumRegistrar<DataModel::Genre>::registrar(0);
		template<> TypeRegistrar<DataModel::Genre> TypeRegistrar<DataModel::Genre>::registrar(0);

		template<>
		const Type& Type::getSingleton<DataModel::GearGenreSetting>()
		{
			return EnumDesc<DataModel::GearGenreSetting>::singleton();
		}
		template<> EnumRegistrar<DataModel::GearGenreSetting> EnumRegistrar<DataModel::GearGenreSetting>::registrar(0);
		template<> TypeRegistrar<DataModel::GearGenreSetting> TypeRegistrar<DataModel::GearGenreSetting>::registrar(0);

		template<>
		const Type& Type::getSingleton<DataModel::GearType>()
		{
			return EnumDesc<DataModel::GearType>::singleton();
		}
		template<> EnumRegistrar<DataModel::GearType> EnumRegistrar<DataModel::GearType>::registrar(0);
		template<> TypeRegistrar<DataModel::GearType> TypeRegistrar<DataModel::GearType>::registrar(0);
	}

	// StringConverter<Enum>::convertToValue link debts (via EnumDesc lookup).
	// convertToString companions added symmetrically (same desc).
	template<> bool StringConverter<DataModel::CreatorType>::convertToValue(const std::string& text, DataModel::CreatorType& value)
	{
		return Reflection::EnumDesc<DataModel::CreatorType>::singleton().convertToValue(text.c_str(), value);
	}
	template<> std::string StringConverter<DataModel::CreatorType>::convertToString(const DataModel::CreatorType& value)
	{
		return Reflection::EnumDesc<DataModel::CreatorType>::singleton().convertToString(value);
	}
	template<> bool StringConverter<DataModel::Genre>::convertToValue(const std::string& text, DataModel::Genre& value)
	{
		return Reflection::EnumDesc<DataModel::Genre>::singleton().convertToValue(text.c_str(), value);
	}
	template<> std::string StringConverter<DataModel::Genre>::convertToString(const DataModel::Genre& value)
	{
		return Reflection::EnumDesc<DataModel::Genre>::singleton().convertToString(value);
	}
	template<> bool StringConverter<DataModel::GearGenreSetting>::convertToValue(const std::string& text, DataModel::GearGenreSetting& value)
	{
		return Reflection::EnumDesc<DataModel::GearGenreSetting>::singleton().convertToValue(text.c_str(), value);
	}
	template<> std::string StringConverter<DataModel::GearGenreSetting>::convertToString(const DataModel::GearGenreSetting& value)
	{
		return Reflection::EnumDesc<DataModel::GearGenreSetting>::singleton().convertToString(value);
	}
	template<> bool StringConverter<DataModel::GearType>::convertToValue(const std::string& text, DataModel::GearType& value)
	{
		return Reflection::EnumDesc<DataModel::GearType>::singleton().convertToValue(text.c_str(), value);
	}
	template<> std::string StringConverter<DataModel::GearType>::convertToString(const DataModel::GearType& value)
	{
		return Reflection::EnumDesc<DataModel::GearType>::singleton().convertToString(value);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// DataModel (ctor 0x2D41E0)
	//////////////////////////////////////////////////////////////////////////////////
	// iOS: internal RBX::registerNewImageAPI() (DataModel.cpp), run once by the ctor under
	// FFlag::ImageRectEnabled. Body not written yet.
	// STUB: PLAYER 0x0064ea00
	// STUB: STUDIO 0x0076b4d0
	static void registerNewImageAPI()
	{
		STUB(0x0064ea00);
	}

	// Defined in Light.cpp (PLAYER 0x0075ed40) and Workspace.cpp (0x007154c0).
	void registerNewLightAPI();
	void registerNetworkStreamingProp();

	// GLOBAL: PLAYER 0x00de4768
	// GLOBAL: STUDIO 0x00f83b38
	static boost::once_flag newImageAPIFlag = BOOST_ONCE_INIT;
	// GLOBAL: PLAYER 0x00de4770
	// GLOBAL: STUDIO 0x00f83b40
	static boost::once_flag newLightAPIFlag = BOOST_ONCE_INIT;
	// GLOBAL: PLAYER 0x00de4778
	// GLOBAL: STUDIO 0x00f83b48
	static boost::once_flag networkStreamingPropFlag = BOOST_ONCE_INIT;

	// GLOBAL: PLAYER 0x00de4c0c
	// GLOBAL: STUDIO 0x00f83fd8
	rbx::atomic<int> DataModel::count;

	// IMetric overrides (2013 vftable 0x00b2ffa4). Bodies not written yet.
	// STUB: PLAYER 0x00642af0
	// STUB: STUDIO 0x00760370
	double DataModel::getMetricValue(const std::string& name) const
	{
		STUB(0x00642af0);
		return 0.0;
	}

	// STUB: PLAYER 0x0064d850
	// STUB: STUDIO 0x0076a340
	DataModel::~DataModel()
	{
		count--;
		if (placeID != 0)
			RbxDbgInfo::RemovePlace(placeID);
	}

	// STUB: PLAYER 0x0064ec50
	// STUB: STUDIO 0x0076b720
	DataModel::DataModel(Verb* lockVerb, DataModel* parentDM)
		:DescribedNonCreatable<DataModel, ServiceProvider, sDataModel>()
		,VerbContainer(NULL)
		,DataModelArbiter()
		,lockVerb(lockVerb)
		,workspace(Creatable<Instance>::create<Workspace>())
		,guiRoot(Creatable<Instance>::create<GuiRoot>())
		,guiRoot2(Creatable<Instance>::create<GuiRoot>())
		,forceArrowCursor(true)
		,drawId(0)
		,networkMetric(NULL)
		,dirtyFlag(false)
		,isInitialized(false)
		,isContentLoaded(false)
		,numPartInstances(0)
		,numInstances(0)
		,shutdownRequestedCount(0)
		,physicsStepID(0)
		,isGameLoaded(false)
		,game(NULL)
		,networkStatsWindowsOn(false)
		,write_requested(0)
		,read_requested(0)
		,writeRequestingThread(0)
		,mouseOverGui(false)
		,remoteBuildMode(false)
		,placeID(0)
		,placeVersion(0)
		,creatorID(0)
		,creatorType(CREATOR_USER)
		,parentDataModel(parentDM)
	{
		if (FFlag::NewLightAPI)
		{
			FASTLOG(FLog::CloseDataModel, "Register new light API");
			boost::call_once(&registerNewLightAPI, newLightAPIFlag);
		}
		if (FFlag::ImageRectEnabled)
		{
			FASTLOG(FLog::CloseDataModel, "Register new image API");
			boost::call_once(&registerNewImageAPI, newImageAPIFlag);
		}
		FASTLOG(FLog::CloseDataModel, "Register network streaming prop");
		boost::call_once(&registerNetworkStreamingProp, networkStreamingPropFlag);

		addHackFlag(0x800);
		count++;
		setName("Game");
		// The parent lock: FWInstance's first byte (fw+0x18), through the out-of-line
		// Instance::fw() 0x006beaa0 and FWValue<bool>::set 0x0052c000.
		fw()->b21.set(true, fwRef());
		guiBuilder.Initialize(this);
	}

	/* mac: 0x2D3742 — creates via Creatable<Instance>::create<DataModel, Verb*, DataModel*> */
	// STUB: PLAYER 0x0064fc30
	// STUB: STUDIO 0x0076c6e0
	shared_ptr<Instance> DataModel::createDataModel(bool flag, Verb* lockVerb, DataModel* parentDM)
	{
		FASTLOG1(FLog::CloseDataModel, "Create DataModel - heartbeat(%d)", flag);
		shared_ptr<DataModel> dm = Creatable<Instance>::create<DataModel>(lockVerb, parentDM);
		doDataModelSetup(dm, flag);
		return dm;
	}

	// STUB: PLAYER 0x00650660
	// STUB: STUDIO 0x0076d120
	std::string DataModel::getMetric(const std::string& name) const
	{
		STUB(0x00650660);
		return std::string();
	}

	/* mac: 0x2D83C4 — asserts "runService" (DataModel.cpp:1485), reads gameTime at runService+116 */
	double DataModel::getGameTime() const
	{
		RBXASSERT(runService.get() != NULL);		// "runService"
		return runService->gameTime();
	}

	/* mac: 0x2D1636 - ServiceProvider::find<Lighting>(this) */
	Instance* DataModel::getLightingDeprecated() const
	{
		return ServiceProvider::find<Lighting>(this);
	}

	std::string DataModel::arbiterName()
	{
		return "DataModel";		// TODO(decompile): exact string from Refl_DataModel.cpp.txt
	}

	/* mac: 0x2D2E70 — parentDM null ? this DataModelArbiter : parentDM's DataModelArbiter.
	   NOTE: 2016 just returns this (parentDM chain removed with the 2-arg ctor). */
	TaskScheduler::Arbiter* DataModel::getSyncronizationArbiter()
	{
		DataModel* dm = parentDataModel ? parentDataModel : this;
		return static_cast<Arbiter*>(static_cast<DataModelArbiter*>(dm));
	}

	//////////////////////////////////////////////////////////////////////////////////
	// IDataState (Verb.h interface: setDirty(bool)=0, isDirty()=0)
	//////////////////////////////////////////////////////////////////////////////////
	void DataModel::setDirty(bool dirty)
	{
		dirtyFlag = dirty;
	}

	bool DataModel::isDirty() const
	{
		return dirtyFlag;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Tree events + arbiter queries
	// mac: Evidence: onDescendantRemoving 0x2E31F6 (assert write_requested==1 at
	//   DataModel.cpp:3162; numInstances--/numPartInstances--; Super call;
	//   setDirty(true) via vtable+140); onDescendantAdded 0x2E3120
	//   (batches 622-624: unconditional numInstances++ at +0xC18, Part-gated
	//   numPartInstances++ at +0xC14 via descriptor isA, DIRECT (non-virtual)
	//   ServiceProvider::onDescendantAdded call, setDirty(true) via
	//   vtable+0x8C=140); getNumPlayers 0x2E32CC (find Players, else 0).
	// s521 CORRECTION (retires the s414 DELTA): the binary takes Instance*,
	//   a true virtual override -- the shared_ptr shape was 2016-carry.
	// LOUD DELTA (residual): the binary asserts write_requested==1 twice,
	//   gated on [this+0xC78]==1 (identity unmapped), before the increments;
	//   NOT ported (no unconditional-assert precedent fits the gate) --
	//   queued with the +0xC78 audit.
	//////////////////////////////////////////////////////////////////////////////////
	// STUB: PLAYER 0x00645d50
	// STUB: STUDIO 0x00762d10
	void DataModel::onDescendantAdded(Instance* descendant)
	{
		numInstances++;
		if (Instance::fastDynamicCast<PartInstance>(descendant))
			numPartInstances++;
		ServiceProvider::onDescendantAdded(descendant);
		this->setDirty(true);
	}

	// STUB: PLAYER 0x00645e00
	// STUB: STUDIO 0x00762dc0
	void DataModel::onDescendantRemoving(const shared_ptr<Instance>& descendant)
	{
		RBXASSERT(write_requested==1);
		numInstances--;
		if (Instance::fastDynamicCast<PartInstance>(descendant.get()))
			numPartInstances--;
		ServiceProvider::onDescendantRemoving(descendant);
		this->setDirty(true);
	}

	// Bodies not written yet (2013 vftable slots +0x58 / +0x64).
	// STUB: PLAYER 0x0062ee00
	// STUB: STUDIO 0x0074eb20
	void DataModel::onChildAdded(Instance* child)
	{
		STUB(0x0062ee00);
		ServiceProvider::onChildAdded(child);
	}

	// STUB: PLAYER 0x0063e180
	// STUB: STUDIO 0x0075c030
	void DataModel::onChildChanged(Instance* instance, const PropertyChanged& event)
	{
		RBXASSERT(write_requested==1);
		ServiceProvider::onChildChanged(instance, event);
		this->setDirty(true);
		itemChangedSignal(shared_from(instance), &event.getDescriptor());
	}

	// STUB: PLAYER 0x0064fce0
	// STUB: STUDIO 0x0076c790
	GuiResponse DataModel::processAccelerators(const GuiEvent& event)
	{
		STUB(0x0064fce0);
		return GuiResponse();
	}

	// STUB: PLAYER 0x0062f010
	// STUB: STUDIO 0x0074eda0
	GuiResponse DataModel::processCameraCommands(const GuiEvent& event)
	{
		STUB(0x0062f010);
		return GuiResponse();
	}

	// STUB: PLAYER 0x00645770
	// STUB: STUDIO 0x00762730
	void DataModel::renderPass2d(Adorn* adorn, IMetric* graphicsMetric)
	{
		STUB(0x00645770);
	}

	// STUB: PLAYER 0x00640970
	// STUB: STUDIO 0x0075e200
	void DataModel::renderMouse(Adorn* adorn)
	{
		STUB(0x00640970);
	}

	// STUB: PLAYER 0x0064ae00
	// STUB: STUDIO 0x007679f0
	bool DataModel::processEvent(const UIEvent& event)
	{
		STUB(0x0064ae00);
		return false;
	}

	int DataModel::getNumPlayers() const
	{
		const Network::Players* players = ServiceProvider::find<Network::Players>(this);
		if (players == NULL)
			return 0;
		return players->getNumPlayers();
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Scalar / string state (member stores; raisePropertyChanged wiring TODO pending
	// the DataModel reflection block decompile)
	//////////////////////////////////////////////////////////////////////////////////

	/* mac: 0x2CDEC2 — 2013 STORES the bool (2016 stubbed this out to a no-op) */
	void DataModel::setRemoteBuildMode(bool remoteBuildMode)
	{
		this->remoteBuildMode = remoteBuildMode;
	}

	bool DataModel::getRemoteBuildMode()
	{
		return remoteBuildMode;
	}

	bool DataModel::getIsPersonalServer() const
	{
		return isPersonalServer;
	}

	void DataModel::setIsPersonalServer(bool value)
	{
		isPersonalServer = value;
	}

	bool DataModel::getIsGameLoaded()
	{
		return isGameLoaded;
	}

	/* 2016 shape (TODO: verify against 0x2E2906-adjacent gameLoaded logic) */
	void DataModel::setIsGameLoaded(bool value)
	{
		isGameLoaded = value;
		if (isGameLoaded)
			gameLoadedSignal();
	}

	

	void DataModel::setJobsExtendedStatsWindow(double seconds)
	{
		TaskScheduler::singleton().setJobsExtendedStatsWindow(seconds);
	}

	static void appendJobInfo(DataModel* dataModel, shared_ptr<const TaskScheduler::Job> job, Reflection::ValueArray* result)
	{
		if (job->getArbiter().get()!=dataModel)
			return;

		shared_ptr<Reflection::ValueArray> info(rbx::make_shared<Reflection::ValueArray>());
		info->push_back(job->name);
		info->push_back(job->averageDutyCycle());
		info->push_back(job->averageStepsPerSecond());
		info->push_back(job->averageStepTime());
		info->push_back(job->averageError());
		info->push_back(job->isRunning());

		result->push_back(shared_ptr<const RBX::Reflection::ValueArray>(info));
	}

	/* mac: 0x2CE8BE — TODO(decompile): verify body */
	// STUB: PLAYER 0x0064b7f0
	// STUB: STUDIO 0x007683e0
	shared_ptr<const Reflection::ValueArray> DataModel::getJobsInfo()
	{
		shared_ptr<Reflection::ValueArray> result(rbx::make_shared<Reflection::ValueArray>());
		std::vector<boost::shared_ptr<const TaskScheduler::Job> > jobs;
		TaskScheduler::singleton().getJobsInfo(jobs);
		std::for_each(jobs.begin(), jobs.end(), boost::bind(appendJobInfo, this, _1, result.get()));
		return result;
	}

	static void appendJobExtendedStats(DataModel* dataModel, shared_ptr<const TaskScheduler::Job> job, Reflection::ValueArray* result)
	{
		if (job->getArbiter().get()!=dataModel)
			return;

		shared_ptr<Reflection::ValueArray> info(rbx::make_shared<Reflection::ValueArray>());

		WindowAverageDutyCycle<>::Stats stats = job->getDutyCycleWindow().getStats();
		info->push_back(job->name);

		info->push_back(stats.time.average);
		info->push_back(stats.time.variance);
		info->push_back((double)stats.time.samples);
		info->push_back(stats.interval.average.seconds());
		info->push_back(stats.interval.variance.seconds());
		info->push_back((double)stats.interval.samples);
		info->push_back(stats.dutyfraction);

		result->push_back(shared_ptr<const RBX::Reflection::ValueArray>(info));
	}

	/* mac: 0x2D0548 — TODO(decompile): verify body */
	// STUB: PLAYER 0x0064c180
	// STUB: STUDIO 0x00768d70
	shared_ptr<const Reflection::ValueArray> DataModel::getJobsExtendedStats()
	{
		// Returns some nice diagnostic information
		shared_ptr<Reflection::ValueArray> result(rbx::make_shared<Reflection::ValueArray>());
		{
			shared_ptr<Reflection::ValueArray> info(rbx::make_shared<Reflection::ValueArray>());
			info->push_back(std::string("name"));
			info->push_back(std::string("time.average"));
			info->push_back(std::string("time.variance"));
			info->push_back(std::string("time.samples"));
			info->push_back(std::string("interval.average"));
			info->push_back(std::string("interval.variance"));
			info->push_back(std::string("interval.samples"));
			info->push_back(std::string("dutyfraction"));

			result->push_back(shared_ptr<const RBX::Reflection::ValueArray>(info));
		}
		std::vector<boost::shared_ptr<const TaskScheduler::Job> > jobs;
		TaskScheduler::singleton().getJobsInfo(jobs);
		std::for_each(jobs.begin(), jobs.end(), boost::bind(appendJobExtendedStats, this, _1, result.get()));
		return result;
	}

	static void getJobTimePeakFractionFunc(DataModel* dataModel, shared_ptr<const TaskScheduler::Job> job, std::string& jobname, double greaterThan, double* result)
	{
		if (job->getArbiter().get()!=dataModel)
			return;

		const WindowAverageDutyCycle<>& data = job->getDutyCycleWindow();

		*result = data.countTimesGreaterThan(Time::Interval(greaterThan));
		*result /= data.timesamples();
	}

	/* mac: 0x2D1056 — TODO(decompile): verify body */
	double DataModel::getJobTimePeakFraction(std::string jobname, double greaterThan)
	{
		double result = -1.0;
		std::vector<boost::shared_ptr<const TaskScheduler::Job> > jobs;
		TaskScheduler::singleton().getJobsByName(jobname, jobs);
		std::for_each(jobs.begin(), jobs.end(), boost::bind(getJobTimePeakFractionFunc, this, _1, jobname, greaterThan, &result));
		return result;
	}

	static void getJobIntervalPeakFractionFunc(DataModel* dataModel, shared_ptr<const TaskScheduler::Job> job, std::string& jobname, double greaterThan, double* result)
	{
		if (job->getArbiter().get()!=dataModel)
			return;

		const WindowAverageDutyCycle<>& data = job->getDutyCycleWindow();

		*result = data.countIntervalsGreaterThan(Time::Interval(greaterThan));
		*result /= data.intervalsamples();
	}

	/* mac: 0x2D123A — TODO(decompile): verify body */
	double DataModel::getJobIntervalPeakFraction(std::string jobname, double greaterThan)
	{
		double result = -1.0;
		std::vector<boost::shared_ptr<const TaskScheduler::Job> > jobs;
		TaskScheduler::singleton().getJobsByName(jobname, jobs);
		std::for_each(jobs.begin(), jobs.end(), boost::bind(getJobIntervalPeakFractionFunc, this, _1, jobname, greaterThan, &result));
		return result;
	}

	/* mac: 0x2CDEE4 */
	void DataModel::setServerSaveUrl(std::string url)
	{
		serverSaveUrl = url;
	}

	/* mac: 0x2D1664 */
	void DataModel::setScreenshotSEOInfo(std::string info)
	{
		screenshotSEOInfo = info;
	}

	std::string DataModel::getScreenshotSEOInfo()
	{
		return screenshotSEOInfo;
	}

	// 2016 shape (2013 EA 0x2E43D8 pending extraction; no setter in tree).
	std::string DataModel::getJobId() const
	{
		return jobId;
	}

	/* mac: 0x2D1686 */
	void DataModel::setVideoSEOInfo(std::string info)
	{
		videoSEOInfo = info;
	}

	/* mac: 0x2D1442 — binary FastLogs "Setting place version %u" then stores + raisePropertyChanged */
	// STUB: PLAYER 0x0062d910
	// STUB: STUDIO 0x0074db80
	void DataModel::setPlaceVersion(int placeVersion)
	{
		if (this->placeVersion != placeVersion)
		{
			this->placeVersion = placeVersion;
			raisePropertyChanged(prop_placeVersion);
		}
	}

	/* mac: 0x2D14A4 — FastLog + AddPlace + conditional store/raise + SC wiring. */
	// FUNCTION: PLAYER 0x0063e0b0
	// FUNCTION: STUDIO 0x0075bf60
	void DataModel::setPlaceID(int placeID, bool robloxPlace)
	{
		FASTLOG1(FLog::CloseDataModel, "Setting place ID %u", placeID);
		RbxDbgInfo::AddPlace(placeID);
		if (this->placeID != placeID)
		{
			this->placeID = placeID;
			raisePropertyChanged(prop_placeId);
		}
		if (ScriptContext* sc = ServiceProvider::create<ScriptContext>(this))
			sc->setRobloxPlace(robloxPlace);
	}

	/* mac: 0x2E425A */
	int DataModel::getPlaceID() const
	{
		return placeID;
	}

	/* mac: 0x2D153C */
	void DataModel::setCreatorID(int creatorID, CreatorType creatorType)
	{
		if (this->creatorID != creatorID)
		{
			this->creatorID = creatorID;
			raisePropertyChanged(prop_creatorId);
		}
		if (this->creatorType != creatorType)
		{
			this->creatorType = creatorType;
			raisePropertyChanged(prop_creatorType);
		}
	}

	/* mac: 0x2D159E */
	void DataModel::setGenre(Genre genre)
	{
		if (this->genre != genre)
		{
			this->genre = genre;
			raisePropertyChanged(prop_genre);
		}
	}

	/* mac: 0x2D15DA — binary fires allowedGearTypeChanged signal at +2784 on allowedGearTypes change */
	void DataModel::setGear(GearGenreSetting gearGenreSetting, int allowedGearTypes)
	{
		if (this->gearGenreSetting != gearGenreSetting)
		{
			this->gearGenreSetting = gearGenreSetting;
			raisePropertyChanged(prop_gearGenreSetting);
		}
		if (this->allowedGearTypes != static_cast<unsigned>(allowedGearTypes))
		{
			this->allowedGearTypes = static_cast<unsigned>(allowedGearTypes);
			allowedGearTypeChanged();
		}
	}

	/* mac: 0x2D164A — binary: bittest(dword at +3484, gearType) */
	bool DataModel::isGearTypeAllowed(GearType gearType)
	{
		return (allowedGearTypes & (1u << static_cast<unsigned>(gearType))) != 0;
	}

	/* mac: 0x2D152C */
	void DataModel::activateExperimentalFeatures()
	{
		experimentalFeatures = true;
	}

	/* mac: 0x2D01B2 — returns 0 ALWAYS (2016 returns true on _WIN32 — divergence) */
	bool DataModel::canSaveLocal() const
	{
		return false;
	}

	/* mac: 0x2D04F2 */
	void DataModel::setUiMessage(std::string message)
	{
		uiMessage = message;
	}

	/* mac: 0x2D0514 */
	void DataModel::clearUiMessage()
	{
		uiMessage.clear();
	}

	/* 2016 setUiMessageBrickCount sets uiMessage = "[[[progress]]]"; batch617
	   (getUpdatedMessageBoxText 0x2D85E2 compares uiMessage against the
	   same literal) verifies the 2013 string is identical. */
	// STUB: PLAYER 0x0062ee30
	// STUB: STUDIO 0x0074ebc0
	void DataModel::setUiMessageBrickCount()
	{
		uiMessage = "[[[progress]]]";
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Verbs (VerbContainer::getVerb + direct doIt — NO whitelist/doItWithChecks layer)
	//////////////////////////////////////////////////////////////////////////////////

	/* mac: 0x2CFF76 — getVerb("Exit"), throw "Couldn't find Exit Verb", vtable slot 7 doIt.
	   NOTE: 2016 uses getWhitelistVerb("E","xi","t") + doItWithChecks (obfuscation +
	   security layer added later). */
	// FUNCTION: PLAYER 0x0062d720
	// FUNCTION: STUDIO 0x0074d990
	void DataModel::close()
	{
		Verb* verb = getVerb("Exit");
		if (verb==NULL)
			throw std::runtime_error("Couldn't find Exit Verb");
		verb->doIt(NULL);
	}

	/* mac: 0x2D00E4 — getVerb("TogglePlayMode"), isChecked gate, doIt.
	   NOTE: 2016 uses getWhitelistVerb("Toggle","Play","Mode"). */
	// FUNCTION: PLAYER 0x0062da20
	// FUNCTION: STUDIO 0x0074dc90
	void DataModel::toggleToolsOff()
	{
		Verb* verb = getVerb("TogglePlayMode");
		// Only toggle if tools are enabled, so user can't use it to turn tools back on
		if (verb!=NULL && verb->isChecked())
			verb->doIt(NULL);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Contents (clearContents 0x2CF1D0)
	//////////////////////////////////////////////////////////////////////////////////

	// One 0x2CF1D0 "find + log + removeAllChildren" block. The binary logs the
	// service subtree size (1 for self + countDescendantsOfType over children =
	// countDescendantsOfType<Instance> on the service) then removes all children.
	// Exact log literals are TODO (address-evidenced VA names in comments).
	template<class ServiceClass>
	static void clearFoundService(DataModel* dataModel, const char* logFormat)
	{
		if (ServiceClass* service = dataModel->find<ServiceClass>())
		{
			if (FLog::CloseDataModel)
			{
				int count = service->template countDescendantsOfType<Instance>();
				FLog::FastLog(FLog::CloseDataModel, logFormat,
					reinterpret_cast<const void*>(static_cast<size_t>(count)));
			}
			service->removeAllChildren();
		}
	}

	// Same for a cached member service (starterPack +2952, starterGui +2960,
	// coreGuiService +2920).
	static void clearMemberService(Instance* service, const char* logFormat)
	{
		if (service == NULL)
			return;
		if (FLog::CloseDataModel)
		{
			int count = service->countDescendantsOfType<Instance>();
			FLog::FastLog(FLog::CloseDataModel, logFormat,
				reinterpret_cast<const void*>(static_cast<size_t>(count)));
		}
		service->removeAllChildren();
	}

	/* mac: 0x2CF1D0 — evidence: F:\2013Decompile\exports\decomp\Refl_DataModel.cpp.txt
	   (C:\tmp\clearcontents.txt work extract).
	   Shape: ScriptContext flag (byte +247 = 1, identity TODO) -> workspace block
	   (terrain FW b21=0 via FWValue<bool>::set; setMouseCommand(null);
	   ContactManager::fastClear + World stats logs; "Clearing workspace" counts
	   (Instances/Parts/BaseScripts); removeAllChildren; setCurrentCamera(null)) ->
	   cached members starterPack/starterGui/coreGuiService (log + clear) ->
	   find<> blocks Teams (log + clear), SpawnerService (ClearContents),
	   ChangeHistoryService (waypoint count log + clearWaypoints), Lighting,
	   JointsService, TestService, ServerScriptService, ReplicatedStorage,
	   ServerStorage (each log + clear) -> ScriptContext::closeStates(reset) +
	   trailing global emit (TODO below).
	   Log-literal TODOs: exact 2013 format strings unknown (VA-evidenced names:
	   aClearingWorldD ×3, aClearingWorksp, aClearingStarte ×2, aClearingCoregu,
	   aClearingTeamsD, aClearingChange, aClearingLighti, aClearingJoints,
	   aClearingTestse, aClearingServer, aClearingReplic, aClearingServer_0,
	   aClosingScriptC). Counts/sequence are binary-exact. */
	// STUB: PLAYER 0x006458d0
	// STUB: STUDIO 0x00762890
	void DataModel::clearContents(bool resettingSimulation)
	{
		ScriptContext* scriptContext = find<ScriptContext>();
		if (scriptContext != NULL)
			scriptContext->contentClearing = true;

		if (workspace)
		{
			if (Instance* terrain = workspace->getTerrain())
			{
				// Binary: FWValue<bool>::set(fw+21, false, Terrain+68) — dict
				// write-through; the holder dict is null in bring-up, so the
				// observable effect is the plain store (b21 = 0).
				if (terrain->fw())
					terrain->fw()->b21 = 0;
			}
			workspace->setMouseCommand(shared_ptr<MouseCommand>());
			// TODO(decompile): ContactManager::fastClear + World stat logs
			// (getNumBodies/getNumPoints/getNumConstraints/getNumHashNodes/
			// getMaxBucketSize — v8world, not in tree).
			if (FLog::CloseDataModel)
			{
				// aClearingWorksp: (instances, parts, scripts).
				int instances = workspace->countDescendantsOfType<Instance>();
				int parts = workspace->countDescendantsOfType<PartInstance>();
				int scripts = workspace->countDescendantsOfType<BaseScript>();
				FLog::FastLog(FLog::CloseDataModel, "Clearing workspace",
					reinterpret_cast<const void*>(static_cast<size_t>(instances)),
					reinterpret_cast<const void*>(static_cast<size_t>(parts)),
					reinterpret_cast<const void*>(static_cast<size_t>(scripts)));
			}
			workspace->removeAllChildren();
			workspace->setCurrentCamera(NULL);
		}

		// aClearingStarte ×2, aClearingCoregu.
		clearMemberService(starterPack.get(), "Clearing starter pack");
		clearMemberService(starterGui.get(), "Clearing starter gui");
		clearMemberService(coreGuiService.get(), "Clearing core gui");

		// aClearingTeamsD.
		clearFoundService<Teams>(this, "Clearing teams");

		if (SpawnerService* spawner = find<SpawnerService>())
			spawner->ClearContents();

		if (ChangeHistoryService* history = find<ChangeHistoryService>())
		{
			// aClearingChange: waypoint-list length (list at +29/+116 — layout
			// TODO; count available once the member lands).
			if (FLog::CloseDataModel)
				FLog::FastLog(FLog::CloseDataModel, "Clearing change history", 0);
			history->clearWaypoints();
		}

		// aClearingLighti, aClearingJoints, aClearingTestse, aClearingServer,
		// aClearingReplic, aClearingServer_0.
		clearFoundService<Lighting>(this, "Clearing lighting");
		clearFoundService<JointsService>(this, "Clearing joints");
		clearFoundService<TestService>(this, "Clearing test service");
		clearFoundService<ServerScriptService>(this, "Clearing server scripts");
		clearFoundService<ReplicatedStorage>(this, "Clearing replicated storage");
		clearFoundService<ServerStorage>(this, "Clearing server storage");

		if (scriptContext != NULL)
		{
			// aClosingScriptC.
			if (FLog::CloseDataModel)
				FLog::FastLog(FLog::CloseDataModel, "Closing script context", 0);
			scriptContext->closeStates(resettingSimulation);
			// TODO(decompile): trailing global emit —
			// (*(void(__cdecl**)(_DWORD*,int,char*))(*dword_FA65C4 + 12))(
			//   dword_FA65C4, (int)scriptContext + 36, &localFalse).
			// Target global + callee (signal disconnect/fire on ScriptContext+36?)
			// unidentified; body pending.
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// HTTP (EAs from Refl_DataModel.cpp.txt; structure mirrors 2016, 2013 asserts kept)
	//////////////////////////////////////////////////////////////////////////////////

	/* mac: 0x2D735A — matches 2016 HttpHelper exactly */
	void DataModel::HttpHelper(std::string* response, std::exception* exception, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction)
	{
		if (response)
			resumeFunction(*response);
		else
			errorFunction(exception->what());
	}

	/* mac: 0x2D7778 — Http(url).get(response, false) */
	std::string DataModel::doHttpGet(const std::string& url)
	{
		std::string response;
		RBX::Http(url).get(response, false);
		return response;
	}

	/* mac: 0x2D7470 — Http(url).get(bind(&HttpHelper, resume, error)) */
	void DataModel::doHttpGet(const std::string& url, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction)
	{
		RBX::Http(url).get(boost::bind(&DataModel::HttpHelper, _1, _2, resumeFunction, errorFunction));
	}

	/* mac: 0x2D79B8 — compress flag is size > 0x100 (same value as 2016's > 256);
	   2013 has NO contentType param (2-string form) */
	std::string DataModel::doHttpPost(const std::string& url, const std::string& data)
	{
		std::string response;
		std::istringstream stream(data);
		RBX::Http(url).post(stream, data.size() > 0x100, response, false);
		return response;
	}

	/* mac: 0x2D7CCE — Http(url).post(data, size > 0x100, bind(&HttpHelper), false) */
	void DataModel::doHttpPost(const std::string& url, const std::string& data, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction)
	{
		RBX::Http(url).post(data, data.size() > 0x100,
			boost::bind(&DataModel::HttpHelper, _1, _2, resumeFunction, errorFunction), false);
	}

	/* mac: 0x2CE20E — asserts "isInitialized" (DataModel.cpp:1270).
	   DIVERGENCE from 2016: async path binds sync doHttpGet into a DETACHED
	   boost::thread via RBX::thread_wrapper (fire-and-forget, "" returned);
	   2016 reuses the Http pool with resume/error callbacks instead. */
	// STUB: PLAYER 0x0064a100
	// STUB: STUDIO 0x00766cf0
	std::string DataModel::httpGet(std::string url, bool synchronous)
	{
		RBXASSERT(isInitialized);
		if (url.size() > 0)
		{
			if (synchronous)
				return doHttpGet(url);
			// TODO(decompile): exact 2013 thread-wrapper name literal
			boost::thread(RBX::thread_wrapper(
				boost::bind(static_cast<std::string(*)(const std::string&)>(&DataModel::doHttpGet), url),
				"rbx_http"));
		}
		return "";
	}

	/* mac: 0x2CDF06 — asserts "isInitialized" (DataModel.cpp:1292); "Empty URL" path */
	// STUB: PLAYER 0x0064a350
	// STUB: STUDIO 0x00766f40
	void DataModel::httpGetAsync(std::string url, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction)
	{
		RBXASSERT(isInitialized);
		if (url.size() == 0)
		{
			errorFunction("Empty URL");
			return;
		}
		doHttpGet(url, resumeFunction, errorFunction);
	}

	/* mac: 0x2CE4F2 — asserts "isInitialized" (DataModel.cpp:1309); detached-thread async.
	   TODO(decompile): verify async tail past the bind (symmetric with httpGet). */
	// STUB: PLAYER 0x0064a5d0
	// STUB: STUDIO 0x007671c0
	std::string DataModel::httpPost(std::string url, std::string data, bool synchronous)
	{
		RBXASSERT(isInitialized);
		if (synchronous)
			return doHttpPost(url, data);
		// TODO(decompile): exact 2013 thread-wrapper name literal
		boost::thread(RBX::thread_wrapper(
			boost::bind(static_cast<std::string(*)(const std::string&, const std::string&)>(&DataModel::doHttpPost), url, data),
			"rbx_http"));
		return "";
	}

	/* mac: 0x2CE086 — shape from mangled name; body follows httpGetAsync pattern.
	   TODO(decompile): verify empty-URL/assert behavior from the 0x2CE086 body. */
	// STUB: PLAYER 0x0064a870
	// STUB: STUDIO 0x00767460
	void DataModel::httpPostAsync(std::string url, std::string data, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction)
	{
		RBXASSERT(isInitialized);
		if (url.size() == 0)
		{
			errorFunction("Empty URL");
			return;
		}
		doHttpPost(url, data, resumeFunction, errorFunction);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// GenericJob + LegacyLock + scoped requests
	// Evidence: LegacyLock ctors 0x2D332C+ / dtor 0x2D3738; scoped_write_request
	//   mac: 0x2E3536/0x2E35D6 + scoped_read_request 0x2E3656/0x2E36D4;
	//   timestamped_safe_queue<function<void(DataModel*)>> push mangle;
	//   LegacyLock FASTLOG strings in the PE ("LegacyLock::Begin type(%d)" etc. —
	//   SHORTER than 2016's, exact 2013 strings used below);
	//   getGenericJob(TaskType) 0x2D6E2C; onlyJobsLeft... 0x2D380A;
	//   currentThreadHasWriteLock() const 0x2E3514.
	// Bodies follow March 2016 (same shapes per mangles) EXCEPT: no
	// scoped_write_transfer (no 2013 mangle), no Impersonator, no cyclic branches.
	// TODO(decompile): verify each body against its EA.
	//////////////////////////////////////////////////////////////////////////////////

	// STUB: PLAYER 0x0063ee60
	DataModel::GenericJob::GenericJob(shared_ptr<DataModel> dataModel, const char* name, TaskType taskType)
		:DataModelJob(name, taskType, false, dataModel, Time::Interval(0))
		,dataModel(dataModel)
	{
	}

	Time::Interval DataModel::GenericJob::sleepTime(const Stats& stats)
	{
		(void)stats;
		return tasks.size() > 0 ? Time::Interval(0) : Time::Interval::max();
	}

	void DataModel::GenericJob::step(Task& task)
	{
		shared_ptr<DataModel> dm(dataModel.lock());
		if (!dm)
			return;

		task(dm.get());
	}

	DataModel::GenericJob::Error DataModel::GenericJob::error(const Stats& stats)
	{
		Error result;

		const int size = tasks.size();
		if (size)
		{
			// Estimate the total error to be headTime * n / 2
			result.error = tasks.head_waittime_sec(stats.timeNow) * (1 + size) / 2.0;

			// These tasks are "urgent" because they are often used
			// with LegacyLock to marshal tasks with the UI or Web Service requests
			result.urgent = size > 0;
		}
		return result;
	}

	TaskScheduler::StepResult DataModel::GenericJob::stepDataModelJob(const Stats& stats)
	{
		(void)stats;
		FASTLOG3(FLog::TaskSchedulerRun, "GenericJob::Step dataModel(%d) type(%d) taskCount(%d)", dataModel.lock().get(), taskType, tasks.size());

		switch (taskType)
		{
		case Write:
		case DataIn:
		case PhysicsIn:
			{
				shared_ptr<DataModel> dm(dataModel.lock());
				if (!dm)
					return TaskScheduler::Done;
				// TODO: clean up, non-scoped write request?
				DataModel::scoped_write_request request(dm.get());

				// need to call processTasks here because of the scoped write request
				processTasks();
			}
			break;
		default:
			{
				processTasks();
			}
			break;
		}
		return TaskScheduler::Stepped;
	}

	void DataModel::GenericJob::processTasks()
	{
		FASTLOG(FLog::DataModelJobs, "Process tasks start");
		Task task;
		int size = tasks.size();
		int count = 0;
		while ((count < size) && (tasks.pop_if_present(task))) // need to keep count because task can add more tasks
		{
			step(task);
			count++;

			// NOTE (2013): no isCyclicExecutive early-continue (2016-only).

			RBX::Time timeNow = RBX::Time::now<RBX::Time::Fast>();
			if (TaskScheduler::priorityMethod != TaskScheduler::FIFO)
				break;
			else if (tasks.head_waittime_sec(timeNow) < 0.2)
			{
				// FIFO mode: if we are able to process incoming tasks in a timely manner, process remaining tasks next frame
				break;
			}
		}
		FASTLOG(FLog::DataModelJobs, "Process tasks finish");
	}

	int DataModel::LegacyLock::mainThreadId = ~0;

	struct DataModel::LegacyLock::Implementation : boost::noncopyable
	{
		// To marshal the task to this thread we use a pair of locks (acquire and release)
		// Since creating mutexes are expensive, we push spent Events into a pool
		// for re-use
		struct Events
		{
			CEvent acquiredLock;
			CEvent releasedLock;
			Events()
				:acquiredLock(false)
				,releasedLock(false)
			{}
		};
		SAFE_STATIC(rbx::safe_queue< shared_ptr<Events> >, eventsPool)

		// The job in this thread that has currently acquired a lock (if any)
		// This thread specific pointer is used to re-entrancy checks
		SAFE_STATIC(rbx::thread_specific_reference<GenericJob>, currentJob)

		DataModel* const dataModel;

		// The job being used for scheduling:
		GenericJob* job;
		// The previous job, if any, being used for scheduling:
		GenericJob* previousJob;
		// The mutexes used to marshal the task across threads:
		shared_ptr<Events> events;

		// NOTE (2013): no writeTransfer member (no scoped_write_transfer mangle).

		static void task(shared_ptr<Events> events)
		{
			RBXPROFILER_SCOPE("Jobs", "LegacyLock");

			// Signal the thread that constructed the Implementation object to proceed
			events->acquiredLock.Set();

			// Wait for Implementation object to go out of scope (task complete)
			events->releasedLock.Wait();

			// Recycle the events pair
			eventsPool().push(events);
		}

		Implementation(DataModel* dataModel, boost::shared_ptr<GenericJob> job)
			:job(job.get())
			,dataModel(dataModel)
		{
			RBXASSERT(job);
			previousJob = currentJob().get();
			// Re-entrancy test. No need to wait if we're recursively locked on this DataModel
			if (this->job != previousJob)
			{
				FASTLOG1(FLog::LegacyLock, "LegacyLock::Begin type(%d)", job->taskType);
				// Set the current job for this thread
				currentJob().reset(this->job);

				// Get/create a pair of events for marshalling
				if (!eventsPool().pop_if_present(events))
					events.reset(new Events());

				// Submit the proxy task for scheduling.
				job->tasks.push(boost::bind(&Implementation::task, events));
				TaskScheduler::singleton().reschedule(job);

				// Wait for the task to signal acquiredLock
				events->acquiredLock.Wait();

				// NOTE (2013): no writeTransfer (no scoped_write_transfer mangle).

				FASTLOG3(FLog::LegacyLock, "LegacyLock::Acquired type(%d), job (%s), events (%p)", job->taskType, job->name.c_str(), events.get());
			}
			else
			{
				if (job->taskType == DataModelJob::Write)
					RBXASSERT(dataModel->currentThreadHasWriteLock());

				FASTLOG1(FLog::LegacyLock, "LegacyLock::RecursivelyAcquired type(%d)", job->taskType);
			}
		}

		~Implementation()
		{
			FASTLOG2(FLog::LegacyLock, "LegacyLock::~Implementation type(%d), events (%p)", job->taskType, events.get());

			if (job != previousJob)
			{
				// NOTE (2013): no writeTransfer reset.

				// The task has completed, so undo state
				currentJob().reset(previousJob);
				// Signal the proxy task that we're done
				events->releasedLock.Set();
				FASTLOG1(FLog::LegacyLock, "LegacyLock::Released type(%d)", job->taskType);
			}
			else
			{
				if (job->taskType == DataModelJob::Write)
					RBXASSERT(dataModel->currentThreadHasWriteLock());

				FASTLOG1(FLog::LegacyLock, "LegacyLock::RecursivelyReleased type(%d)", job->taskType);
			}
		}
	};

	DataModel::LegacyLock::LegacyLock(boost::shared_ptr<DataModel> dataModel, DataModelJob::TaskType taskType)
		// TODO: What if an invalid DataModelJob::Function is passed in???
		:implementation(dataModel ? new Implementation(dataModel.get(), dataModel->getGenericJob(taskType)) : 0)
	{
		//assert(GetCurrentThreadId() != mainThreadId);
	}

	DataModel::LegacyLock::LegacyLock(DataModel* dataModel, DataModelJob::TaskType taskType)
		:implementation(dataModel ? new Implementation(dataModel, dataModel->getGenericJob(taskType)) : 0)
	{
		//assert(GetCurrentThreadId() != mainThreadId);
	}

	DataModel::LegacyLock::~LegacyLock()
	{
	}

	shared_ptr<DataModel::GenericJob> DataModel::tryGetGenericJob(DataModelJob::TaskType taskType)
	{
		RBX::mutex::scoped_lock lock(genericJobsLock);
		return genericJobs[taskType];
	}

	/* mac: 0x2D6E2C — TODO(decompile): verify body */
	// STUB: PLAYER 0x00630d70
	// STUB: STUDIO 0x00750a00
	shared_ptr<DataModel::GenericJob> DataModel::getGenericJob(DataModelJob::TaskType taskType)
	{
		shared_ptr<DataModel::GenericJob> job = tryGetGenericJob(taskType);
		if (!job)
		{
			// This can also happen if the job is requested after the data model is closed.
			throw std::runtime_error("Can't submit requested task type");
		}
		return job;
	}

	void DataModel::submitTask(Task task, DataModelJob::TaskType taskType)
	{
		if (shared_ptr<GenericJob> job = tryGetGenericJob(taskType))
		{
			job->tasks.push(task);
			TaskScheduler::singleton().reschedule(job);
		}
	}

	/* mac: 0x2D380A — TODO(decompile): verify body */
	// STUB: PLAYER 0x0063a120
	// STUB: STUDIO 0x00758d40
	bool DataModel::onlyJobsLeftForThisArbiterAreGenericJobs()
	{
		std::vector<boost::shared_ptr<const TaskScheduler::Job> > jobs;
		TaskScheduler::singleton().getJobsInfo(jobs);
		std::vector<boost::shared_ptr<const TaskScheduler::Job> >::iterator itr =
			jobs.begin();

		while (itr != jobs.end()) {
			if ((*itr)->getArbiter().get() == this) {
				bool anyMatch = false;
				for (int i = 0; i < DataModelJob::TaskTypeMax && !anyMatch; ++i) {
					if (genericJobs[i] == *itr) {
						anyMatch = true;
					}
				}
				RBXASSERT(anyMatch);
				if (!anyMatch) {
					return false;
				}
			}
			itr++;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////
// Shutdown (closeDataModel 0x2D4060 / doCloseDataModel 0x2D3952)
//////////////////////////////////////////////////////////////////////////////////

bool DataModel::BlockingDataModelShutdown = false;

/* mac: 0x2D4060 — evidence: Refl_DataModel.cpp.txt (aClosedatamodel/aFinishedWaitin).
   (shared_ptr<DataModel>, bool): bool path takes LegacyLock(Write), logs
   "Finished waitin...", runs doCloseDataModel, unlocks; else direct. */
// STUB: PLAYER 0x00649af0
// STUB: STUDIO 0x007666e0
void DataModel::closeDataModel(shared_ptr<DataModel> dataModel, bool useLegacyLock)
{
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "closeDataModel", 0);
	if (dataModel)
	{
		if (useLegacyLock)
		{

LegacyLock lock(dataModel, DataModelJob::Write);  // binary: TaskType 1
			if (FLog::CloseDataModel)
				FLog::FastLog(FLog::CloseDataModel, "Finished waiting", 0);
			doCloseDataModel(dataModel);
		}
		else
		{
			doCloseDataModel(dataModel);
		}
	}
}

// mac: 0x2D3952 FW loop helper: FWValue<bool>::set(fw+21, false, child+68) per direct
// child (dict write-through; null-dict => plain b21 = 0 store).
struct ClearChildFWFlag
{
	void operator()(shared_ptr<Instance> instance) const
	{
		if (instance->fw())
			instance->fw()->b21 = 0;
	}
};

/* mac: 0x2D3952 — evidence: Refl_DataModel.cpp.txt (C:\\tmp\\doclosedm.txt work extract).
   17 log points (VA-evidenced: aDoclosedatamod, aRaisingClose, aRemovingAllPla,
   aClearingConten, aVisitingChildr, aRemovingAllChi, aResettingPlaye,
   aResettingLocal, aResettingWorks, aResettingRunse, aResettingStart ×2,
   aResettingGuiro, aResettingGuiho, aClearingServic, aRemovingGeneri,
   aCloseDatamodel — exact literals TODO).
   Shape: Impersonator(LocalGUI_, binary arg 1) -> ChangeHistory setEnabled(false)
   -> fire +128/+132 void() signals (batch600: BOTH fires are
   signal_with_args<0,void()>::operator() at DM+128 then DM+132 --
   the ServiceProvider closing/closingLate pair, NOT DataModel's own
   shadow (whose neighbor takes a string)) -> assert isInitialized (DataModel.cpp:638), isInitialized=false
   -> Players removeAllChildren -> clearContents(false) -> child FW b21=0 loop ->
   removeAllChildren(dm) -> reset playerHud/localBackpack/workspace/runService/
   starterPack/starterGui/guiRoot/guiRoot2 -> ServiceProvider::clearServices ->
   assert onlyJobsLeft... (DataModel.cpp:683) -> scheduler remove/removeBlocking
   per GenericJobs slot -> done (tss Context restore via Impersonator dtor; the
   binary's explicit thread_specific_ptr reset is redundant with RAII).
   DIVERGENCES/TODOs: blocking gate drops the DebugSettings+112 conjunct
   (settings module not in tree); exact log literals pending. */
// STUB: PLAYER 0x006495b0
// STUB: STUDIO 0x007661a0
void DataModel::doCloseDataModel(shared_ptr<DataModel> dataModel)
{
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "doCloseDataModel", 0);

	Security::Impersonator impersonator(Security::LocalGUI_);

	if (dataModel)
	{
		if (ChangeHistoryService* history = dataModel->find<ChangeHistoryService>())
		{
			history->setEnabled(false);
		}
	}

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Raising close", 0);
	dataModel->ServiceProvider::closingSignal();
	dataModel->ServiceProvider::closingLateSignal();

	RBXASSERT(dataModel->isInitialized);

dataModel->isInitialized = false;

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Removing all players", 0);
	if (dataModel)
	{
		if (Network::Players* players = dataModel->find<Network::Players>())
			players->removeAllChildren();
	}

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Clearing contents", 0);

dataModel->clearContents(false);

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Visiting children", 0);

dataModel->visitChildren(ClearChildFWFlag());

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Removing all children", 0);

dataModel->removeAllChildren();

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting player hud", 0);
dataModel->playerHud.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting local backpack", 0);
	dataModel->localBackpack.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting workspace", 0);
dataModel->workspace.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting run service", 0);
	dataModel->runService.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting starter pack", 0);
	dataModel->starterPack.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting starter gui", 0);
	dataModel->starterGui.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting gui root", 0);
	dataModel->guiRoot.reset();
	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Resetting gui root2", 0);
	dataModel->guiRoot2.reset();

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Clearing services", 0);

dataModel->clearServices();

RBXASSERT(dataModel->onlyJobsLeftForThisArbiterAreGenericJobs());

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Removing generic jobs", 0);
	// Binary gates on (BlockingDataModelShutdown && DebugSettings+112); the
	// settings module is not in the tree yet — BlockingDataModelShutdown alone.
	bool blocking = BlockingDataModelShutdown;
	TaskScheduler& scheduler = TaskScheduler::singleton();
	for (GenericJobs::iterator it = dataModel->genericJobs.begin();
		it != dataModel->genericJobs.end(); ++it)
	{
		if (shared_ptr<GenericJob> job = *it)
		{
			if (blocking)
				scheduler.removeBlocking(job);
			else
				scheduler.remove(job);
		}
		it->reset();
	}

	if (FLog::CloseDataModel)
		FLog::FastLog(FLog::CloseDataModel, "Close datamodel", 0);
}

	//////////////////////////////////////////////////////////////////////////////////
	// Stats (delegates 0x2D16A8/0x2D16D2/0x2D16F4 -> GuiBuilder at +3488)
	//////////////////////////////////////////////////////////////////////////////////

	/* mac: 0x2D16A8 — tail-calls GuiBuilder::addCustomStat(this+3488, name, text). */
	void DataModel::addCustomStat(std::string name, std::string value)
	{
		guiBuilder.addCustomStat(name, value);
	}

	/* mac: 0x2D16D2 — tail-calls GuiBuilder::removeCustomStat(this+3488, name). */
	// FUNCTION: PLAYER 0x0062d840
	// FUNCTION: STUDIO 0x0074dab0
	void DataModel::removeCustomStat(std::string name)
	{
		guiBuilder.removeCustomStat(name);
	}

	/* mac: 0x2D16F4 — tail-calls GuiBuilder::saveCustomStats(this+3488). */
	// FUNCTION: PLAYER 0x0062d890
	// FUNCTION: STUDIO 0x0074db00
	void DataModel::writeStatsSettings()
	{
		guiBuilder.saveCustomStats();
	}

	/* onRunTransition handler (bound in initializeContents 0x2D3336).
	   2016 shape carried (exact 2013 body TODO — needs its EA):
	   STOPPED-after-RUNNING resets the world; RUNNING starts; PAUSED stops. */
	void DataModel::onRunTransition(RunTransition event)
	{
		RBXASSERT(isInitialized);
		if (!isInitialized)
			return;

		/* RUN-STATE MIRROR (loud DELTA): the 2013 DataModel.cpp onRunTransition
		   body is unported (EA queues behind the DM-era), but the binary keeps a
		   writer-side RunState slot at RunService+112. Maintain it here so
		   RSS-era readers see the same value the transition signal carries. */
		if (runService)
			runService->runState = event.newState;

		switch (event.newState)
		{
		case RS_STOPPED:
			if (event.oldState == RS_RUNNING && workspace)
				workspace->reset();
			break;
		case RS_RUNNING:
			if (workspace)
				workspace->start();
			break;
		case RS_PAUSED:
			if (workspace)
				workspace->stop();
			break;
		}
		RBXASSERT(isInitialized);
	}

/* mac: 0x2D2EB2 — creates Write/Read/None GenericJobs + singleton().add, LegacyLock(Write),
	   initializeContents(bool), isInitialized=true (+3005), clears +3237, FastLogs
	   "DataModel created: %p". NO mainThreadId line, NO loading-screen block (2016-only). */
	// STUB: PLAYER 0x0064f7f0
	// STUB: STUDIO 0x0076c2a0
	void DataModel::doDataModelSetup(shared_ptr<DataModel> dataModel, bool startHeartbeat)
	{
		shared_ptr<DataModel::GenericJob> j;
		j = shared_ptr<DataModel::GenericJob>(new DataModel::GenericJob(dataModel, "Write Marshalled", DataModelJob::Write));
		dataModel->genericJobs[DataModelJob::Write] = j;
TaskScheduler::singleton().add(j);
		j = shared_ptr<DataModel::GenericJob>(new DataModel::GenericJob(dataModel, "Read Marshalled", DataModelJob::Read));
		dataModel->genericJobs[DataModelJob::Read] = j;
TaskScheduler::singleton().add(j);
		j = shared_ptr<DataModel::GenericJob>(new DataModel::GenericJob(dataModel, "None Marshalled", DataModelJob::None));
		dataModel->genericJobs[DataModelJob::None] = j;
TaskScheduler::singleton().add(j);

		// TODO: Do we need this lock here? If initializing has no side effects
		//       like starting a task, then it shouldn't be necessary.
		LegacyLock lock(dataModel, DataModelJob::Write);

dataModel->initializeContents(startHeartbeat);
		dataModel->isInitialized = true;
		dataModel->suppressNavKeys = false;

		FASTLOG1(FLog::CloseDataModel, "DataModel created: %p", dataModel.get());
	}

	/* mac: 0x2D3336 — asserts "!isInitialized" (DataModel.cpp:872). Parents workspace +
	   both GuiRoots, zeroes +3109, creates 25 services (6 plain + backpack/HUD +
	   pack/gui/core + RunService + 16 more ending with AssetService), wires the
	   stored members, setDefaultMouseCommand + setVerbParent on the workspace,
	   zeroes +3109 again, starts RunService when startHeartbeat, binds
	   onRunTransition to runService->runTransitionSignal. Full order from the
	   decompile (see header for slots). TODO: +3109 byte identity;
	   onRunTransition body (needs workspace run-state); verify GuiRoot2 role. */
	// STUB: PLAYER 0x0064f210
	// STUB: STUDIO 0x0076bcc0
	void DataModel::initializeContents(bool startHeartbeat)
	{
	RBXASSERT(!isInitialized);

workspace->setAndLockParent(this);
	guiRoot->setAndLockParent(this);
	guiRoot2->setAndLockParent(this);

	// Binary zeroes +3109 here (and again after setVerbParent).
	isPersonalServer = false;

	ServiceProvider::create<ContentProvider>(this);
	ServiceProvider::create<ContentFilter>(this);
	ServiceProvider::create<KeyframeSequenceProvider>(this);
	ServiceProvider::create<GuiService>(this);
	ServiceProvider::create<ChatService>(this);
	ServiceProvider::create<MarketplaceService>(this);

	localBackpack = shared_from<LocalBackpack>(ServiceProvider::create<LocalBackpack>(this));
	playerHud = Creatable<Instance>::create<PlayerHUD>();

	starterPack = shared_from<StarterPackService>(ServiceProvider::create<StarterPackService>(this));
	starterGui = shared_from<StarterGuiService>(ServiceProvider::create<StarterGuiService>(this));
	coreGuiService = shared_from<CoreGuiService>(ServiceProvider::create<CoreGuiService>(this));
	coreGuiService->createRobloxScreenGui();

	runService = shared_from<RunService>(ServiceProvider::create<RunService>(this));
	workspace->setDefaultMouseCommand();
	workspace->setVerbParent(this);

	ServiceProvider::create<Soundscape::SoundService>(this);

	isPersonalServer = false;

	if (startHeartbeat)
		runService->start();

	// mac: 0x2D3336: bind onRunTransition to the RunService transition signal.
	runService->runTransitionSignal.connect(boost::bind(&DataModel::onRunTransition, this, _1));

		ServiceProvider::create<JointsService>(this);
		ServiceProvider::create<CollectionService>(this);
		ServiceProvider::create<PhysicsService>(this);
		ServiceProvider::create<BadgeService>(this);
		ServiceProvider::create<GeometryService>(this);
		ServiceProvider::create<FriendService>(this);
		ServiceProvider::create<RenderHooksService>(this);
		ServiceProvider::create<InsertService>(this);
		ServiceProvider::create<SocialService>(this);
		ServiceProvider::create<GamePassService>(this);
		ServiceProvider::create<DebrisService>(this);
		ServiceProvider::create<ScriptInformationProvider>(this);
		ServiceProvider::create<CookiesService>(this);
		ServiceProvider::create<TeleportService>(this);
		ServiceProvider::create<PersonalServerService>(this);
		ServiceProvider::create<Network::Players>(this);
		ServiceProvider::create<FWService>(this);
		ServiceProvider::create<UserInputService>(this);
		ServiceProvider::create<ContextActionService>(this);
		ServiceProvider::create<ScriptService>(this);
		ServiceProvider::create<AssetService>(this);
	}

	DataModel* DataModel::get(Instance* context)
	{
		return context ? Instance::fastDynamicCast<DataModel>(context->getRootAncestor()) : NULL;
	}
	const DataModel* DataModel::get(const Instance* context)
	{
		return context ? Instance::fastDynamicCast<DataModel>(context->getRootAncestor()) : NULL;
	}

	bool DataModel::currentThreadHasWriteLock() const
	{
		// TODO: Do we need a mutex around this?
		return writeRequestingThread == GetCurrentThreadId();
	}

	bool DataModel::currentThreadHasWriteLock(Instance* context)
	{
		DataModel* dm = get(context);
		return !dm || dm->currentThreadHasWriteLock();
	}

	DataModel::scoped_write_request::scoped_write_request(Instance* context)
		: dataModel(DataModel::get(context))
	{
		if (dataModel)
		{
			RBX::mutex::scoped_lock lock(dataModel->debugLock);

			RBXASSERT(dataModel->write_requested == 0);
			RBXASSERT(dataModel->read_requested == 0);
			RBXASSERT(dataModel->writeRequestingThread == 0);

			dataModel->write_requested = 1;
			dataModel->writeRequestingThread = GetCurrentThreadId();
		}
	}

	DataModel::scoped_write_request::~scoped_write_request()
	{
		if (dataModel)
		{
			RBX::mutex::scoped_lock lock(dataModel->debugLock);

			RBXASSERT(dataModel->write_requested == 1);
			RBXASSERT(dataModel->read_requested == 0);
			RBXASSERT(dataModel->writeRequestingThread == GetCurrentThreadId());

			dataModel->write_requested = 0;
			dataModel->writeRequestingThread = 0;
		}
	}

	// Place this code around tasks that write to a DataModel
	DataModel::scoped_read_request::scoped_read_request(Instance* context)
		: dataModel(DataModel::get(context))
	{
		if (dataModel)
		{
			RBX::mutex::scoped_lock lock(dataModel->debugLock);

			RBXASSERT(dataModel->write_requested == 0);

			dataModel->read_requested++;
		}
	}

	DataModel::scoped_read_request::~scoped_read_request()
	{
		if (dataModel)
		{
			RBX::mutex::scoped_lock lock(dataModel->debugLock);

			RBXASSERT(dataModel->write_requested == 0);
			RBXASSERT(dataModel->read_requested > 0);

			dataModel->read_requested--;
		}
	}

	/* mac: 0x2D257A — matches 2016 exactly. Binary reads Visit+96 string inline;
	   2016 spells it visit->getUploadUrl(). */
	/* mac: 0x2D7FFE — "asset/?id=<n>" on the provider base URL + fetch. */
	// STUB: PLAYER 0x00640280
	// STUB: STUDIO 0x0075db10
	std::auto_ptr<std::istream> DataModel::loadAssetIdIntoStream(int assetID)
	{
		std::string parameters = "asset/?id=" + boost::lexical_cast<std::string>(assetID);
		std::string url = ServiceProvider::create<ContentProvider>(this)->getBaseUrl() + parameters;

		StandardOut::singleton()->printf(RBX::MESSAGE_INFO, "DataModel loading from: %s", url.c_str());

		return ServiceProvider::create<ContentProvider>(this)->getContent(ContentId(url));
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Load / save (0x2CD928/0x2CDA8C/0x2CDBAA/0x2CDDFE + helper 0x2D7FFE)
	// Evidence: LoadSave.txt + LoadCallees.txt decompiles; March 2016
	// DataModel.cpp used as shape template (see session notes — 2013 omits the
	// 2016 additions: RobloxReplicatedStorage/ReplicatedFirst clears,
	// LuaSourceContainer blocking loads, processAfterLoad, isContentLoaded
	// guards, analytics, workspaceLoadedSignal, timed stats).
	//////////////////////////////////////////////////////////////////////////////////

	// FUNCTION: PLAYER 0x00641c50
	// FUNCTION: STUDIO 0x0075f440
	bool DataModel::canSave(const Instance* instance)
	{
		// If the UploadURL is empty, they don't have permissions to save.  Shut. It. Down.
		if (Visit* visit = ServiceProvider::find<Visit>(instance))
			return !visit->getUploadUrl().empty();

		return true;
	}

	/* mac: 0x2CD928 — server-gated world replace: clear terrain + service
	   children, SerializerV2::load, re-join physics. */
	// STUB: PLAYER 0x00640430
	// STUB: STUDIO 0x0075dcc0
	void DataModel::loadWorld(int assetID)
	{
		if (!Network::Players::backendProcessing(this))
		{
			StandardOut::singleton()->printf(MESSAGE_WARNING,
				"Game:LoadWorld should only be called from a server script");
			return;
		}

		std::auto_ptr<std::istream> stream = loadAssetIdIntoStream(assetID);

		if (workspace)
		{
			workspace->clearTerrain();
			workspace->removeAllChildren();
		}

		if (Lighting* lighting = ServiceProvider::find<Lighting>(this))
			lighting->removeAllChildren();
		if (Soundscape::SoundService* service = ServiceProvider::find<Soundscape::SoundService>(this))
			service->removeAllChildren();
		if (ServerStorage* service = ServiceProvider::find<ServerStorage>(this))
			service->removeAllChildren();
		if (ReplicatedStorage* service = ServiceProvider::find<ReplicatedStorage>(this))
			service->removeAllChildren();

		SerializerV2 serializer;
		serializer.load(*stream, this);

		if (workspace)
			workspace->joinAllHack();
	}

	/* mac: 0x2CDA8C — server-gated game replace: clear script/starter children,
	   SerializerV2::load. (No joinAllHack in the binary.) */
	// FUNCTION: PLAYER 0x006405c0
	// FUNCTION: STUDIO 0x0075de50
	void DataModel::loadGame(int assetID)
	{
		if (!Network::Players::backendProcessing(this))
		{
			StandardOut::singleton()->printf(MESSAGE_WARNING,
				"Game:LoadWorld should only be called from a server script");
			return;
		}

		std::auto_ptr<std::istream> stream = loadAssetIdIntoStream(assetID);

		if (ServerScriptService* service = ServiceProvider::find<ServerScriptService>(this))
			service->removeAllChildren();
		if (starterGui)
			starterGui->removeAllChildren();
		if (starterPack)
			starterPack->removeAllChildren();

		SerializerV2 serializer;
		serializer.load(*stream, this);
	}

	/* mac: 0x2CDBAA — content replace: fetch, SerializerV2::load, re-join, zero
	   the three workspace load-stats, ensure terrain. */
	// STUB: PLAYER 0x00640720
	// STUB: STUDIO 0x0075dfb0
	void DataModel::loadContent(ContentId contentId)
	{
		RBXASSERT(isInitialized);  // DataModel.cpp:1406

		StandardOut::singleton()->printf(RBX::MESSAGE_INFO,
			"DataModel Loading %s", contentId.c_str());

		ContentProvider* contentProvider = ServiceProvider::create<ContentProvider>(this);
		std::auto_ptr<std::istream> stream = contentProvider->getContent(contentId);

		SerializerV2 serializer;
		serializer.load(*stream, this);

		if (workspace)
		{
			workspace->joinAllHack();
			workspace->setStatsSyncHttpGetTime(0.0);
			workspace->setStatsXMLLoadTime(0.0);
			workspace->setStatsJoinAllTime(0.0);
			workspace->createTerrain();
		}
	}

	/* sub_E6FEC (0x2CDDFE callee) — serialize + POST to the content URL.
	   2016 keeps this split in ClientShared/DataModelSerialize.cpp; the 2013
	   TU is unknown so it lives here. */
	void DataModel::internalSave(ContentId contentId)
	{
		RBXASSERT(isInitialized);
		shared_ptr<std::stringstream> stream = serializeDataModel();
		std::string response;
		Http(contentId.toString()).post(*stream, true, response);
	}

	/* mac: 0x2CDDFE — gated save + finished signal. */
	void DataModel::save(ContentId contentId)
	{
		if (canSave(this))
			internalSave(contentId);
		saveFinishedSignal();
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Core-script lifecycle (0x2D5790/0x2D617A/0x2E2906/0x2E3AB0)
	// Evidence: CoreScripts.txt decompiles.
	//////////////////////////////////////////////////////////////////////////////////

	boost::function<void(DataModel*)> DataModel::loaderFunc;

	/* mac: 0x2D5790 — fetch the provider base URL, ensure a ScriptContext, run
	   loadfile('<baseUrl>/game/studio.ashx')() as a trusted thread named
	   getStringById(4) ("ashx"). The binary builds the script string
	   char-by-char with hash telemetry into a static (anti-scan obfuscation;
	   plain concatenation here) and brackets execution with "Load
	   CoreScripts" / "Finished Loading CoreScripts" log entries. */
	void DataModel::loadCoreScripts()
	{
		std::string baseUrl;
		if (ContentProvider* provider = ServiceProvider::find<ContentProvider>(this))
			baseUrl = provider->getBaseUrl();

		if (shared_ptr<ScriptContext> ctx = shared_from<ScriptContext>(ServiceProvider::create<ScriptContext>(this)))
		{
			ScriptContext* scriptContext = ctx.get();
			if (scriptContext != NULL)
			{
			std::string script = "loadfile('" + baseUrl + "/game/studio.ashx')()";
			if (Log* log = Log::current())
				log->writeEntry(Log::Information, "Load CoreScripts");
			ProtectedString trusted = ProtectedString::fromTrustedSource(script);
			scriptContext->executeInNewThread(RBX::Security::RobloxGameScript_, trusted, getStringById(StudioASHX));
			if (Log* doneLog = Log::current())
				doneLog->writeEntry(Log::Information, "Finished Loading CoreScripts");
		}
	}
	}

	/* mac: 0x2D617A — build the in-game GUI, then load core scripts. */
	// STUB: PLAYER 0x006400f0
	// FUNCTION: STUDIO 0x0075d980
	void DataModel::startCoreScripts(Adorn* adorn, bool buildInGameGui)
	{
		guiBuilder.buildGui(adorn, workspace.get(), buildInGameGui);
		loadCoreScripts();
	}

	/* mac: 0x2E2906 — first call sets +3108 and fires the loaded signal. */
	void DataModel::gameLoaded()
	{
		// +3108 flag (adjacent to the +3109 unknown3109 bool; the member
		// stand-in is isGameLoaded — exact MSVC slot TODO).
		if (!isGameLoaded)
		{
			isGameLoaded = true;
			gameLoadedSignal();
		}
	}

	/* mac: 0x2E3AB0 — invoke the Studio plugin-loader hook. */
	void DataModel::loadPlugins()
	{
		loaderFunc(this);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Shutdown + upload (0x2D2CBA/0x2D01BA + sub_E6A00)
	// Evidence: PlaceShutdown.txt decompiles. Result codes match the 2016
	// RequestShutdownResult enum (NOT_HANDLED=0, HANDLED=1, LOCAL_SAVE=2,
	// NO_SAVE_NEEDED=3); the header keeps the binary's int return.
	//////////////////////////////////////////////////////////////////////////////////

	// Lua-shutdown master switch (binary gates the hook on a static at
	// +740146 alongside the member; identity TODO — defaults true so a set
	// hook fires, matching 2016 behavior).
	static bool sLuaShutdownEnabled = true;

	/* mac: 0x2D2CBA — shutdown routing: unsavable -> TogglePlayMode verb state;
	   no upload URL -> LOCAL_SAVE; Lua hook (when armed) -> its verdict;
	   else NOT_HANDLED. */
	int DataModel::requestShutdown(bool useLuaShutdownForSave)
	{
		if (canSave(this))
		{
			Visit* visit = ServiceProvider::find<Visit>(this);
			if (visit == NULL || visit->getUploadUrl().empty())
				return 2;  // CLOSE_LOCAL_SAVE
			if (useLuaShutdownForSave && !requestShutdownCallback.empty() && sLuaShutdownEnabled)
			{
				++shutdownRequestedCount;
				return requestShutdownCallback() ? 1 : 0;
			}
			return 0;  // CLOSE_NOT_HANDLED
		}
		Verb* verb = workspace->getVerb("TogglePlayMode");
		if (verb == NULL || !verb->isChecked())
			return 3;  // CLOSE_NO_SAVE_NEEDED
		return 2;  // CLOSE_LOCAL_SAVE
	}

	static void handleAsyncSaveResult(std::string* response, std::exception* exception,
		boost::function<void(bool)> resumeFunction)
	{
		if (exception != NULL || response == NULL)
			resumeFunction(false);
		else
			resumeFunction(true);
	}

	/* sub_E6A00 (0x2D01BA callee) — serialize + async POST, resume on done. */
	void DataModel::internalSaveAsync(ContentId contentId, boost::function<void(bool)> resumeFunction)
	{
		RBXASSERT(isInitialized);
		shared_ptr<std::stringstream> stream = serializeDataModel();
		Http saveRequest(contentId.toString());
		saveRequest.post(stream, true, boost::bind(&handleAsyncSaveResult, _1, _2, resumeFunction));
	}

	/* mac: 0x2D01BA — gated upload: no save rights / null / no Visit /
	   non-http URL -> resume(false); else async save. */
	void DataModel::saveToRoblox(boost::function<void(bool)> resumeFunction, boost::function<void(std::string)> errorFunction)
	{
		(void)errorFunction;
		if (this == NULL || !canSave(this))
		{
			resumeFunction(false);
			return;
		}
		Visit* visit = ServiceProvider::find<Visit>(this);
		if (visit == NULL)
		{
			resumeFunction(false);
			return;
		}
		std::string uploadUrl = visit->getUploadUrl();
		if (uploadUrl.find("http") != 0)
		{
			resumeFunction(false);
			return;
		}
		internalSaveAsync(ContentId(uploadUrl), resumeFunction);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Shutdown + place upload (0x2D038C/0x2D2A36/0x2D25EA + sub_E8856 worker)
	// Evidence: ShutdownSave.txt decompiles.
	//////////////////////////////////////////////////////////////////////////////////

	/* mac: 0x2D038C — Lua-armed shutdown runs a workspace verb: ShutdownClientAndSave
	   when saving, else ShutdownClient. doIt direct with NULL (like close()). */
	// STUB: PLAYER 0x00641c90
	// STUB: STUDIO 0x0075f480
	void DataModel::completeShutdown(bool saveLocal)
	{
		if (shutdownRequestedCount <= 0)
			return;
		Verb* verb;
		if (saveLocal && canSave(this))
			verb = workspace->getVerb("ShutdownClientAndSave");
		else
			verb = workspace->getVerb("ShutdownClient");
		if (verb != NULL)
			verb->doIt(NULL);
	}

	/* mac: 0x2D7276 — per-class instance census for save-error reports. */
	void DataModel::traverseDataModelReporting(shared_ptr<Instance> node)
	{
		std::string className = node->getClassNameStr();
		if (dataModelReportingData.find(className) == dataModelReportingData.end())
			dataModelReportingData[className] = 0;
		dataModelReportingData[className]++;
	}

	/* sub_E8856 (0x2D25EA callee): serialize-with-filter + async POST.
	   2016 ClientShared shape carried (uploadPlace); the response-tuple
	   fill is TODO (binary parses the reply into the 2-tuple — layout TBD). */
	/* sub_E8856 response fan-out: exception -> error, else resume(tuple). */
	void DataModel::uploadPlaceResponse(shared_ptr<Reflection::Tuple> responseTuple,
		std::string* response, std::exception* exception,
		boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction,
		boost::function<void(std::string)> errorFunction)
	{
		if (exception != NULL || response == NULL)
		{
			errorFunction(exception != NULL ? exception->what() : "Error on upload");
			return;
		}
		resumeFunction(responseTuple);
	}

	static void uploadPlaceAsync(DataModel* dataModel, const std::string& uploadUrl,
		Instance::SaveFilter saveFilter,
		boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction,
		boost::function<void(std::string)> errorFunction)
	{
		shared_ptr<Reflection::Tuple> tuple(new Reflection::Tuple(2));
		if (uploadUrl.empty())
		{
			errorFunction("Url is empty");
			return;
		}
		if (!Http::isRobloxSite(uploadUrl.c_str()))
		{
			errorFunction("Url is invalid");
			return;
		}
		try
		{
			shared_ptr<std::stringstream> stream = dataModel->serializeDataModel(saveFilter);
			if (stream->str().empty())
			{
				errorFunction("No data to save");
				return;
			}
			std::string payload = stream->str();
			bool compress = payload.size() > 256;
			Http(uploadUrl).post(payload, compress,
				boost::bind(&DataModel::uploadPlaceResponse,
					tuple, _1, _2, resumeFunction, errorFunction));
		}
		catch (std::exception& e)
		{
			errorFunction(e.what());
		}
	}

	/* mac: 0x2D25EA — build "<placeID>&isAppCreation=true" on
	   "<baseUrl>ide/publish/UploadExistingAsset", then upload. */
	// STUB: PLAYER 0x0063f680
	// STUB: STUDIO 0x0075cf10
	bool DataModel::serverSavePlace(const Instance::SaveFilter saveFilter,
		boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction,
		boost::function<void(std::string)> errorFunction)
	{
		std::string uploadUrl = ServiceProvider::create<ContentProvider>(this)->getBaseUrl()
			+ "ide/publish/UploadExistingAsset";
		std::string query = RBX::format("%d", placeID) + "&isAppCreation=true";
		uploadPlaceAsync(this, uploadUrl + query, saveFilter, resumeFunction, errorFunction);
		return true;
	}

	/* mac: 0x2D2A36 — gated async save with Lua-facing error strings. */
	// STUB: PLAYER 0x0063f980
	// STUB: STUDIO 0x0075d210
	void DataModel::savePlaceAsync(const Instance::SaveFilter saveFilter,
		boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction,
		boost::function<void(std::string)> errorFunction)
	{
		if (placeID <= 0)
		{
			errorFunction("Game:SavePlace placeID is not valid!");
			return;
		}
		if (Network::Players::frontendProcessing(this))
		{
			errorFunction("Game:SavePlace can only be called from a server script, aborting save function");
			return;
		}
		if (Network::Players::backendProcessing(this))
		{
			serverSavePlace(saveFilter, resumeFunction, errorFunction);
			return;
		}
		errorFunction("Game:SavePlace could not determine if client or server");
	}

	/* Serialize helper (2016 ClientShared shape, binary default SAVE_ALL). */
	shared_ptr<std::stringstream> DataModel::serializeDataModel(const Instance::SaveFilter saveFilter)
	{
		shared_ptr<std::stringstream> stream(new std::stringstream());
		SerializerBinary::serialize(*stream, this, 0, saveFilter);
		return stream;
	}

} // namespace RBX

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Network::Players);
}

DECOMP_SIZE_ASSERT(RBX::DataModel::GenericJob, 0x248);
// DataModel is 0xe10 in PLAYER (operator new in 0x0064f760); STUDIO's is 0xe0c (no
// networkStatsWindowsOn, GuiBuilder 0x1c). GuiBuilder is 0x20 (ctor 0x00644f30).
DECOMP_SIZE_ASSERT(RBX::GuiBuilder, 0x20);
DECOMP_SIZE_ASSERT(RBX::DataModel, 0xe10);
