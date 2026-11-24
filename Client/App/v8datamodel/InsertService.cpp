/* App/v8datamodel/InsertService.cpp -- October 2013. The setters, the ctor and the implicit dtor from
   the PLAYER bytes (0x007cfb40..0x007dbe40); the insert, asset-loading, web-request and reflection
   bodies are not written yet. See include/V8DataModel/InsertService.h for the layout evidence. */

#include "stdafx.h"

#include "V8DataModel/InsertService.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Workspace.h"
#include "security/SecurityContext.h"
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

} // namespace RBX
