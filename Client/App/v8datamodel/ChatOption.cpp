/* App/v8datamodel/ChatOption.cpp -- October 2013 (C2 only; D2 TODO). */

#include "stdafx.h"

#include "V8DataModel/ChatOption.h"

namespace RBX {

	// mac: 0x46F958 (116B).
	ChatOption::ChatOption(const std::string& label)
		: children()
		, label(label)
	{
	}

} // namespace RBX
