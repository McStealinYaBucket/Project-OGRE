/* V8DataModel/DataModel.h — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     - ctor 0x2D41E0 (F:\2013Decompile\exports\decomp\DataModel_ctor.txt):
       DataModel(Verb* lockVerb, DataModel* parentDM) — TWO args (2016 dropped parentDM);
       __bzero(this,144); ServiceProvider base at +0; VerbContainer at +144;
       DataModelArbiter at +184; Verb* at +2764; signal ptrs at +2772..
       Described<DataModel, sDataModel, NonFactoryProduct<ServiceProvider,...>, Functionality 27, Security::None>
     - 123 unique DataModel:: methods in Refl_DataModel.cpp.txt (F:\2013Decompile\exports\decomp\Refl_DataModel.cpp.txt)
     - Member offsets (Mach-O/Itanium ABI — MSVC sizes differ, ORDER preserved):
       +2984 bool isInitialized = 1 (ctor); +2988 std::string uiMessage (setUiMessage 0x2D04F2,
       clearUiMessage 0x2D0514); +3440 bool remoteBuildMode (0x2CDEC2/0x2CDED4);
       +3444 serverSaveUrl (0x2CDEE4); +3448 screenshotSEOInfo (0x2D1664);
       +3452 videoSEOInfo (0x2D1686); +3456 int placeID (0x2D14A4, 0x2E425A);
       +3460 experimental flag (activateExperimentalFeatures 0x2D152C sets =1);
       +3464 int placeVersion (0x2D1442); +3468 int creatorID / +3472 CreatorType (0x2D153C);
       +3476 Genre (0x2D159E); +3480 GearGenreSetting / +3481.. allowedGearTypes int (0x2D15DA);
       +3484 unsigned allowedGearTypes bitmask (isGearTypeAllowed 0x2D164A bittest);
       +3488 GuiBuilder (addCustomStat 0x2D16A8 / removeCustomStat 0x2D16D2 /
       writeStatsSettings 0x2D16F4 delegate to GuiBuilder at this+3488);
       +3532 DataModel* parentDM (ctor +883, getSyncronizationArbiter 0x2D2E70:
       parentDM null ? this+184 : parentDM+184).
     - Enums from EnumDesc decompiles: CreatorType User(0)/Group(1) (0x2D171A);
       Genre 14 values All..Tutorial (0x2D1934); GearGenreSetting AllGenres(0)/
       MatchingGenreOnly(1) (0x2D1C86); GearType 9 values MeleeWeapons..Transport (0x2D1EA0).
     - canSave 0x2D257A is STATIC bool(const Instance*): find<Visit>, return
       !uploadUrl.empty() (Visit+96). canSaveLocal 0x2D01B2 returns 0 always
       (2016 returns true on Windows — divergence).
     - getSyncronizationArbiter 0x2D2E70 follows parentDM (2016 just returns this).
     - 2013 close 0x2CFF76 uses VerbContainer::getVerb("Exit") + throw
       "Couldn't find Exit Verb" + direct doIt (no whitelist/doItWithChecks).
       toggleToolsOff 0x2D00E4 uses getVerb("TogglePlayMode") + isChecked + doIt.
     - 2013 setRemoteBuildMode STORES the bool (2016 is a no-op stub).
     - setPlaceID 0x2D14A4 is (int placeID, bool robloxPlace) + ScriptContext wiring
       (2016 adds Http::placeID/Analytics — later additions).
   2016-only members (hackFlagSet, CoreGuiService/ContextActionService refs,
   DFFlags, universeId/vipServer, HeapValue wrappers) are NOT in the 2013 binary.
   PORTED since (sessions 26-38, see 01_first_steps.md): loadWorld/loadGame/
    loadContent/save (+asset stream, internalSave/Async, serialize helper),
    loadCoreScripts/startCoreScripts/gameLoaded/loadPlugins(+loaderFunc),
    setPlaceID (+AddPlace), requestShutdown (+hook), saveToRoblox,
    onRunTransition wiring, presence chains, enum converters, MD5/RbxDbgInfo.
    TODO(decompile): full ctor member list (Workspace +2936, GuiRoots +2968/+2976,
   PhysicsInstructions +3008, GenericJobs +3284..), DataModel reflection block
   (prop descriptors for raisePropertyChanged wiring), Visit.h/Workspace.h. */

