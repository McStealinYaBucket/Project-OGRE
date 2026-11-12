/* V8DataModel/Visit.h — October 2013 (copied from March 2016 reference, verified).
   Evidence: symbolled Studio 0.126 decompilation
     - DataModel::canSave 0x2D257A calls ServiceProvider::find<Visit>(this) then reads
       a std::string at Visit+96 and returns length != 0. 2016 Visit::getUploadUrl()
       returns uploadUrl; canSave returns !visit->getUploadUrl().empty() — matches.
     - DataModel::saveToRoblox 0x2D01BA and requestShutdown 0x2D2CBA also read Visit+96
       (saveToRoblox checks find("http")==0 to decide the upload path).
     => Visit has a std::string uploadUrl as its first data member after the
        Instance+Service bases (binary reads it inline, consistent with the 2016 layout
        where uploadUrl is the first member).
   The ping thread / SetPing surface is carried over from 2016 (structure identical);
   TODO(decompile): verify Visit ctor + setPing bodies from the 2013 binary. */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

#include "rbx/boost.hpp"
#include <boost/thread/condition.hpp>

namespace RBX {
	namespace Network {
		class Player;
	}

	extern const char* const sVisit;

	class ModelInstance;
	class PartInstance;
	class Workspace;

	class Visit
		: public DescribedCreatable<Visit, Instance, sVisit, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	private:
		std::string uploadUrl;
		scoped_ptr<RBX::worker_thread> pingThread;

	public:
		Visit();
		~Visit();

		void setPing(std::string url, int interval);
		static worker_thread::work_result ping(std::string url, int interval);

		void setUploadUrl(std::string value);
		std::string getUploadUrl() { return uploadUrl; }
	};

} // namespace
