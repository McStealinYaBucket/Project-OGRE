/* Util/standardout.h -- October 2013 reconstruction.
   The process-wide output channel (the Output window and logs listen on
   messageOut). iOS 10-29 has StandardOut's ctor and dtor, singleton(),
   allowPrintWarnings, print_exception(function0<void> const&, MessageType,
   bool), printf(MessageType, char const*, ...) and the three print overloads
   (std::string const&, char const*, std::exception const&).
   Members in the order the .cpp constructs them: the mutex, then the signal.
   MESSAGE_SENSITIVE is kept for standardout.cpp and AsyncHttpQueue.cpp; no
   2013 string confirms or rules it out. */
#pragma once

#include <string>
#include <time.h>

#include "rbx/signal.h"
#include "boost/enable_shared_from_this.hpp"
#include "RbxFormat.h"

namespace RBX {

typedef enum
{
	MESSAGE_OUTPUT,
	MESSAGE_INFO,
	MESSAGE_WARNING,
	MESSAGE_ERROR,
	MESSAGE_SENSITIVE,
	MESSAGE_TYPE_MAX
} MessageType;

// One line of output, stamped with the wall-clock time it was posted.
struct StandardOutMessage
{
	MessageType type;
	std::string message;
	time_t time;

	StandardOutMessage() : type(MESSAGE_OUTPUT) {}
	StandardOutMessage(MessageType t, const char* text) : type(t), message(text) { ::time(&time); }
};

class StandardOut : public boost::enable_shared_from_this<StandardOut>
{
	boost::mutex sync;

	StandardOut() {} // only singleton() creates it

public:
	static shared_ptr<StandardOut> singleton();

	static bool allowPrintWarnings; // false drops MESSAGE_WARNING

	rbx::signal<void(const StandardOutMessage&)> messageOut;

	// Runs f; if it throws, prints the exception as `type` and rethrows on request.
	static void print_exception(const boost::function0<void>& f, MessageType type, bool rethrow);

	void print(MessageType type, const std::string& text);
	void print(MessageType type, const char* text);
	void print(MessageType type, const std::exception& e);
	void printf(MessageType type, const char* fmt, ...) RBX_PRINTF_ATTR(3, 4);
};

} // namespace RBX
