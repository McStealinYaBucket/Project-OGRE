/* Util/utilities.h -- October 2013 reconstruction.

   Small shared helpers:
   - rot13 (iOS 10-29: RBX::rot13(std::string), by value) and isCamel,
     both defined in App/util/Utilities.cpp;
   - StringConverter<T>, specialised per type in Utilities.cpp;
   - copy_on_write_ptr<T>, a shared_ptr that clones on write when shared.

   2013 evidence for layout: copy_on_write_ptr's two dereference asserts are
   recorded in PLAYER and STUDIO as "object.get() != 0 ... Util/utilities.h"
   at lines 65 (operator*) and 71 (operator->). They stay on those lines.

   Dropped: the 2016 sha1() declaration (no 2013 symbol, no caller, no
   definition here) and the C++11 explicit operator bool branch, which VC9
   never takes (boost 1.53 defines BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
   for it), so the safe-bool conversion below is the only one compiled. */
#ifndef RBX_UTIL_UTILITIES_H
#define RBX_UTIL_UTILITIES_H

#include <functional>
#include <algorithm>
#include <string>
#include "rbx/boost.hpp"
#include "rbx/Debug.h"

namespace RBX {

std::string rot13(std::string text);

bool isCamel(const char* name);

// Text <-> value conversion; each supported T has its own specialisation.
template <typename T>
class StringConverter
{
public:
	static std::string convertToString(const T& value);
	static bool convertToValue(const std::string& text, T& value);
};

// Shares one T between copies until someone writes; not thread-safe.
template <class T>
class copy_on_write_ptr : public boost::noncopyable
{
	mutable boost::shared_ptr<T> object;

public:
	copy_on_write_ptr() {}                           // empty
	copy_on_write_ptr(const T& v) : object(new T(v)) {}  // owns a copy of v

	// True once something has been stored. boost's safe-bool idiom: the
	// conversion yields shared_ptr's unspecified_bool_type (a member
	// pointer), which tests in if() and ! but does not convert to int,
	// so a copy_on_write_ptr cannot be added or compared by accident.
	// Converting the shared_ptr itself does the work.
	operator typename boost::shared_ptr<T>::unspecified_bool_type() const
	{
		return object;
	}

	// Short-lived read access; do not keep the reference.
	// Both assert on an empty pointer.
	const T& operator*() const
	{
		RBXASSERT(object.get() != 0);
		return *object;
	}

	const T* operator->() const
	{
		RBXASSERT(object.get() != 0);
		return object.get();
	}

	// Read-only snapshot; may be empty.
	boost::shared_ptr<const T> read() const { return object; }

	// Write access: creates the object if empty, clones it if shared.
	// The returned reference is only good until the next write().
	boost::shared_ptr<T>& write()
	{
		if (!object)
			object = boost::shared_ptr<T>(new T());
		else if (object.use_count() > 1)
			object = boost::shared_ptr<T>(new T(*object));
		return object;
	}

	void reset() { object.reset(); }
};

} // namespace RBX

#endif
