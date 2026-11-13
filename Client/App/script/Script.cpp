/* App/v8datamodel/Script.cpp -- October 2013 (full C1 port).
   Recon evidence: see V8DataModel/Script.h header comment (batches 730-771, MAC dbq24).
   Verbatim sources: Script ctor 0x1812DC (batch768 disasm), requestCode 0x182D00
   (batch742:216-257), preloadCode 0x18290E (batch768:141-160), requestHash 0x182DE2
   (batch768:170-179), isRobloxScript 0x182DA4 (batch742:280-294), setEmbeddedCode
   0x180F08 (batch747:468-504), getEmbeddedCodeSafe 0x180E52 (batch747:507-535),
   getHashSlow 0x182F18 (batch745:397+), initStatics sub_1814B5 (batch763:218-249,
   literal "print 'Hello world!'\r\n"), ES setCode 0x181790 (batch742:344 /
   batch770:297-320), ES CalculateHash (batch770:329-342), ES embeddedCodeIsReady
   0x1821EC (batch745:411 / batch770:347-422), ES requestEmbeddedCode 0x182446
   (batch770:426-499). */

#include "stdafx.h"

#include "V8DataModel/Script.h"
#include "V8DataModel/ScriptInformationProvider.h"
#include "V8DataModel/DataModelJob.h"
#include "V8DataModel/DataModel.h"	// requestEmbeddedCode's submitTask re-question arm

#include "V8Tree/Service.h"
#include "Network/Players.h"
#include "Util/MD5Hasher.h"
#include "Util/DFFlag.h"

#include <boost/bind.hpp>

namespace RBX {
	const char* const sScript = "Script";
}

using namespace RBX;

namespace {

	// InitFunc_122 sentinels (sub_1814B5, batch763:218-249). 2013 stored these as
	// boost::flyweights (dword_FADA18 = default, dword_FADA1C = empty); the port
	// keeps plain ProtectedStrings and compares by value (deliberate DELTA).
	ProtectedString s_emptyScriptCode;
	ProtectedString s_defaultScriptCode;
	boost::once_flag s_staticsOnce = BOOST_ONCE_INIT;

	void initStatics()
	{
		s_defaultScriptCode = ProtectedString::fromTrustedSource("print 'Hello world!'\r\n");
		// s_emptyScriptCode: default-constructed by static initialization
		// (sub_1814B5 assigns ProtectedString() into dword_FADA1C).
	}

} // anonymous namespace

REFLECTION_BEGIN();
const Reflection::PropDescriptor<Script, ProtectedString> Script::prop_EmbeddedSourceCode("Source", category_Data, &Script::getEmbeddedCodeSafe, &Script::setEmbeddedCode, Reflection::PropertyDescriptor::STANDARD, Security::RobloxPlace);
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Script);
}

namespace RBX {

	std::string BaseScript::adminScriptsPath;	// definition (2016 home; InitFunc_122 TU)

	const ProtectedString& Script::emptyScriptCode()
	{
		return s_emptyScriptCode;
	}

	const ProtectedString& Script::getDefaultScriptCode()
	{
		boost::call_once(&initStatics, s_staticsOnce);
		return s_defaultScriptCode;
	}

	// mac: 0x182DA4 (batch742:280-294)
	// STUB: PLAYER 0x006692c0
	// STUB: STUDIO 0x0077a1b0
	bool Script::isRobloxScript()
	{
		if (scriptId.isNull())
		{
			if (embeddedSource.ready)
				return embeddedSource.unknown135;
			return false;
		}
		return true;
	}

	Script::~Script()
	{
	}

	// mac: 0x182DE2 (batch768:170-179): reference return.
	// STUB: PLAYER 0x00669300
	// STUB: STUDIO 0x0077a1f0
	const std::string& Script::requestHash()
	{
		if (scriptId.isNull())
			return embeddedSource.hash;
		return BaseScript::emptyString;
	}

	// mac: 0x182E06 (batch769:1159-1180, eighth era): the persistent-cost payload is
	// the code string, counted only for embedded scripts (scriptId empty --
	// linked scripts carry no source payload). ready&&!fresh reads the empty
	// static PS (the getEmbeddedCodeSafe guard arm, dword_FADA1C landed as
	// ProtectedString::emptyString); otherwise embeddedSource.source. The
	// length adder is Instance::computeStringCost (selects >= 1).
	// STUB: PLAYER 0x0066a510
	// STUB: STUDIO 0x0077b2f0
	int Script::getPersistentDataCost() const
	{
		int cost = Instance::getPersistentDataCost();
		if (scriptId.isNull())
		{
			const ProtectedString* ps = &embeddedSource.source;
			if (embeddedSource.ready && !embeddedSource.fresh)
				ps = &ProtectedString::emptyString;
			cost += computeStringCost(ps->getStringForImmediateUse());
		}
		return cost;
	}

