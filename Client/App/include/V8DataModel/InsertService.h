/* V8DataModel/InsertService.h -- October 2013.
   2013 layout from the PLAYER bytes: ctor 0x007db980 (STUDIO 0x008775b0), implicit dtor 0x007dbbf0,
   `new 0x188` in FactoryProduct<InsertService>::Creator::create 0x00565c70, the eight setters
   0x007cfb40..0x007cfd40 (their member offsets) and onServiceProvider 0x007de310 (which connection
   takes which signal). The class is Described<InsertService, sInsertService,
   FactoryProduct<InsertService, Instance, sInsertService, Instance>, 3 (INTERNAL), 0> (mangled name
   of classDescriptor 0x0078a0f0, out-of-line Described ctor 0x007d8030), plus Service at 0x60.
   Signal signatures from iOS 10-29 (EventDesc/RemoteEventDesc<InsertService, ...>); the Callback
   map from iOS std::map<std::string, InsertService::Callback>::operator[]; the url members from
   iOS setFreeModelUrl/... (2016 keeps the setters but drops the members).
   Where 2013 disagrees with 2016: INTERNAL, not PERSISTENT_HIDDEN; seven url strings and a
   trustLevel float; no internalDeleteSignal, backendInternalDeleteConnection, holder or
   allowInsertFreeModels (the object ends at 0x188 with loadCount). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/ContentId.h"
#include "rbx/signal.h"
#include "rbx/atomic.h"
#include "boost/thread/recursive_mutex.hpp"
#include <map>

namespace RBX {

	extern const char* const sInsertService;

	// SIZE 0x188
	class InsertService
		: public DescribedCreatable<InsertService, Instance, sInsertService, Reflection::ClassDescriptor::INTERNAL>
		, public Service	// 0x60 (isPublic, ctor stores 1)
	{
	public:
		InsertService();

		// SYNTHETIC: PLAYER 0x007dbbf0
		// SYNTHETIC: STUDIO 0x00877820
		// RBX::InsertService::~InsertService

		// SYNTHETIC: PLAYER 0x007dbe40
		// SYNTHETIC: STUDIO 0x00877a70
		// RBX::InsertService::`scalar deleting destructor'

		rbx::remote_signal<void(std::string, ContentId)> insertRequestSignal;				// 0x64
		rbx::remote_signal<void(std::string, int, int)> insertRequestAssetSignal;			// 0x6c
		rbx::remote_signal<void(std::string, int, int)> insertRequestAssetVersionSignal;	// 0x74
		rbx::remote_signal<void(std::string, shared_ptr<Instance>)> insertReadySignal;		// 0x7c
		rbx::remote_signal<void(std::string, std::string)> insertErrorSignal;				// 0x84

		void insert(shared_ptr<Instance> instance);

		void setTrustLevel(float value);
		void setFreeModelUrl(std::string value);
		void setFreeDecalUrl(std::string value);
		void setBaseSetsUrl(std::string value);
		void setUserSetsUrl(std::string value);
		void setCollectionUrl(std::string value);
		void setAssetUrl(std::string value);
		void setAssetVersionUrl(std::string value);

	private:
		std::string freeModelUrl;		// 0x8c  setFreeModelUrl 0x007cfb60
		std::string freeDecalUrl;		// 0xa8  setFreeDecalUrl 0x007cfbb0
		std::string baseSetsUrl;		// 0xc4  setBaseSetsUrl 0x007cfc00
		std::string userSetsUrl;		// 0xe0  setUserSetsUrl 0x007cfc50
		std::string collectionUrl;		// 0xfc  setCollectionUrl 0x007cfca0
		std::string assetUrl;			// 0x118 setAssetUrl 0x007cfcf0
		std::string assetVersionUrl;	// 0x134 setAssetVersionUrl 0x007cfd40
		float trustLevel;				// 0x150 setTrustLevel 0x007cfb40; ctor 0.5

		rbx::signals::connection backendInsertConnection;				// 0x154 insertRequestSignal
		rbx::signals::connection backendInsertAssetConnection;			// 0x158 insertRequestAssetSignal
		rbx::signals::connection backendInsertAssetVersionConnection;	// 0x15c insertRequestAssetVersionSignal
		rbx::signals::connection frontendInsertReadyConnection;			// 0x160
		rbx::signals::connection frontendInsertErrorConnection;			// 0x164

		boost::recursive_mutex callbackSync;	// 0x168 (the dtor's CloseHandle reads +0x174)

		struct Callback
		{
			boost::function<void(shared_ptr<Instance>)> resumeFunction;
			boost::function<void(std::string)> errorFunction;
		};
		typedef std::map<std::string, Callback> CallbackLibrary;
		CallbackLibrary callbackLibrary;	// 0x178 (node _Isnil at +0x71: Callback is 8-aligned)
		rbx::atomic<int> loadCount;			// 0x184
	};

} // namespace RBX

// TEMPLATE: PLAYER 0x007d8030
// TEMPLATE: STUDIO 0x00873ae0
// ??0?$Described@VInsertService@RBX@@$E?sInsertService@2@3QBDBV?$FactoryProduct@VInsertService@RBX@@VInstance@2@$E?sInsertService@2@3QBDBV32@@2@$02$0A@@Reflection@RBX@@QAE@XZ