#pragma once

#include "V8DataModel/DataModelJob.h"
#include "V8Tree/Instance.h"
#include "V8Tree/Verb.h"
#include "V8Tree/Service.h"
#include "Util/IMetric.h"
#include "rbx/RunningAverage.h"
#include "V8DataModel/PhysicsInstructions.h"

#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <string>
#include <map>
#include <vector>

#include "Util/ContentId.h"
#include "V8DataModel/GuiBuilder.h"
#include "Util/RunTransition.h"

class XmlElement;

// Adorn = RBX::Adorn (binary mangle truth: startCoreScripts
// mac: 0x2D617A = PNS_5AdornE,batch791).
#include "Gui/Adorn.h"

namespace RBX {

class Workspace;
class Lighting;
class GuiRoot;
class LocalBackpack;
class PlayerHUD;
class RunService;
class StarterPackService;
class StarterGuiService;
class CoreGuiService;
class Verb;
class DataModel;
class GenericJob;
class InputObject;
class GuiResponse;
class GuiEvent;
class UIEvent;
class Game;
class IMetric;

extern const char* const sDataModel;

/* 2013 bases (RTTI of vftable 0x00b2ffec; ctor 0x0064ec50 vfptr stores 0x0064ecce..0x0064ed03),
   in declaration order: DescribedNonCreatable<..., ServiceProvider, ...> @0x0, VerbContainer
   @0x8c, IDataState @0xa0, IMetric @0xa4 (2016 has it; the 2013 tree lacked it), DataModelArbiter
   @0xa8, Diagnostics::Countable<DataModel> after them; members start at 0xac8. Size 0xe10
   (operator new in 0x0064f760, STUDIO 0x0076c210). Members 0xac8..0xe10 are mapped from the ctor
   and dtor at the end of the class; no function using them is at 100% yet, so they are not
   proven. 2013 disagrees with 2016: no savePlaceThrottle, contextActionService, starterPlayer-,
   userInputService, isShuttingDown, universe/VIP members or HeapValue placeID/creatorID;
   PhysicsInstructions comes before the hack-flag set; adds playerHud, localBackpack, guiRoot2,
   parentDataModel. STUDIO lacks networkStatsWindowsOn (see the member block). */
// SIZE 0xe10
// VTABLE: PLAYER 0x00b2ffec RBX::Debugable
// VTABLE: STUDIO 0x00bf607c RBX::Debugable
class DataModel
	: public DescribedNonCreatable<DataModel, ServiceProvider, sDataModel>
	, public VerbContainer
	, public IDataState
	, public IMetric
	, public DataModelArbiter
	, public Diagnostics::Countable<DataModel>
{
	friend class GuiBuilder;  // buildGui reads guiRoot (+2968) directly, like the binary.
public:
	// Tamper-reporting bitmask (BSS 0xFA6E5C proven in the Oct 2013
	// binary; Lua fork ORs HATE_* bits into it).
	static unsigned int sendStats;

	enum CreatorType
	{
		CREATOR_USER  = 0,
		CREATOR_GROUP = 1
	};

	/* mac: 0x2D1934: 14 values (binary addPair order) */
	enum Genre
	{
		GENRE_ALL			= 0,
		GENRE_TOWN_AND_CITY = 1,
		GENRE_FANTASY		= 2,
		GENRE_SCI_FI		= 3,
		GENRE_NINJA			= 4,
		GENRE_SCARY			= 5,
		GENRE_PIRATE		= 6,
		GENRE_ADVENTURE		= 7,
		GENRE_SPORTS		= 8,
		GENRE_FUNNY			= 9,
		GENRE_WILD_WEST		= 10,
		GENRE_WAR			= 11,
		GENRE_SKATE_PARK	= 12,
		GENRE_TUTORIAL		= 13
	};

	/* mac: 0x2D1EA0: 9 values (binary addPair order) */
	enum GearType
	{
		GEAR_TYPE_MELEE_WEAPONS			= 0,
		GEAR_TYPE_RANGED_WEAPONS		= 1,
		GEAR_TYPE_EXPLOSIVES			= 2,
		GEAR_TYPE_POWER_UPS				= 3,
		GEAR_TYPE_NAVIGATION_ENHANCERS	= 4,
		GEAR_TYPE_MUSICAL_INSTRUMENTS	= 5,
		GEAR_TYPE_SOCIAL_ITEMS			= 6,
		GEAR_TYPE_BUILDING_TOOLS		= 7,
		GEAR_TYPE_PERSONAL_TRANSPORT	= 8
	};

	enum GearGenreSetting
	{
		GEAR_GENRE_ALL		= 0,
		GEAR_GENRE_MATCH	= 1
	};

	/* ctor 0x2D41E0: (Verb* lockVerb, DataModel* parentDM).
	   parentDM = the linked DataModel for studio play-solo views (removed by 2016). */
	DataModel(Verb* lockVerb, DataModel* parentDM);
	~DataModel();

	// TaskScheduler::Arbiter surface
	// NOTE: no DataModel::areExclusive override (no 2013 mangle; Arbiter's covers).
	virtual int getNumPlayers() const;
	virtual std::string arbiterName();

	// Job/type queries
	bool onlyJobsLeftForThisArbiterAreGenericJobs();
	typedef boost::function<void(DataModel*)> Task;
	// 2013 layout (ctor PLAYER 0x0063ee60; `new 0x248` three times in doDataModelSetup
	// 0x0064f7f0): DataModelJob (0x218), dataModel @0x218, tasks @0x220 (0x24 bytes: no
	// 2016 headTimestamp).
	// SIZE 0x248
	class GenericJob : public DataModelJob
	{
	public:
		weak_ptr<DataModel> const dataModel;	// 0x218
		rbx::timestamped_safe_queue<Task> tasks;	// 0x220

		GenericJob(shared_ptr<DataModel> dataModel, const char* name, TaskType taskType);
	private:
		virtual Time::Interval sleepTime(const Stats& stats);
		void step(Task& task);
		virtual Job::Error error(const Stats& stats);
		virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats);
		void processTasks();
	};
	typedef boost::array<shared_ptr<GenericJob>, DataModelJob::TaskTypeMax> GenericJobs;
	shared_ptr<GenericJob> tryGetGenericJob(DataModelJob::TaskType type);
	shared_ptr<GenericJob> getGenericJob(DataModelJob::TaskType type);
	void submitTask(Task task, DataModelJob::TaskType taskType);