	// mac: 0x18290E (batch768:141-160)
	// STUB: PLAYER 0x0066d500
	// STUB: STUDIO 0x0077e2b0
	void Script::preloadCode()
	{
		if (scriptId.isNull()
			&& Network::Players::frontendProcessing(this)
			&& Network::Players::backendProcessing(this))
		{
			requestCode(NULL);	/* result discarded (2013 flyweight warm-up; port discards the plain copy) */
		}
		else
		{
			BaseScript::preloadCode();
		}
	}

	// mac: 0x180F08 (batch747:468-504): three-literal blacklist (values below are the
	// binary's exact find arguments, lengths 15/48/62 -- each literal is verbatim).
	// STUB: PLAYER 0x0066e580
	// STUB: STUDIO 0x0077f2e0
	void Script::setEmbeddedCode(const ProtectedString& code)
	{
		std::string immediate = code.getStringForImmediateUse();
		if (immediate.find("Death to roblox") == std::string::npos
			&& immediate.find("string.reverse(calco[2] .. calco[3] .. calco[16]") == std::string::npos
			&& immediate.find("fade playername --this fades the player named playername a bit") == std::string::npos)
		{
			if (!(embeddedSource.source == code))
			{
				embeddedSource.setCode(code, false);
				restartScript();
				raisePropertyChanged(prop_EmbeddedSourceCode);
			}
		}
		else
		{
			embeddedSource.ready = true;
			embeddedSource.fresh = false;
		}
	}

	// mac: 0x182F18 ("GetHash" BoundFuncDesc, InitFunc_122)
	std::string Script::getHashSlow()
	{
		return scriptId.isNull() ? embeddedSource.hash : std::string();
	}

	// mac: 0x180E52 (batch747:507-535). Port comment: getter const-marked for the
	// reflection GetSet; CalculateHash is const in the port (loads only).
	const ProtectedString& Script::getEmbeddedCodeSafe() const
	{
		if (embeddedSource.ready && !embeddedSource.fresh)
			return ProtectedString::emptyString;

		if (!Network::Players::clientIsPresent(this) || embeddedSource.source == getDefaultScriptCode())
			return embeddedSource.source;

		std::string calcHash = embeddedSource.CalculateHash();
		if (calcHash.compare(embeddedSource.hash) != 0)
			return ProtectedString::emptyString;
		return embeddedSource.source;
	}

	// mac: 0x1812DC (batch768 disasm): BaseScript() -> Described fixes ->
	// setName("Script") -> boost::call_once(initStatics) -> embeddedSource.setCode(
	// getDefaultScriptCode(), true). (GCC ctor also left ES.requested unzeroed --
	// port value-inits it false, LOUD DELTA.)
	// STUB: PLAYER 0x0066e970
	// STUB: STUDIO 0x0077f6d0
	Script::Script()
	{
		setName("Script");
		boost::call_once(&initStatics, s_staticsOnce);
		embeddedSource.setCode(getDefaultScriptCode(), true);
	}

	// Instance.h:756 signature; 2013 guard body (checkpoint decode): an Embedded
	// source without a fresh hash serializes to nothing.
	XmlElement* Script::writeXml(const boost::function<bool(Instance*)>& isInScope, RBX::CreatorRole creatorRole)
	{
		if (embeddedSource.ready && !embeddedSource.fresh)
			return NULL;
		return Instance::writeXml(isInScope, creatorRole);
	}

	bool Script::askSetParent(const Instance* /*instance*/) const
	{
		return true;
	}

	// mac: 0x182D00 (batch742:216-257)
	// STUB: PLAYER 0x0066ebe0
	// STUB: STUDIO 0x0077f940
	BaseScript::Code Script::requestCode(ScriptInformationProvider* sip)
	{
		if (!scriptId.isNull())
			return BaseScript::requestCode(sip);

		ScriptInformationProvider* localSip = sip;
		if (localSip == NULL)
			localSip = ServiceProvider::create<ScriptInformationProvider>(this);

		if (localSip != NULL && embeddedSource.embeddedCodeIsReady(localSip))
			return Code(true, embeddedSource.requestEmbeddedCode(localSip, this));

		return Code(false, emptyScriptCode());
	}

	// ------------------------------------------------------------------
	// EmbeddedSource
	// ------------------------------------------------------------------

	// batch768 ctor disasm (Script+128 block): source default; ready/fresh/
	// unknown135 zeroed; requested NOT zeroed in 2013 (.Glide DELTA above);
	// hash default-empty.
	Script::EmbeddedSource::EmbeddedSource()
		: ready(false)
		, requested(false)
		, fresh(false)
		, unknown135(false)
	{
	}

