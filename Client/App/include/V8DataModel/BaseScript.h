/* V8DataModel/BaseScript.h -- October 2013 (full: script base + engine glue).
   Recon evidence (batches 730-771, MAC dbq24 0.126.0.39503):
   - Class shape: NonFactoryProduct<BaseScript, Instance> @0x187650; sBaseScript = "BaseScript".
   - BaseScript ctor 0x1810F4 (batch768 disasm): scriptId (ContentId) @+92 ctor'd; workspace
     RuntimeScriptService* @+100 zeroed; localPlayer weak @+104-111 zeroed; disabled @+112 =
     false; badLinkedScript @+114 = false; unknown115 @+115 = 1 (READER DECODED: requestCode
     0x1829FE passes it to ContentProvider::verifyRequestedScriptSignature, batch756:34);
     threadNode @+116 zeroed; starting/stopped signals initialized (starting =
     signal<void(lua_State*)> lolled in startScript 0x193E26; stopped = signal<void()>
     fired in ScriptContext::disassociateState).
   - computeNewWorkspace 0x1827A2 (batch740:443-475); restartScript 0x182754
     (batch745:10-31, returns RuntimeScriptService*, forwards to
     RuntimeScriptService::releaseScript/runScript); preloadCode 0x18287E
     (batch740:478-504: walk ancestors, scriptShouldRun hit -> requestCode(0), result
     discarded i.e. the flyweight warm-up); requestHash 0x182DD0 -> BaseScript::emptyString;
     onServiceProvider 0x1829B8 (batch746:62-85: old -> clear workspace + releaseScript;
     new -> preloadCode(); NO Instance super-call); setScriptId 0x18104A (batch763:288-305:
     ContentId-assign + badLinkedScript = false + restartScript() +
     raisePropertyChanged(prop_SourceCodeId)); setDisabled 0x1810B0 (batch763:308-323:
     change -> restartScript() + raisePropertyChanged(prop_Disabled)); isRobloxScript
     0x182D9A -> return 1.
   - requestCode signature: __ZN3RBX10BaseScript11requestCodeEPNS_25ScriptInformationProviderE
     = virtual Code requestCode(ScriptInformationProvider*); ALL decoded call sites pass
     NULL (the "requestCode(NULL)" transcription). Base impl 0x1829FE (batch756 disasm) =
     the LinkedSource fetch path.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BaseScript.h */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/ContentId.h"
#include "Util/ProtectedString.h"
#include "script/ThreadRef.h"

#include "rbx/signal.h"

#include <string>

struct lua_State;

namespace RBX {

	class RuntimeScriptService;
	class ScriptInformationProvider;

	extern const char* const sBaseScript;

