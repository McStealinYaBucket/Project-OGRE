/* Win/DumpErrorUploader.cpp -- October 2013 client shell (PLAYER and STUDIO compile it; path: see
   DumpErrorUploader.h). */

#include "DumpErrorUploader.h"

#include "Util/Http.h"
#include "Util/standardout.h"

// GLOBAL: PLAYER 0x00e25178
// GLOBAL: STUDIO 0x00fa5714
boost::scoped_ptr<RBX::Http> DumpErrorUploader::crashEventRequest;
// GLOBAL: PLAYER 0x00e25198
// GLOBAL: STUDIO 0x00fa5718
// DumpErrorUploader::crashEventData
std::istringstream DumpErrorUploader::crashEventData("Crash happened!");
// GLOBAL: PLAYER 0x00e2517c
// GLOBAL: STUDIO 0x00fa579c
std::string DumpErrorUploader::crashEventResponse;

// Posts the prepared crash event (MainLogManager's ProcessException calls it). 2013 has none of
// 2016's analytics counters or InfluxDb points, and catches everything.
// FUNCTION: PLAYER 0x00435f10
// FUNCTION: STUDIO 0x0041ab40
void DumpErrorUploader::UploadCrashEventFile(struct _EXCEPTION_POINTERS* excInfo)
{
	try
	{
		if (crashEventRequest)
		{
			RBX::StandardOut::singleton()->printf(RBX::MESSAGE_INFO, "Crash Event post");
			crashEventRequest->post(crashEventData, true, crashEventResponse, false);
			RBX::StandardOut::singleton()->printf(RBX::MESSAGE_INFO, "Crash Event posted, response: %s", crashEventResponse.c_str());
		}
	}
	catch (...)
	{
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "Exception during upload crash event");
	}
}