	// mac: 0x181790 (batch742:344 / batch770:297-320)
	void Script::EmbeddedSource::setCode(const ProtectedString& code, bool updateHash)
	{
		source = code;		// 2013: flyweight-handle copy, ++refcount; port: plain assignment
		ready = updateHash;		// @+4 = a3
		fresh = updateHash;		// @+6 = a3
		requested = false;		// @+5 = 0
		hash = CalculateHash();	// (0x181D12: assign from CalculateHash)
	}

	// MD5 (batch770:329-342): MD5Hasher::create() -> addData(getStringForImmediateUse)
	// -> toString(); hasher deleted.
	std::string Script::EmbeddedSource::CalculateHash() const
	{
		MD5Hasher* hasher = MD5Hasher::create();
		hasher->addData(source.getStringForImmediateUse());
		std::string result = hasher->toString();
		delete hasher;
		return result;
	}

	// mac: 0x1821EC (batch745:411 / batch770:347-422)
	// LOUD DELTA (SIP-era): for !ready scripts with a provider, the 2013 bound a
	// weak DataModel + weak Script + the current hash into a finish callback
	// routed through SaveScriptInfoHelper / EmbeddedSource::saveResult
	// (0x181D86 / 0x1821A2) and registered it with the provider; the exact bind /
	// mark-up is SIP-era deferred, so the C1 port performs the request and
	// discards the callback slot.
	bool Script::EmbeddedSource::embeddedCodeIsReady(ScriptInformationProvider* sip)
	{
		if (source == Script::emptyScriptCode() || source == Script::getDefaultScriptCode())
			return true;

		if (!ready)
		{
			if (sip != NULL)
			{
				if (!requested)
				{
					requested = true;
					bool accurateFlag = DFFlag::GetAccurateScriptInfoResultsFromScriptContext;
					typedef boost::function<void(ScriptInformationProvider::RequestResult, const std::string&, bool, bool, bool)> ScriptInfoResult;
					sip->getScriptInfo(hash, accurateFlag, false, ScriptInfoResult(), false);
				}
			}
			else
			{
				ready = true;
				fresh = true;
			}
		}

		return ready;
	}

	namespace {
		// requestEmbeddedCode's hash-changed re-question target (0x182446's
		// submitTask arm bound a weak<DataModel> to a void(weak<DataModel>)
		// static); its body is SIP-era deferred (LOUD DELTA).
		void reRequestScriptInfo(boost::weak_ptr<DataModel> dataModel)
		{
			(void)dataModel;
		}
	}

	// mac: 0x182446 (batch770:426-499)
	const ProtectedString& Script::EmbeddedSource::requestEmbeddedCode(ScriptInformationProvider* sip, Script* script)
	{
		if (source != Script::emptyScriptCode() && source != Script::getDefaultScriptCode())
		{
			if (embeddedCodeIsReady(sip) && fresh)
			{
				std::string calcHash = CalculateHash();
				if (calcHash.compare(hash) != 0)
				{
					// hash changed underneath: re-question through the DataModel
					// (Write task; decompile 476: submitTask(bind(...), 1 = Write))
					// and return the empty sentinel (binary :490).
					boost::shared_ptr<DataModel> dataModel = shared_from_dynamic_cast<DataModel>(script);
					if (dataModel)
					{
						dataModel->submitTask(
							boost::bind(&reRequestScriptInfo, boost::weak_ptr<DataModel>(dataModel)),
							DataModelJob::Write);
					}
					return Script::emptyScriptCode();
				}
			}
			else
			{
				return Script::emptyScriptCode();	// binary :495
			}
		}
		return source;
	}

	// mac: 0x1821A2 (batch763:453-475, verbatim)
	void Script::EmbeddedSource::saveResult(ScriptInformationProvider::RequestResult requestResult, const std::string& resultHash, bool flagOne, bool flagTwo, bool flagThree)
	{
		(void)flagTwo;	/* a5 unread in the binary body */
		if (hash.compare(resultHash) == 0)
		{
			if ((requestResult & 0xFFFFFFFD) == 1)
			{
				fresh = !flagOne;
				unknown135 = flagThree;
			}
			else
			{
				fresh = true;
			}
			ready = true;
		}
	}

	// mac: 0x181D86 -- mangle verified; decompile not restored in batches 7xx.
	// SIP-era LOUD DELTA: no-op until the SIP-era landing models the
	// DataModel/Script application path (its result consumer is saveResult).
	void Script::SaveScriptInfoHelper(
		boost::weak_ptr<DataModel> dataModel,
		boost::weak_ptr<Script> script,
		std::string hash,
		ScriptInformationProvider::RequestResult requestResult,
		bool flagOne,
		bool flagTwo,
		bool flagThree)
	{
		(void)dataModel;
		(void)script;
		(void)hash;
		(void)requestResult;
		(void)flagOne;
		(void)flagTwo;
		(void)flagThree;
	}

} // namespace RBX