	class LegacyLock : boost::noncopyable
	{
		struct Implementation;
		boost::scoped_ptr<Implementation> const implementation;
	public:
		static int mainThreadId;
		// NOTE (2013): no Impersonator (no mangle — 2016-only)
		LegacyLock(shared_ptr<DataModel> dataModel, DataModelJob::TaskType taskType);
		LegacyLock(DataModel* dataModel, DataModelJob::TaskType taskType);
		~LegacyLock();
	};

	// Core lifecycle
	void close();
	// mac: 0x2D4060: (shared_ptr<DataModel>, bool) — bool takes the LegacyLock path.
	static void closeDataModel(shared_ptr<DataModel> dataModel, bool useLegacyLock);
	// mac: 0x2D3952: (shared_ptr<DataModel>).
	static void doCloseDataModel(shared_ptr<DataModel> dataModel);
	// mac: 0x2D3952 shutdown gate: blocking scheduler removal when set (init TODO).
	static bool BlockingDataModelShutdown;
	void completeShutdown(bool saveLocal);
	void clearContents(bool resettingSimulation);
	// mac: 0x2D2EB2: (shared_ptr<DataModel>, bool). The bool is passed through to
	// initializeContents (like 2016's startHeartbeat); there is NO
	// shouldShowLoadingScreen param and no loading-screen block in 2013.
	static void doDataModelSetup(shared_ptr<DataModel> dataModel, bool startHeartbeat);
	void initializeContents(bool startHeartbeat);
	void loadContent(ContentId contentId);
	void loadGame(int assetID);
	void loadWorld(int assetID);
	// mac: 0x2D7FFE: build "asset/?id=<n>" on the ContentProvider base URL,
	// log "DataModel loading from: %s", fetch -> stream.
	std::auto_ptr<std::istream> loadAssetIdIntoStream(int assetID);
	void loadCoreScripts();  // 0x2D5790: run loadfile('<baseUrl>/game/studio.ashx')()
	void loadPlugins();      // 0x2E3AB0: invoke loaderFunc hook
	// mac: 0x2D617A (was misdeclared void() — binary takes Adorn* + bool).
	void startCoreScripts(Adorn* adorn, bool buildInGameGui);
	void gameLoaded();       // 0x2E2906: +3108 flag + loaded signal
	int requestShutdown(bool useLuaShutdownForSave);

