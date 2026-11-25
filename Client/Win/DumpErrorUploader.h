/* Win/DumpErrorUploader.h -- October 2013 client shell. No 2013 __FILE__ string names this file
   (nothing in it asserts), so the path is the 2016 tree's (Win/DumpErrorUploader.cpp).
   Only the crash-event upload and its statics are written. Their dynamic initialisers (PLAYER
   0x00a5b3d0.., one TU) run in the order crashEventRequest, crashEventData, crashEventResponse;
   2016's crashCounterNamePrefix does not exist yet. 2016's ErrorUploader base and the dump
   upload itself are not written. */

#pragma once

#include <sstream>
#include <string>

#include <boost/scoped_ptr.hpp>

namespace RBX
{
	class Http;
}

class DumpErrorUploader
{
public:
	static void UploadCrashEventFile(struct _EXCEPTION_POINTERS* info = NULL);

	static boost::scoped_ptr<RBX::Http> crashEventRequest;
	static std::istringstream crashEventData;
	static std::string crashEventResponse;
};
