/* App/v8datamodel/Visit.cpp — reconstructed for October 2013 (first pass).
   Evidence: symbolled Studio 0.126 decompilation
     - DataModel::canSave 0x2D257A / saveToRoblox 0x2D01BA / requestShutdown 0x2D2CBA
       all use ServiceProvider::find<Visit> + read uploadUrl at Visit+96.
   The reflection block (SetUploadUrl/GetUploadUrl/SetPing, Security::Roblox) and the
   ping-thread bodies are carried over from March 2016 source (style template);
   TODO(decompile): verify against the 2013 Visit methods from the binary.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Visit.cpp */

#include "stdafx.h"

#include "V8DataModel/Visit.h"
#include "Util/StandardOut.h"
#include "Util/Http.h"

#include <boost/thread/xtime.hpp>

namespace RBX {
	const char* const sVisit = "Visit";
}

using namespace RBX;

REFLECTION_BEGIN();
// Still used
static Reflection::BoundFuncDesc<Visit, void(std::string)> UploadUrlFunction(&Visit::setUploadUrl, "SetUploadUrl", "url", Security::Roblox);
static Reflection::BoundFuncDesc<Visit, std::string()> getUploadUrl(&Visit::getUploadUrl, "GetUploadUrl", Security::Roblox);
static Reflection::BoundFuncDesc<Visit, void(std::string, int)> desc_setPing(&Visit::setPing, "SetPing", "pingUrl", "interval", Security::Roblox);
REFLECTION_END();


Visit::Visit()
	: uploadUrl("")
{
	setName("Visit");
}

Visit::~Visit()
{
}

void Visit::setUploadUrl(std::string value)
{
	uploadUrl = value;
}

worker_thread::work_result Visit::ping(std::string url, int interval)
{
	try
	{
		std::string response;
		RBX::Http(url).get(response);
	}
	catch (RBX::base_exception& /*exp*/)
	{
	}

	// TODO: use a timed condition so that interruption can be instantaneous
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC_);
	xt.sec += interval;
	boost::thread::sleep(xt);

	return RBX::worker_thread::more;
}

void Visit::setPing(std::string url, int interval)
{
	if (url!="")
	{
		// Create a new thread
		pingThread.reset(new worker_thread(
			boost::bind(&Visit::ping, url, interval),
			"rbx_visit"));
	}
	else
		// Just stop the current ping thread:
		pingThread.reset();
}

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Visit);
}