	class BaseScript
		: public DescribedNonCreatable<BaseScript, Instance, sBaseScript>
	{
	public:
		// The request result: { bool loaded; ProtectedString script; } (sret layout
		// confirmed by both decomps: startScript's +132 slot call and
		// Script::requestCode's `*this = 0/1; *(this+4) = flyweight` writes).
		struct Code
		{
			bool loaded;
			ProtectedString script;
			Code() : loaded(false) {}
			Code(bool loaded, const ProtectedString& script) : loaded(loaded), script(script) {}
		};

		BaseScript();
		/*override*/ virtual ~BaseScript();

		// statics (InitFunc_122 TU)
		// Substitution root for development libraries (client sets it at
		// startup; empty keeps the asset-id fetch path). Definition in
		// v8datamodel/Script.cpp (2016 home).
		static std::string adminScriptsPath;	// 0xEAD3A4

		// requestHash result-holder (0x182DD0 returns a reference to it).
		static const std::string emptyString;

		// mac: 0x1827A2 (batch740:443-475): disabled -> NULL; else walk the ancestor
		// chain; first IScriptFilter hit's scriptShouldRun -> 
		// ServiceProvider::create<RuntimeScriptService>(this), else NULL.
		RuntimeScriptService* computeNewWorkspace();

		// mac: 0x182754 (batch745:10-31): restartScript -- if workspace != computeNewWorkspace(),
		// release the old (workspace = NULL first) then run the new; returns the workspace.
		RuntimeScriptService* restartScript();

		// mac: 0x182822 (batch769:1009-1030, eighth era): the reparent half of the
		// runner hand-off -- Instance super-call, then the same release/run
		// swap restartScript uses, keyed off computeNewWorkspace() (the
		// binary's +100 workspace member IS this pointer slot).
		/*override*/ virtual void onAncestorChanged(const AncestorChanged& event);

		// mac: 0x18287E (batch740:478-504 / batch768:109-137): walk the ancestor chain
		// (IScriptFilter -> scriptShouldRun hit) then requestCode(NULL) to warm the
		// code storage. The result is discarded (2013 flyweight warm-up; DELTA note:
		// the port discards the plain copy).
		/*override*/ virtual void preloadCode();

		// Mangled: __ZN3RBX10BaseScript11requestCodeEPNS_25ScriptInformationProviderE.
		// Base impl 0x1829FE (batch756 disasm) = the LinkedSource fetch:
		// badLinkedScript -> Code(false); else create<ContentProvider>(this) ->
		// requestContentString(scriptId, timeout) -> verifyRequestedScriptSignature
		// (content, scriptId.getAssetId(), unknown115) -> fromTrustedSource into
		// Code(true, ...). Unfilled/verified-false arms remain loud DELTAs in the .cpp.
		virtual Code requestCode(ScriptInformationProvider* sip);

		// mac: 0x182D9A: pure BaseScript always reports roblox-authored.
		virtual bool isRobloxScript();

		// reportError tail (0x19664C: vtable +140/0x8C).  CoreScript overrides it;
		// the default is empty.  vtable+0x7c, before requestHash (+0x80): 2013 vftables
		// 0x00b339a4 (BaseScript) and 0x00b85254 (CoreScript), iOS 10-29 agrees.
		virtual void extraErrorReporting(lua_State* thread) { (void)thread; }

		// mac: 0x182DD0: returns BaseScript::emptyString (reference return; batch768:163-167).
		virtual const std::string& requestHash();  // vtable+0x80

		// mac: 0x1829B8 (batch746:62-85): oldProvider -> clear workspace & releaseScript;
		// newProvider -> preloadCode(); NO Instance super-call (pin).
		/*override*/ virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);

		// mac: 0x1810B0 (batch763:308-323): change -> restartScript() + raisePropertyChanged
		bool getDisabled() const { return disabled; }
		void setDisabled(bool value);

		// mac: 0x18104A (batch763:288-305): ContentId-assign + badLinkedScript=false +
		// restartScript() + raisePropertyChanged(prop_SourceCodeId)
		const ContentId& getScriptId() const { return scriptId; }
		void setScriptId(const ContentId& value);

		// class statics (InitFunc_122 TU): props
		// "Disabled" / "Behavior" (STANDARD, Security::None)
		static const Reflection::PropDescriptor<BaseScript, bool> prop_Disabled;
		// "LinkedSource" / "Data" (STANDARD, Security::None)
		static const Reflection::PropDescriptor<BaseScript, ContentId> prop_SourceCodeId;

		// mac: 0x1810F4-ctor members (batch768 disasm order):
		// scriptId @+92; workspace @+100; localPlayer weak @+104-111; disabled @+112;
		// badLinkedScript @+114; unknown115 @+115 (=1; the requestCode
		// verifyRequestedScriptSignature consumed it); threadNode @+116; starting(+120)
		// and stopped signals.
		ContentId scriptId;
		RuntimeScriptService* workspace;		// the run-tracking RSS (RuntimeScriptService*)
		boost::weak_ptr<Instance> localPlayer;	// slot @+104 (reader site not yet landed)
		bool disabled;
		bool badLinkedScript;
		bool unknown115;	// ctor writes 1; consumed by requestCode's signature-verify flag
		rbx::signal<void(lua_State*)> starting;
		rbx::signal<void()> stopped;
		boost::intrusive_ptr<Lua::WeakThreadRef::Node> threadNode;
	};

} // namespace RBX
