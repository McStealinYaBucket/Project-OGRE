/* App/v8datamodel/BaseScript.cpp -- October 2013 (full C1 port: script base + engine glue).
   Recon evidence: see V8DataModel/BaseScript.h header comment (batches 730-771, MAC dbq24).
   All bodies reconstructed from verbatim decompiles/disasm:
     restartScript 0x182754, computeNewWorkspace 0x1827A2, preloadCode 0x18287E,
     onServiceProvider 0x1829B8, setScriptId 0x18104A, setDisabled 0x1810B0,
     requestHash 0x182DD0, isRobloxScript 0x182D9A, requestCode 0x1829FE
     (batch756 disasm + batch763:125-217 disasm). */

#include "stdafx.h"

#include "V8DataModel/BaseScript.h"
#include "V8DataModel/IScriptFilter.h"
#include "V8DataModel/ContentProvider.h"

#include "V8Tree/Service.h"

namespace RBX {
	const char* const sBaseScript = "BaseScript";
}

using namespace RBX;

REFLECTION_BEGIN();
const Reflection::PropDescriptor<BaseScript, bool> BaseScript::prop_Disabled("Disabled", category_Behavior, &BaseScript::getDisabled, &BaseScript::setDisabled, Reflection::PropertyDescriptor::STANDARD, Security::None);
const Reflection::PropDescriptor<BaseScript, ContentId> BaseScript::prop_SourceCodeId("LinkedSource", category_Data, &BaseScript::getScriptId, &BaseScript::setScriptId, Reflection::PropertyDescriptor::STANDARD, Security::None);
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::BaseScript);
}

namespace RBX {

	const std::string BaseScript::emptyString;

	// STUB: PLAYER 0x0066cf10
	// STUB: STUDIO 0x0077dce0
	BaseScript::BaseScript()
		: workspace(NULL)
		, disabled(false)
		, badLinkedScript(false)
		, unknown115(true)
		, threadNode()
	{
		// mac: 0x1810F4 ctor order (batch768 disasm): scriptId ContentId-ctor'd @+92;
		// workspace @+100 zeroed; localPlayer weak @+104/108 zeroed; disabled
		// @+112 = false; badLinkedScript @+114 = false; unknown115 @+115 = 1;
		// threadNode @+116 zeroed; starting/stopped signals default-constructed.
		// (scriptId/localPlayer/localPlayer-weak/starting/stopped all default-construct
		// above via the init list omissions.)
	}

	BaseScript::~BaseScript()
	{
	}

	// mac: 0x1827A2 (batch740:443-475)
	// STUB: PLAYER 0x0066e450
	// STUB: STUDIO 0x0077f1b0
	RuntimeScriptService* BaseScript::computeNewWorkspace()
	{
		if (disabled)
			return NULL;

		Instance* ancestor = getParent();
		if (ancestor == NULL)
			return NULL;

		bool ok = false;
		do
		{
			if (ok)
				break;

			IScriptFilter* filter = dynamic_cast<IScriptFilter*>(ancestor);
			if (filter)
				ok = filter->scriptShouldRun(this);

			ancestor = ancestor->getParent();
		}
		while (ancestor != NULL);

		return ok ? ServiceProvider::create<RuntimeScriptService>(this) : NULL;
	}

	// mac: 0x182754 (batch745:10-31)
	// DELTA note: the binary tail returns the runScript() call's result; the port's
	// RuntimeScriptService::runScript is void, so the restart returns the (new) workspace.
	RuntimeScriptService* BaseScript::restartScript()
	{
		RuntimeScriptService* newWorkspace = computeNewWorkspace();
		if (workspace == newWorkspace)
			return workspace;

		if (workspace != NULL)
		{
			RuntimeScriptService* old = workspace;
			workspace = NULL;
			old->releaseScript(this);
		}

		workspace = newWorkspace;

		if (newWorkspace != NULL)
			newWorkspace->runScript(this);

		return newWorkspace;
	}