	// Plugin-loading hook invoked by loadPlugins (set by Studio; TODO: setter EA).
	static boost::function<void(DataModel*)> loaderFunc;

	// Save / place info
	static bool canSave(const Instance* instance);
	bool canSaveLocal() const;
	void save(ContentId contentId);
	void internalSaveAsync(ContentId contentId, boost::function<void(bool)> resumeFunction);  // sub_E6A00 (0x2D01BA callee)
	// mac: 0x2CDDFE callee (sub_E6FEC): serialize + upload. 2016 keeps it in
	// ClientShared/DataModelSerialize.cpp; same split here.
	void internalSave(ContentId contentId);
	shared_ptr<std::stringstream> serializeDataModel(const Instance::SaveFilter saveFilter = SAVE_ALL);
	void saveToRoblox(boost::function<void(bool)> resumeFunction, boost::function<void(std::string)> errorFunction);
	// DataModel's own virtuals, in 2013 slot order (vftable 0x00b2ffec +0x70..+0x84, 34 slots;
	// iOS names; OverlayDataModel overrides savePlaceAsync/renderPass2d/renderMouse).
	virtual GuiResponse processAccelerators(const GuiEvent& event);		// vtable+0x70 0x0064fce0
	virtual GuiResponse processCameraCommands(const GuiEvent& event);	// vtable+0x74 0x0062f010
	virtual void savePlaceAsync(const Instance::SaveFilter saveFilter, boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction, boost::function<void(std::string)> errorFunction);	// vtable+0x78 0x0063f980
	virtual void renderPass2d(Adorn* adorn, IMetric* graphicsMetric);	// vtable+0x7c 0x00645770
	virtual void renderMouse(Adorn* adorn);								// vtable+0x80 0x00640970
	virtual bool processEvent(const UIEvent& event);					// vtable+0x84 0x0064ae00
	// sub_E8856 response fan-out (0x2D25EA callee family; public for bind).
	static void uploadPlaceResponse(shared_ptr<Reflection::Tuple> responseTuple, std::string* response, std::exception* exception, boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction, boost::function<void(std::string)> errorFunction);
	bool serverSavePlace(const Instance::SaveFilter saveFilter, boost::function<void(shared_ptr<const Reflection::Tuple>)> resumeFunction, boost::function<void(std::string)> errorFunction);
	void serverSave();
	void setServerSaveUrl(std::string url);
	void setCreatorID(int creatorID, CreatorType creatorType);
	int getCreatorID() const { return creatorID; }
	CreatorType getCreatorType() const { return creatorType; }
	void setPlaceID(int placeID, bool robloxPlace);
	int getPlaceID() const;
	void setPlaceVersion(int placeVersion);
	int getPlaceVersion() const { return placeVersion; }
	void setGenre(Genre genre);
	Genre getGenre() const { return genre; }
	void setGear(GearGenreSetting gearGenreSetting, int allowedGearTypes);
	GearGenreSetting getGearGenreSetting() const { return gearGenreSetting; }
	bool isGearTypeAllowed(GearType gearType);

