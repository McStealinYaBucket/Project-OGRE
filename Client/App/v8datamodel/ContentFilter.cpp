/* App/v8datamodel/ContentFilter.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ContentFilter.h"

namespace RBX {
	const char* const sContentFilter = "ContentFilter";

	// GLOBAL: PLAYER 0x00b77cc0
	const unsigned ContentFilter::MAX_CONTENT_FILTER_SIZE = 1024;
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ContentFilter);
}