	// mac: 0x182822 (batch769:1009-1030, eighth era): the parent-changed hand-off.
	// Instance super-call, then the same swap restartScript performs, with
	// NO early return when the computed service equals the current one (the
	// binary falls through; a match means neither release nor run fires).
	// Binary returns the service pointer as its word; the Instance base
	// virtual is void here (landing convention, like the scoped family).
	// FUNCTION: PLAYER 0x0066e500
	// FUNCTION: STUDIO 0x0077f260
	void BaseScript::onAncestorChanged(const AncestorChanged& event)
	{
		Instance::onAncestorChanged(event);

		RuntimeScriptService* newWorkspace = computeNewWorkspace();
		if (workspace == newWorkspace)
			return;

		if (workspace != NULL)
		{
			RuntimeScriptService* old = workspace;
			workspace = NULL;
			old->releaseScript(this);
		}

		workspace = newWorkspace;

		if (newWorkspace != NULL)
			newWorkspace->runScript(this);
	}

	// mac: 0x18287E (batch740:478-504 / batch768:109-137)
	// STUB: PLAYER 0x0066d480
	// STUB: STUDIO 0x0077e230
	void BaseScript::preloadCode()
	{
		Instance* ancestor = getParent();
		while (ancestor != NULL)
		{
			IScriptFilter* filter = dynamic_cast<IScriptFilter*>(ancestor);
			if (filter)
			{
				filter->scriptShouldRun(this);
				requestCode(NULL);	/* result discarded (2013: flyweight cache warm-up; port: plain copy discarded) */
				return;
			}
			ancestor = ancestor->getParent();
		}
	}

	// mac: 0x1829FE (batch756 + batch763:125-217 disasm) -- the LinkedSource fetch path.
	// DELTA notes:
	//  - the 2013 timeout float came from a pointer-global (off_EADAD8 deref); the
	//    port passes ContentProvider::PRIORITY_SCRIPT (its named script priority).
	//  - the !content / !contentProvider arms return the not-loaded Code; the
	//    binary's 182C84/182C93 tails were only partly decoded -- any
	//    badLinkedScript=true side effect there remains unported (writer site
	//    undecoded; setScriptId resets the flag).
	// STUB: PLAYER 0x0066ea50
	// STUB: STUDIO 0x0077f7b0
	BaseScript::Code BaseScript::requestCode(ScriptInformationProvider* sip)
	{
		(void)sip;	/* decoded body never reads the provider */

		if (badLinkedScript)
			return Code();

		ContentProvider* contentProvider = ServiceProvider::create<ContentProvider>(this);
		if (contentProvider == NULL)
			return Code();

		boost::shared_ptr<const std::string> content = contentProvider->requestContentString(scriptId, ContentProvider::PRIORITY_SCRIPT);
		if (!content)
			return Code();

		std::string assetId = scriptId.getAssetId();
		const char* verified = ContentProvider::verifyRequestedScriptSignature(content->c_str(), assetId, unknown115);

		Code code;
		code.loaded = true;
		code.script = ProtectedString::fromTrustedSource(verified);
		return code;
	}

	// mac: 0x182D9A
	// FUNCTION: PLAYER 0x006be550 FOLDED
	// FUNCTION: STUDIO 0x0078ad10 FOLDED
	bool BaseScript::isRobloxScript()
	{
		return true;
	}

	// mac: 0x182DD0
	// FUNCTION: PLAYER 0x006692f0
	// FUNCTION: STUDIO 0x0077a1e0
	const std::string& BaseScript::requestHash()
	{
		return emptyString;
	}

	// mac: 0x1829B8 (batch746:62-85). No Instance super-call (pin).
	// FUNCTION: PLAYER 0x00669280
	// FUNCTION: STUDIO 0x0077a170
	void BaseScript::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		if (oldProvider != NULL)
		{
			RuntimeScriptService* old = workspace;
			if (old != NULL)
			{
				workspace = NULL;
				old->releaseScript(this);
			}
		}

		if (newProvider != NULL)
			preloadCode();
	}

	// mac: 0x1810B0 (batch763:308-323)
	void BaseScript::setDisabled(bool value)
	{
		if (disabled != value)
		{
			disabled = value;
			restartScript();
			raisePropertyChanged(prop_Disabled);
		}
	}

	// mac: 0x18104A (batch763:288-305)
	void BaseScript::setScriptId(const ContentId& value)
	{
		if (scriptId != value)
		{
			scriptId = value;
			badLinkedScript = false;
			restartScript();
			raisePropertyChanged(prop_SourceCodeId);
		}
	}

} // namespace RBX
