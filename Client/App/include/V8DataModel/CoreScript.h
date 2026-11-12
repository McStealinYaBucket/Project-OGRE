/* V8DataModel/CoreScript.h -- October 2013 (full: substituted admin/core script).
   Recon evidence (batches 730-771, MAC dbq24):
   - Class shape: Described<CoreScript, sCoreScript, NonFactoryProduct<BaseScript,
     sCoreScript>, ClassDescriptor::Functionality 1 (= INTERNAL_LOCAL),
     Security::Permissions 0> (batch770 ctor disasm + batch768:234-268 CoreScript
     ctor 0x15F8B4). CoreScript : BaseScript (NOT Script!), created with a
     ContentId which goes straight into prop_SourceCodeId via virtual dispatch
     (port: direct static-prop call -- binary used the getset object's setValue).
   - CoreScript ctor 0x15F8B4: BaseScript() -> *(WORD*)(this+128) = 0 (missing @+128
     and reported @+129 both false) -> setName("CoreScript") ->
     prop_SourceCodeId.setValue(this, contentId) -> setRobloxLocked(this, true).
   - onServiceProvider 0x15FA22 (batch738:898-975 / batch745:34-100): old &&
     !workspace -> sc = ServiceProvider::find<ScriptContext>(oldProvider);
     RBXASSERT(sc) ("sc", script/CoreScript.cpp:33); RBXASSERT(sc->hasScript(this))
     ("sc->hasScript(this)", :34); sc->removeScript(this); then
     BaseScript::onServiceProvider in BOTH arms.
   - requestCode 0x15FB68 (batch738:994-1128): adminScriptsPath.empty() || missing ->
     BaseScript::requestCode; else path = initial_path()/adminScriptsPath/
     (getName() + ".lua"); filesystem status <= 1 -> missing = true +
     StandardOut::singleton()->printf(MESSAGE_WARNING(2), "Missing substitution:
     \"%s\"", path) + reported = true + Base fallback; else !reported ->
     printf(MESSAGE_INFO(1), "Substituting \"%s\" with \"%s\"", getFullName, path)
     + reported = true; iffstream(file) >> stringstream -> Code(true, fromTrustedSource).
   - extraErrorReporting 0x1601A6 (batch769:1561-1691): safe_lua_tostring(thread, -1)
     fallback "Error occurred, no output from Lua."; "\n\nCallStack:\n" +
     ScriptContext::extractCallStack; RBXASSERT(source.get() == this)
     (CoreScript.cpp:95); PlaceID via DataModel::get(+3456); .cse write =
     FileSystem::getUserDirectory() + name + "_ln<line>_.cse"; ifstream open ->
     RBX::Log::timeStamp(file, 1) + the report; close.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CoreScript.h */

#pragma once

#include "V8DataModel/BaseScript.h"

struct lua_State;

namespace RBX {

	extern const char* const sCoreScript;

	class ScriptContext;

	class CoreScript
		: public DescribedNonCreatable<CoreScript, BaseScript, sCoreScript, Reflection::ClassDescriptor::INTERNAL_LOCAL>
	{
	public:
		CoreScript(const ContentId& contentId);
		/*override*/ virtual ~CoreScript();

		// mac: 0x15FA22 (batch738:898+)
		/*override*/ virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);

		// mac: 0x15FB68 (batch738:994-1128)
		/*override*/ virtual Code requestCode(ScriptInformationProvider* sip);

		// mac: 0x1601A6 (batch769:1561-1691)
		/*override*/ virtual void extraErrorReporting(lua_State* thread);

		// ctor-zeroed pair at CoreScript+128/129 (the ctor's single WORD write):
		// substitution state flags used by requestCode.
		bool missing;
		bool reported;
	};

} // namespace RBX
