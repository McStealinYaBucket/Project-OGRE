/* V8DataModel/Script.h -- October 2013 (full: user Script + embedded source).
   Recon evidence (batches 730-771, MAC dbq24):
   - Class shape: FactoryProduct<Script, BaseScript> @0x18756C; sScript = "Script".
     Described flags from the ctor mangle: ClassDescriptor::Functionality 27 = PERSISTENT,
     Security::Permissions 0 (batch768; port default form kept = same effect).
   - Script ctor 0x1812DC (batch768 disasm): BaseScript() -> setName("Script") ->
     boost::call_once(&guard, initStatics) -> embeddedSource.setCode(getDefaultScriptCode(), true).
   - EmbeddedSource nested class (verified mangles: RBX::Script::EmbeddedSource::setCode /
     ::CalculateHash / ::embeddedCodeIsReady / ::requestEmbeddedCode; instance at Script+128):
       source ProtectedString @+0 (2013 boost::flyweight -- port keeps a plain value,
       DELTA: sentinels emptyScriptCode/defaultScriptCode compared by value below),
       ready @+4, requested @+5 (GCC ctor does NOT zero it -- port value-inits false,
       LOUD DELTA), fresh @+6, unknown135 @+7 (zeroed), hash std::string @+8.
     setCode 0x181790 (batch770:297-320): source=code; ready=fresh=updateHash;
     requested=0; hash=CalculateHash().
     CalculateHash (batch770:329-342): MD5Hasher::create() -> addData(
     source.getStringForImmediateUse()) -> toString(); hasher deleted.
     embeddedCodeIsReady 0x1821EC (batch770:347-422): source sentinels -> true; !ready
     gate; sip-fed branch requests ScriptInformationProvider::getScriptInfo with the
     hash and marks requested (SIP-era body -- LOUD DELTA keep-alive); no/empty sip ->
     ready=fresh=true; returns ready.
     requestEmbeddedCode 0x182446 (batch770:426-499): sentinel source -> source;
     embeddedCodeIsReady(sip) && fresh -> hash compare -- changed ->
     DataModel::submitTask re-question + return emptyScriptCode sentinel; not ready/fresh
     -> emptyScriptCode sentinel; else source.
   - Script::requestCode 0x182D00 (batch742:216-257): scriptId non-empty ->
     BaseScript::requestCode; else sip (arg or create<ScriptInformationProvider>(this));
     embeddedCodeIsReady -> Code(true, requestEmbeddedCode) else Code(false).
   - Script::preloadCode 0x18290E (batch768:141-160): scriptId empty &&
     Players::frontendProcessing && Players::backendProcessing -> requestCode(NULL)
     (flyweight warm-up, discarded); else BaseScript::preloadCode.
   - Script::requestHash 0x182DE2 (batch768:170-179): scriptId empty -> embeddedSource.hash
     else BaseScript::emptyString (REFERENCE return).
   - getHashSlow 0x182F18: scriptId empty -> embeddedSource.hash else std::string()
     ("GetHash" BoundFuncDesc, InitFunc_122).
   - writeXml guard: embeddedSource.ready && !fresh -> NULL, else Instance::writeXml;
   - askSetParent -> true.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Script.h */

#pragma once

#include "V8DataModel/BaseScript.h"
#include "V8DataModel/ScriptInformationProvider.h"
#include "Util/ProtectedString.h"

#include <string>

namespace RBX {

	extern const char* const sScript;

	class DataModel;