	// Access control / read-write scoping (2016 shapes; scoped ctors/dtors EAs:
	// write 0x2E3536/0x2E35D6, read 0x2E3656/0x2E36D4; LegacyLock ctors 0x2D332C+;
	// NO scoped_write_transfer in 2013 (no mangle — 2016-only))
	class scoped_write_request
	{
		DataModel* const dataModel;
	public:
		// Place this code around tasks that write to a DataModel
		scoped_write_request(Instance* context);
		~scoped_write_request();
	};
	class scoped_read_request
	{
		DataModel* const dataModel;
	public:
		// Place this code around tasks that read a DataModel
		scoped_read_request(Instance* context);
		~scoped_read_request();
	};
	friend class scoped_write_request;
	friend class scoped_read_request;


	bool currentThreadHasWriteLock() const;
	static bool currentThreadHasWriteLock(Instance* context);

	// Services
	Workspace* getWorkspace() const { return workspace.get(); }
	GuiRoot* getGuiRoot() const { return guiRoot.get(); }  // +2968 (bring-up accessor).
	static DataModel* get(Instance* context);
	static const DataModel* get(const Instance* context);
	/* overrides TaskScheduler::Arbiter::getSyncronizationArbiter (2013 spelling preserved) */
	virtual Arbiter* getSyncronizationArbiter();

	// Rendering / input
	void renderStep();
	void renderPass();
	void renderGuiRoot();
	void renderMessageBox();
	void renderPlayerGui();
	void processUiEvent(const shared_ptr<InputObject>& event);
	void processWorkspaceEvent();
	void processPlayerGui();
	void switchViewMode();
	void toggleToolsOff();
	void setUiMessage(std::string message);
	void clearUiMessage();
	void setUiMessageBrickCount();
	// Bring-up view of uiMessage (no binary getter recovered).
	const std::string& getUiMessage() const { return uiMessage; }
	// NOTE (s467): computeGuiInset/getRenderMouseCursor/getUpdated-
	// MessageBoxText/physicsStep below are DECL-ONLY (no bodies, no EA;
	// Adorn-layout / mouse / terrain-cells / stepper eras) -- do not call.
	int computeGuiInset();
	int getRenderMouseCursor() const;
	std::string getScreenshotSEOInfo();
	std::string getUpdatedMessageBoxText();

	// Physics
	void physicsStep();

	// HTTP (shapes from mangled names in studio_function_list.txt:
	// httpGetAsync 0x2CDF06, httpPostAsync 0x2CE086, httpGet 0x2CE20E, httpPost 0x2CE4F2,
	// doHttpGet 0x2D7470/0x2D7778, doHttpPost 0x2D79B8/0x2D7CCE)
	std::string httpGet(std::string url, bool synchronous);
	std::string httpPost(std::string url, std::string data, bool synchronous);
	void httpGetAsync(std::string url, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction);
	void httpPostAsync(std::string url, std::string data, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction);
	static std::string doHttpGet(const std::string& url);
	static void doHttpGet(const std::string& url, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction);
	static std::string doHttpPost(const std::string& url, const std::string& data);
	static void doHttpPost(const std::string& url, const std::string& data, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction);

	// Load / stream
	void loadAssetIdIntoStream(unsigned assetId, std::istream& stream);

	// Tree events
	/*override*/ virtual void onChildAdded(Instance* child);		// vtable+0x58 0x0062ee00
	/*override*/ virtual void onChildChanged(Instance* instance, const PropertyChanged& event);	// vtable+0x64 0x0063e180
	// mac: 0x2E3120 (batches 622-624): takes Instance* -- a true override of
	// the Instance virtual (s521 correction: the shared_ptr shape was
	// 2016-carry and merely HID the virtual, so live adopts skipped the
	// census; now fixed, vtable-neutral).
	void onDescendantAdded(Instance* descendant);
	void onDescendantRemoving(const shared_ptr<Instance>& descendant);

