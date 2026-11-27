/* App/v8datamodel/Settings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Settings.h"
#include "V8DataModel/GlobalSettings.h"
#include "V8DataModel/Selection.h"
#include "decomp.h"

namespace RBX {
	const char* const sSettings = "Settings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Settings);
}

namespace RBX {
	// FUNCTION: PLAYER 0x006562c0
	// FUNCTION: STUDIO 0x0073edd0
	bool Settings::InvalidDescendentDetector::invalid(const Instance* instance)
	{
		return !dynamic_cast<const GlobalBasicSettings::Item*>(instance)
			&& !dynamic_cast<const GlobalAdvancedSettings::Item*>(instance)
			&& !dynamic_cast<const Selection*>(instance);
	}

	// FUNCTION: PLAYER 0x006566d0
	// FUNCTION: STUDIO 0x0073f1b0
	void Settings::InvalidDescendentDetector::operator()(shared_ptr<Instance> descendant)
	{
		anyInvalid |= invalid(descendant.get());
	}

	// Instance::visitDescendants<InvalidDescendentDetector> stays out of line (PLAYER 0x006569a0, STUDIO 0x0073f360).
	// FUNCTION: PLAYER 0x00656a80
	// FUNCTION: STUDIO 0x0073f440
	void Settings::verifyAddDescendant(const Instance* newParent, const Instance* instanceGettingNewParent) const
	{
		bool invalid = InvalidDescendentDetector::invalid(instanceGettingNewParent);
		if (!invalid)
		{
			InvalidDescendentDetector detector;
			instanceGettingNewParent->visitDescendants(detector);
			invalid = detector.anyInvalid;
		}
		if (invalid)
			throw RBX::runtime_error("Not allowed to add that under settings");
		Super::verifyAddDescendant(newParent, instanceGettingNewParent);
	}

	// Body not written yet (visitDescendants<InvalidDescendentDetector>, then writes settingsFile unless
	// settingsErased). STUB body so the shell's Application::Shutdown pairs its call.
	// STUB: PLAYER 0x006576d0
	// STUB: STUDIO 0x0073fe60
	void Settings::saveState()
	{
		STUB(0x006576d0);
	}

	// The base DescribedNonCreatable<Settings, ServiceProvider, sSettings> ctor stays out of line
	// (PLAYER 0x00657cf0).
	// FUNCTION: PLAYER 0x00657f60
	// FUNCTION: STUDIO 0x00740700
	Settings::Settings(const std::string& settingsFile)
		: settingsFile(settingsFile)
		, settingsErased(false)
	{
	}
}
