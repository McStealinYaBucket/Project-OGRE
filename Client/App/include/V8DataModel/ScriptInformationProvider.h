/* V8DataModel/ScriptInformationProvider.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<ScriptInformationProvider> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "ScriptInformationProvider" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ScriptInformationProvider.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

#include <boost/function.hpp>
#include <string>

namespace RBX {

	extern const char* const sScriptInformationProvider;

	class ScriptInformationProvider
		: public DescribedNonCreatable<ScriptInformationProvider, Instance, sScriptInformationProvider, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		ScriptInformationProvider() {}

		// C1 additions (LOUD DELTA -- the 2013 async script-info path is only
		// reflected by the decoded ES call shape: 0x181D86 SaveScriptInfoHelper /
		// mac: 0x1821A2 EmbeddedSource::saveResult mangles; the parameter spellings of
		// the two trailing bools and the RequestResult values are unported):
		enum RequestResult { RR_Unknown = 0 };

		virtual void getScriptInfo(
			const std::string& hash,
			bool accurateFlag,
			bool flagTwo,
			const boost::function<void(RequestResult, const std::string&, bool, bool, bool)>& onFinish,
			bool memberFlag) {}
	};

} // namespace RBX