	// Metrics / stats
	void reportMeasurement(std::string id, std::string key1, std::string value1, std::string key2, std::string value2);
	// mac: 0x2D16A8/0x2D16D2 — delegates to guiBuilder (+3488); strings by value (ESs).
	void addCustomStat(std::string name, std::string value);
	void removeCustomStat(std::string name);
	// IMetric (PLAYER 0x00650660 / 0x00642af0)
	/*override*/ virtual std::string getMetric(const std::string& name) const;
	/*override*/ virtual double getMetricValue(const std::string& name) const;
	void setJobsExtendedStatsWindow(double seconds);
	// TODO(decompile): bodies pending (getJobsInfo 0x2CE8BE, getJobsExtendedStats 0x2D0548)
	shared_ptr<const Reflection::ValueArray> getJobsExtendedStats();
	shared_ptr<const Reflection::ValueArray> getJobsInfo();
	double getJobIntervalPeakFraction(std::string jobname, double greaterThan);
	double getJobTimePeakFraction(std::string jobname, double greaterThan);
	double getGameTime() const;
	bool getIsGameLoaded();
	void setIsGameLoaded(bool value);
	bool getIsPersonalServer() const;
	void setIsPersonalServer(bool value);
	std::string getJobId() const;  // 2016 shape; 2013 EA 0x2E43D8 pending extraction
	void setRemoteBuildMode(bool remoteBuildMode);
	bool getRemoteBuildMode();

	// Screenshots / SEO (2013 task-based screenshot path)
	// NOTE (s467): TakeScreenshotTask/ScreenshotUploadTask are DECL-ONLY
	// (no bodies, no callers, no EA) -- do not call. computeGuiInset /
	// getRenderMouseCursor below are likewise decl-only (queued behind
	// the Adorn-layout era).
	void TakeScreenshotTask();
	void ScreenshotUploadTask();
	void setScreenshotSEOInfo(std::string info);
	void setVideoSEOInfo(std::string info);
	std::string getVideoSEOInfo() const { return videoSEOInfo; }

	Instance* getLightingDeprecated() const;
	static Reflection::RefPropDescriptor<DataModel, Instance> prop_lighting;

	// Misc
	void activateExperimentalFeatures();
	void traverseDataModelReporting(shared_ptr<Instance> node);  // 0x2D7276
	// Bring-up view of the save-error census (no binary getter recovered).
	int getReportingCount(const std::string& name) const {
		std::map<std::string, int>::const_iterator it = dataModelReportingData.find(name);
		return it == dataModelReportingData.end() ? 0 : it->second;
	}
	void writeStatsSettings();  // 0x2D16F4 — delegates to GuiBuilder::saveCustomStats (+3488).
	void updatePhysicsInstructions();
	void onRunTransition(RunTransition event);  // bound in initializeContents 0x2D3336
	// IDataState (Verb.h interface: setDirty(bool)=0, isDirty()=0)
	virtual void setDirty(bool dirty);
	virtual bool isDirty() const;



private:
	// Async HTTP completion fan-out (0x2D735A; matches 2016 exactly)
	static void HttpHelper(std::string* response, std::exception* exception, boost::function<void(std::string)> resumeFunction, boost::function<void(std::string)> errorFunction);

protected:
	// NOTE: no processAcceleratorsImpl (no 2013 mangle — 2016 scaffolding for
	// its InputObject-based processAccelerators; 2013's is (GuiEvent) 0x2DA2B0).

public:

	/* mac: 0x2D3742: createDataModel(bool, Verb*, DataModel*) — bool = ? (log "createDataModel") */
	static shared_ptr<Instance> createDataModel(bool flag, Verb* lockVerb, DataModel* parentDM);

