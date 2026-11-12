/* App/v8datamodel/ChangeHistoryService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ChangeHistoryService.h"

namespace RBX {
	const char* const sChangeHistoryService = "ChangeHistoryService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ChangeHistoryService);
}

namespace RBX {

// mac: 0x2CF1D0 calls clearWaypoints (waypoint list at +29/+116). TODO: body.
void ChangeHistoryService::clearWaypoints()
{
}

// mac: 0x2D3952: doCloseDataModel setEnabled(false). TODO: body.
void ChangeHistoryService::setEnabled(bool enabled)
{
	(void)enabled;
}

} // namespace RBX
