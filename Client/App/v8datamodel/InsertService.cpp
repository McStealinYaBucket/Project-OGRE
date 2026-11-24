/* App/v8datamodel/InsertService.cpp -- October 2013. Setters, ctor, implicit dtor and onServiceProvider
   (its assert on the 2013 line 178) from the PLAYER bytes (0x007cfb40..0x007de310); the insert, asset-
   loading, web-request and reflection bodies are not written yet. Layout evidence in the header. */

#include "stdafx.h"

#include "V8DataModel/InsertService.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Workspace.h"
#include "security/SecurityContext.h"
#include "Network/Players.h"
#include <boost/bind.hpp>
#include "decomp.h"

DECOMP_SIZE_ASSERT(RBX::InsertService, 0x188);

namespace RBX {
	const char* const sInsertService = "InsertService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::InsertService);
}

namespace RBX {

	// FUNCTION: PLAYER 0x007cfb40
	// FUNCTION: STUDIO 0x0086b520
	void InsertService::setTrustLevel(float value)
	{
		trustLevel = value;
	}

	// FUNCTION: PLAYER 0x007cfb60
	// FUNCTION: STUDIO 0x0086b540
	void InsertService::setFreeModelUrl(std::string value)
	{
		freeModelUrl = value;
	}

	// FUNCTION: PLAYER 0x007cfbb0
	// FUNCTION: STUDIO 0x0086b590
	void InsertService::setFreeDecalUrl(std::string value)
	{
		freeDecalUrl = value;
	}

	// FUNCTION: PLAYER 0x007cfc00
	// FUNCTION: STUDIO 0x0086b5e0
	void InsertService::setBaseSetsUrl(std::string value)
	{
		baseSetsUrl = value;
	}

	// FUNCTION: PLAYER 0x007cfc50
	// FUNCTION: STUDIO 0x0086b630
	void InsertService::setUserSetsUrl(std::string value)
	{
		userSetsUrl = value;
	}

	// FUNCTION: PLAYER 0x007cfca0
	// FUNCTION: STUDIO 0x0086b680
	void InsertService::setCollectionUrl(std::string value)
	{
		collectionUrl = value;
	}

	// FUNCTION: PLAYER 0x007cfcf0
	// FUNCTION: STUDIO 0x0086b6d0
	void InsertService::setAssetUrl(std::string value)
	{
		assetUrl = value;
	}

	// FUNCTION: PLAYER 0x007cfd40
	// FUNCTION: STUDIO 0x0086b720
	void InsertService::setAssetVersionUrl(std::string value)
	{
		assetVersionUrl = value;
	}

	// 2013 passes two bools to publicInsertRaw and requires permission 2 (2016: Plugin).
	// STUB: PLAYER 0x007d6a20
	void InsertService::insert(shared_ptr<Instance> instance)
	{
		if (DataModel* dataModel = (DataModel*)getParent())
		{
			if (!instance)
				throw std::runtime_error("instance must be non-nil");

			if (instance->getRobloxLocked())
				RBX::Security::Context::current().requirePermission((RBX::Security::Permissions)2, "Roblox locked in InsertService:Insert()");

			Instances instances;
			instances.push_back(instance);

			std::vector<weak_ptr<PartInstance> > parts;
			dataModel->getWorkspace()->publicInsertRaw(instances, dataModel->getWorkspace(), parts, true, true);
		}
	}

	// 2013: no holder Folder; the service itself is made non-archivable, then loadCount is cleared.
	// FUNCTION: PLAYER 0x007db980
	// STUB: STUDIO 0x008775b0
	InsertService::InsertService()
		: trustLevel(0.5f)
	{
		setName(sInsertService);
		Instance::propArchivable.setValue(this, false);
		loadCount = 0;
	}


	// The three backend handlers fire the remote insert events; their bodies wait on the
	// InsertService reflection block (RemoteEventDesc statics, e.g. 0x00e0fd3c).
	// STUB: PLAYER 0x007ddf70
	// STUB: STUDIO 0x00879ba0
	void InsertService::backendInsertRequested(std::string key, ContentId contentId)
	{
		STUB(0x007ddf70);
	}

	// STUB: PLAYER 0x007de070
	// STUB: STUDIO 0x00879ca0
	void InsertService::backendInsertAssetVersionRequested(std::string key, int assetVersionId, int userId)
	{
		STUB(0x007de070);
	}

	// STUB: PLAYER 0x007de1c0
	// STUB: STUDIO 0x00879df0
	void InsertService::backendInsertAssetRequested(std::string key, int assetId, int userId)
	{
		STUB(0x007de1c0);
	}



























	// Only the backend (server) listens for insert requests. Instance::onServiceProvider is empty
	// and inline, so Super adds no call.
	// FUNCTION: PLAYER 0x007de310
	// FUNCTION: STUDIO 0x00879f40
	void InsertService::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		if (oldProvider && Network::Players::backendProcessing(oldProvider))
		{
			RBXASSERT(backendInsertConnection.connected());
			backendInsertConnection.disconnect();
			backendInsertAssetConnection.disconnect();
			backendInsertAssetVersionConnection.disconnect();
		}

		Super::onServiceProvider(oldProvider, newProvider);

		if (newProvider && Network::Players::backendProcessing(newProvider))
		{
			backendInsertConnection = insertRequestSignal.connect(boost::bind(&InsertService::backendInsertRequested, this, _1, _2));
			backendInsertAssetConnection = insertRequestAssetSignal.connect(boost::bind(&InsertService::backendInsertAssetRequested, this, _1, _2, _3));
			backendInsertAssetVersionConnection = insertRequestAssetVersionSignal.connect(boost::bind(&InsertService::backendInsertAssetVersionRequested, this, _1, _2, _3));
		}
	}

} // namespace RBX