	/* Hack-flag telemetry (batch279: dm+0xC30 boost::mutex locked through
	   boost::unique_lock + dm+0xC5C boost::unordered_set; ScriptContext
	   inlines lock+insert at the HATE_ILLEGAL_SCRIPTS sites, so the 2013
	   form is header-inline like 2016's addHackFlag). 2013 body = lock +
	   insert only -- no sendStats OR at the sites (the Lua fork ORs
	   sendStats directly; see HackDefines.h). */
	// __forceinline on WIN32 (as 2016): the DataModel ctor 0x0064ec50 inlines it (unique_lock
	// at esp+0x20, insert 0x00428430) although the ctor is large.
#ifdef WIN32
	__forceinline
#else
	inline
#endif
	void addHackFlag(unsigned int flag)
	{
		boost::mutex::scoped_lock l(hackFlagSetMutex);
		hackFlagSet.insert(flag);
	}

	// Live DataModels (ctor ++ at 0x0064f179, dtor -- at 0x0064d8c3); separate from
	// Diagnostics::Countable<DataModel>::count, which the base counts.
	static rbx::atomic<int> count;

	// ---- 2013 members (PLAYER ctor 0x0064ec50 and dtor 0x0064d850 read store by store) ----
	// Names: the existing tree's where a 2013 function proves the slot, else the 2016 member in
	// the same position whose ctor value matches (src2016), else m_unk. STUDIO (Oct 17) lacks
	// networkStatsWindowsOn (0xc60), so its members 0xc64..0xcac sit 4 bytes lower
	// (STUDIO ctor 0x0076b720: mutex 0xc60, hackFlagSet 0xc68, jobId 0xc80, write_requested 0xc9c,
	// itemChangedSignal 0xca8) and the 8-aligned boost::function at 0xcb0 absorbs the
	// difference; its GuiBuilder is 4 bytes smaller (parentDataModel at 0xe08). This header
	// follows PLAYER.
	struct MouseStats	// src2016: two RunningAverageTimeInterval<>(0.5), ctor 0x00641bd0
	{
		RunningAverageTimeInterval<> osMouseMove;	// 0x00
		RunningAverageTimeInterval<> mouseMove;		// 0x40
		MouseStats()
			: osMouseMove(0.5)
			, mouseMove(0.5)
		{
		}
	};

