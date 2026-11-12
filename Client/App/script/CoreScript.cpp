/* App/script/CoreScript.cpp -- October 2013 (full C1 port: admin-substituted core scripts).
   Recon evidence (batches 730-771, MAC dbq24; see V8DataModel/CoreScript.h header):
     ctor 0x15F8B4 (batch768:234-268), onServiceProvider 0x15FA22
     (batch738:898-975 / batch745:34-100), requestCode 0x15FB68
     (batch738:994-1128), extraErrorReporting 0x1601A6 (batch769:1561-1691).
   Assert texts byte-verified from the ReleaseAssert strings:
     "sc" (script/CoreScript.cpp:33), "sc->hasScript(this)" (:34),
     "source.get() == this" (:95).
   2013 voltages preserved: MESSAGE_WARNING = printf type 2, MESSAGE_INFO = 1,
     "Missing substitution:", "Substituting \"\" with \"\"", the three-literal
     find blacklist lives in Script::setEmbeddedCode (not here). */

#include "stdafx.h"

#include "V8DataModel/CoreScript.h"
#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/DataModel.h"

#include "Util/standardout.h"
#include "Util/FileSystem.h"
#include "rbx/Log.h"
#include "lua/Lua.hpp"	// RBX::Lua::safe_lua_tostring (extraErrorReporting)

#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

namespace RBX {
	const char* const sCoreScript = "CoreScript";
}

using namespace RBX;

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::CoreScript);
}

namespace RBX {

	// mac: 0x15F8B4 (batch768:234-268): BaseScript() -> *(WORD*)(this+128) = 0
	// (CoreScript's own missing/reported pair) -> setName("CoreScript") ->
	// prop_SourceCodeId.setValue(this, contentId) (2013 went through the getset
	// virtual; the port calls the bound prop directly) -> setRobloxLocked(this, true).
	// FUNCTION: PLAYER 0x008aa560
	// FUNCTION: STUDIO 0x00992be0
	CoreScript::CoreScript(const ContentId& contentId)
		: missing(false)
		, reported(false)
	{
		setName("CoreScript");
		BaseScript::prop_SourceCodeId.setValue(this, contentId);
		setRobloxLocked(true);
	}

	CoreScript::~CoreScript()
	{
	}

	// mac: 0x15FA22 (batch738:898-975): old && !workspace -> the asserts + removeScript;
	// BaseScript::onServiceProvider runs in BOTH arms.
	// STUB: PLAYER 0x008aa420
	// STUB: STUDIO 0x00992aa0
	void CoreScript::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		if (oldProvider != NULL && workspace == NULL)
		{
			ScriptContext* scriptContext = ServiceProvider::find<ScriptContext>(oldProvider);
			RBXASSERT(scriptContext);						/* "sc" @ script/CoreScript.cpp:33 */
			RBXASSERT(scriptContext->hasScript(this));		/* "sc->hasScript(this)" @ :34 */
			scriptContext->removeScript(this);
		}
		BaseScript::onServiceProvider(oldProvider, newProvider);
	}

	// mac: 0x15FB68 (batch738:994-1128). DELTA note: the port keeps the 2013
	// filesystem::detail::status(...) <= 1 form verbatim (file-not-found /
	// the arm's own sense), so a not-locatable substitution re-enters the
	// BaseScript fetch path exactly once (missing latches).
	// STUB: PLAYER 0x008aa930
	// STUB: STUDIO 0x00992fc0
	BaseScript::Code CoreScript::requestCode(ScriptInformationProvider* sip)
	{
		if (BaseScript::adminScriptsPath.empty() || missing)
			return BaseScript::requestCode(sip);

		boost::filesystem::path filePath = boost::filesystem::initial_path();
		filePath /= BaseScript::adminScriptsPath;
		std::string fileName = getName();
		fileName.append(".lua");
		filePath /= fileName;
		std::string pathString = filePath.string();

		// (boost 1.53: the v2-era "(int)file_status" cast is gone; type()
		// 0 = status_error/unknown, 1 = file_not_found -- same <= 1 test.)
		boost::filesystem::file_status fileStatus = boost::filesystem::detail::status(filePath, 0);
		if (fileStatus.type() <= 1)
		{
			missing = true;
			StandardOut::singleton()->printf(MESSAGE_WARNING, "Missing substitution: \"%s\"", pathString.c_str());
			reported = true;
			return BaseScript::requestCode(sip);
		}

		if (reported == false)
		{
			StandardOut::singleton()->printf(MESSAGE_INFO, "Substituting \"%s\" with \"%s\"", getFullName().c_str(), pathString.c_str());
			reported = true;
		}

		std::stringstream buffer;
		std::ifstream file(pathString.c_str());
		buffer << file.rdbuf();
		file.close();

		std::string content = buffer.str();
		return Code(true, ProtectedString::fromTrustedSource(content));
	}

	// mac: 0x1601A6 (batch769:1561-1691)
	// STUB: PLAYER 0x008aa090
	// STUB: STUDIO 0x00992710
	void CoreScript::extraErrorReporting(lua_State* thread)
	{
		std::stringstream report;
		const char* message = Lua::safe_lua_tostring(thread, -1);
		if (message != NULL && *message != '\0')
			report << message;
		else
			report << "Error occurred, no output from Lua.";
		report << "\n\nCallStack:\n";

		boost::shared_ptr<BaseScript> source;
		int line = -1;
		report << ScriptContext::extractCallStack(thread, source, line);

		RBXASSERT(source.get() == this);				/* "source.get() == this" @ script/CoreScript.cpp:95 */

		DataModel* dataModel = DataModel::get(this);
		if (dataModel != NULL)
		{
			report << "\nPlaceID: ";
			report << dataModel->getPlaceID();
		}
		else
		{
			report << "\nError finding PlaceID!";
		}

		std::string content = report.str();

		std::stringstream pathStream;
		std::string userDirectory;
		FileSystem::getUserDirectory(userDirectory);
		pathStream << userDirectory;
		pathStream << getName();
		pathStream << "_ln";
		pathStream << line;
		pathStream << "_";
		pathStream << ".cse";

		std::string fileName = pathStream.str();

		std::ofstream file;
		file.open(fileName.c_str());	// OGRE vc9: open(std::string) is C++11
		if (file.is_open())
		{
			RBX::Log::timeStamp(file, true);
			file << content;
		}
		file.close();
	}

} // namespace RBX