	class Script
		: public DescribedCreatable<Script, BaseScript, sScript>
	{
	public:
		Script();
		/*override*/ virtual ~Script();

		// mac: 0x182D00 (batch742:216-257)
		/*override*/ virtual Code requestCode(ScriptInformationProvider* sip);

		// mac: 0x18290E (batch768:141-160)
		/*override*/ virtual void preloadCode();

		/*override*/ virtual bool isRobloxScript();

		// mac: 0x182DE2 (batch768:170-179): reference return.
		/*override*/ virtual const std::string& requestHash();

		// Instance.h:756 signature; guard body in the .cpp.
		/*override*/ virtual XmlElement* writeXml(const boost::function<bool(Instance*)>& isInScope, RBX::CreatorRole creatorRole);
		// returns true (Instance.h:782).
		/*override*/ virtual bool askSetParent(const Instance* instance) const;

		// mac: 0x182E06 (batch769:1159-1180, eighth era): cost adds the code payload
		// only when scriptId is empty (embedded code, no linked id); the source
		// choice follows the writeXml/getEmbeddedCodeSafe guard (ready&&!fresh
		// reads the empty static PS -- landed as ProtectedString::emptyString,
		// the getEmbeddedCodeSafe arm), sized with Instance::computeStringCost
		// (max(1, len/100)).
		/*override*/ virtual int getPersistentDataCost() const;

		// mac: 0x182F18 ("GetHash" BoundFuncDesc, InitFunc_122)
		std::string getHashSlow();

		// mac: 0x181D86 (mangle __ZN3RBX6Script20SaveScriptInfoHelperE...; body
		// not restored in batches 7xx -- SIP-era LOUD DELTA stub in the .cpp):
		// the bound finish-callback slot for the SIP's script-info requests.
		static void SaveScriptInfoHelper(
			boost::weak_ptr<DataModel> dataModel,
			boost::weak_ptr<Script> script,
			std::string hash,
			ScriptInformationProvider::RequestResult requestResult,
			bool flagOne,
			bool flagTwo,
			bool flagThree);

		// "Source"/"Data" property (STANDARD, Security::RobloxPlace; InitFunc_122)
		// body = 0x180E52 verbatim (cpp): ready&&!fresh -> ProtectedString::emptyString;
		// !Players::clientIsPresent || source==default -> source; hash mismatch -> empty; else source.
		const ProtectedString& getEmbeddedCodeSafe() const;
		void setEmbeddedCode(const ProtectedString& code);

		// Statics (call_once-initialized; definitions in v8datamodel/Script.cpp).
		static const ProtectedString& emptyScriptCode();
		static const ProtectedString& getDefaultScriptCode();

		static const Reflection::PropDescriptor<Script, ProtectedString> prop_EmbeddedSourceCode;

		class EmbeddedSource
		{
		public:
			EmbeddedSource();

			// mac: 0x181790 (batch770:297-320)
			void setCode(const ProtectedString& code, bool updateHash);

			// MD5 of the immediate-use source (batch770:329-342). Port marks const
			// (loads only) so the const prop getter can call it; 2013 was non-const.
			std::string CalculateHash() const;

			// mac: 0x1821EC (batch770:347-422)
			bool embeddedCodeIsReady(ScriptInformationProvider* sip);

			// mac: 0x182446 (batch770:426-499)
			const ProtectedString& requestEmbeddedCode(ScriptInformationProvider* sip, Script* script);

			// mac: 0x1821A2 (batch763:453-475, verbatim): apply a provider result when
			// its hash matches the current one; (requestResult & ~2) == 1 ->
			// fresh = !flagOne, unknown135 = flagThree; else fresh = true;
			// ready = true either way.
			void saveResult(ScriptInformationProvider::RequestResult requestResult, const std::string& resultHash, bool flagOne, bool flagTwo, bool flagThree);

			// Instance offset @Script+128 (batch768 ctor disasm):
			ProtectedString source;	// @+0 -- 2013 flyweight; DELTA: plain value side
			bool ready;				// @+4
			bool requested;			// @+5 -- GCC ctor does NOT zero; port inits false (LOUD DELTA)
			bool fresh;				// @+6
			bool unknown135;		// @+7
			std::string hash;		// @+8
		};

		EmbeddedSource embeddedSource;
	};

} // namespace RBX