	std::auto_ptr<Verb> lockVerb;								// 0xac8 dtor deletes it (slot 1)
	rbx::signal<void()> screenshotSignal;						// 0xacc src2016
	rbx::signal<void(const std::string&)> screenshotReadySignal;	// 0xad0 src2016
	rbx::signal<void(bool)> screenshotUploadSignal;				// 0xad4 src2016
	rbx::signal<void(bool)> graphicsQualityShortcutSignal;		// 0xad8 event "GraphicsQualityChangeRequest"
	rbx::signal<void()> allowedGearTypeChanged;					// 0xadc event "AllowedGearTypeChanged"
	rbx::signal<void(const UIEvent&)> m_unk0xae0;				// 0xae0 fired by processUiEvent 0x0063dfed
	rbx::signal<void()> gameLoadedSignal;						// 0xae4 event "Loaded"
	rbx::signal<void()> luaExceptionSignal;						// 0xae8 fired by ScriptContext::reportError 0x006861a7
	MouseStats mouseStats;										// 0xaf0 (8-aligned: 0xaec is padding)
	shared_ptr<PlayerHUD>		playerHud;						// 0xb70 renderGuiRoot 0x0062ee50
	shared_ptr<CoreGuiService>	coreGuiService;					// 0xb78
	shared_ptr<LocalBackpack>	localBackpack;					// 0xb80 renderGuiRoot 0x0062ee67
	shared_ptr<Workspace>		workspace;						// 0xb88 ctor 0x0064edd1
	shared_ptr<RunService>		runService;						// 0xb90 getMetric reads IMetric+0xaec
	shared_ptr<StarterPackService> starterPack;					// 0xb98 renderGuiRoot 0x0062ee70
	shared_ptr<StarterGuiService> starterGui;					// 0xba0
	shared_ptr<GuiRoot>			guiRoot;						// 0xba8 ctor 0x0064ee06
	shared_ptr<GuiRoot>			guiRoot2;						// 0xbb0 ctor 0x0064ee17
	bool				forceArrowCursor;						// 0xbb8 src2016 (ctor true)
	std::string			uiMessage;								// 0xbbc
	int					drawId;									// 0xbd8 src2016 (renderPass3dAdorn 0x0064ab1d ++)
	IMetric*			tempMetric;								// 0xbdc src2016 (renderPass2d 0x006457e7 sets, then NULL)
	IMetric*			networkMetric;							// 0xbe0 src2016 (ctor NULL)
	volatile bool		dirtyFlag;								// 0xbe4 IDataState
	bool				isInitialized;							// 0xbe5 asserts in httpGet & co.
	volatile bool		isContentLoaded;						// 0xbe6 src2016 (Replicator jobs 0x00598ff7 read it). volatile: the ctor 0x0064ee43 stores 0xbe4/0xbe5 before the EH state, only with this volatile (probe)
	PhysicsInstructions	physicsInstructions;					// 0xbe8 ctor 0x0084d2f0 (DataModel ctor 0x0064ee60)
	int					numPartInstances;						// 0xc48 onDescendantAdded 0x00645dcf
	int					numInstances;							// 0xc4c onDescendantAdded 0x00645da2
	int					shutdownRequestedCount;					// 0xc50
	int					physicsStepID;							// 0xc54 src2016 (physicsStep 0x00642a7a ++)
	bool				isGameLoaded;							// 0xc58
	bool				isPersonalServer;						// 0xc59 getter 0x0062d240, setter 0x0062d250
	Game*				game;									// 0xc5c Game::setupDataModel 0x006556b8
	bool				networkStatsWindowsOn;					// 0xc60 PLAYER only (processAccelerators 0x0064fd1e)
	boost::mutex hackFlagSetMutex;								// 0xc64
	boost::unordered_set<unsigned int> hackFlagSet;				// 0xc6c
	std::string			jobId;									// 0xc84 getter 0x0062d1e0
	volatile long write_requested;								// 0xca0 assert "write_requested==1"
	volatile long read_requested;								// 0xca4
	volatile DWORD writeRequestingThread;						// 0xca8 LegacyLock::Implementation 0x006420db
	rbx::signal<void(shared_ptr<Instance>, const Reflection::PropertyDescriptor*)> itemChangedSignal;	// 0xcac event "ItemChanged"
	// +3208 (mac): Lua shutdown hook invoked by requestShutdown 0x2D2CBA (set by Studio).
	boost::function<bool()> requestShutdownCallback;			// 0xcb0 (0x20, 8-aligned)
	rbx::signal<void()> saveFinishedSignal;						// 0xcd0
	weak_ptr<Instance>	guiTargetInstance;						// 0xcd4 src2016 (processEvent 0x0064aef1)
	bool				mouseOverGui;							// 0xcdc src2016 (ctor false; processEvent writes)
	bool				suppressNavKeys;						// 0xcdd (not set by the ctor)
	RBX::mutex genericJobsLock;									// 0xce0
	GenericJobs genericJobs;									// 0xcf8
	RBX::mutex			debugLock;								// 0xd50
	std::map<std::string, int> dataModelReportingData;			// 0xd68
	bool				remoteBuildMode;						// 0xd74
	std::string			serverSaveUrl;							// 0xd78
	std::string			screenshotSEOInfo;						// 0xd94
	std::string			videoSEOInfo;							// 0xdb0
	int					placeID;								// 0xdcc setPlaceID 0x0063e0eb
	bool				experimentalFeatures;					// 0xdd0 (not set by the ctor)
	int					placeVersion;							// 0xdd4 setPlaceVersion 0x0062d942
	int					creatorID;								// 0xdd8
	CreatorType			creatorType;							// 0xddc
	Genre				genre;									// 0xde0 (not set by the ctor)
	GearGenreSetting	gearGenreSetting;						// 0xde4 (not set by the ctor)
	unsigned			allowedGearTypes;						// 0xde8 (not set by the ctor)
	GuiBuilder			guiBuilder;								// 0xdec 0x20 (STUDIO 0x1c); ctor 0x00644f30, dtor 0x0056b810
	DataModel*			parentDataModel;						// 0xe0c second ctor argument
};

} // namespace RBX
