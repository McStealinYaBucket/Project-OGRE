// Guid.cpp -- reconstructed from October 2013 binaries.
// Source of truth: symbolled Mach-O Studio binary (0.126.0.39503), EAs noted per function.
// The Mach-O decompilation was built for __APPLE__ (CFUUID path in generateStandardGUID,
// boost::algorithm::to_lower); the Windows branch below is the equivalent MSVC path from the
// same source file (2016 reference kept where the binary agrees).

#include "Util/Guid.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/thread/once.hpp>
#include <rbx/atomic.h>

#ifdef _WIN32
#include "objbase.h"
#endif

namespace RBX {
	template<>
	bool StringConverter<RBX::Guid::Data>::convertToValue(const std::string& text, RBX::Guid::Data& value)
	{
		RBXASSERT(false); // not fully implemented
		return false;
	}
}

// mac: 0x204336: Guid::Guid does boost::call_once(&unk_FAE0EC guard, sub_204291) then
// *this = dword_FAE0F0 (localScope) and index = RbxInterlockedIncrementAcquire(&dword_FAE0F4).
// I.e. data.scope = getLocalScope(); data.index = ++nextIndex; with a function-local
// once-init of the local scope.
static rbx::atomic<int> nextIndex = 0;
static RBX::Guid::Scope* localScope;
RBX::Guid::Scope RBX::Guid::Scope::nullScope;

const RBX::Guid::Scope& RBX::Guid::Scope::null()
{
	return nullScope;
}

static void initLocalScope()
{
	RBX::Guid::Scope scope;
	RBX::Guid::generateRBXGUID(scope);

	// Note: localScope has to be a pointer to avoid initialization order fiasco between
	// localScope and getLocalScope() callers. We never free this memory (see SAFE_HEAP_STATIC
	// in rbx/threadsafe.h).
	localScope = new RBX::Guid::Scope(scope);
}

const RBX::Guid::Scope& RBX::Guid::getLocalScope()
{
	// __cxa_guard_acquire pattern in the binary => function-local static / call_once
	static boost::once_flag flag = BOOST_ONCE_INIT;
	boost::call_once(&initLocalScope, flag);
	return *localScope;
}

// mac: 0x204336
RBX::Guid::Guid()
{
	data.scope = getLocalScope();
	data.index = ++nextIndex;
}

// mac: 0x204380 -- Mach-O build: CFUUIDCreate/CFUUIDCreateString, wrap in "{}", boost::to_lower.
// (Windows build of the same source uses CoCreateGuid + StringFromGUID2.)
void RBX::Guid::generateStandardGUID(std::string& result)
{
	// Creates a string like this: {c200e360-38c5-11ce-ae62-08002b2b79ef}
#ifdef _WIN32
	::GUID guid;
	CoCreateGuid(&guid);

	// This string will hold a string-ized GUID.  It needs
	// to be 38 characters, plus the terminator
	// character.  But just to be safe we'll make
	// it a little bigger.
	WCHAR wszGUID[ 64 ] = { L"" };
	StringFromGUID2( guid, wszGUID, 64 );

	char ansiClsid[64];
	WideCharToMultiByte(CP_ACP, 0, wszGUID, 64, ansiClsid, 64, 0, 0);

	result = ansiClsid;
#else
	CFUUIDRef uuid = CFUUIDCreate(0);
	CFStringRef uuidStrRef = CFUUIDCreateString(0, uuid);

	char szGUID[64];
	CFStringGetCString(uuidStrRef, &szGUID[0], sizeof(szGUID), kCFStringEncodingASCII);

	CFRelease(uuidStrRef);
	CFRelease(uuid);
	result = "{"; result.append(szGUID); result.append("}");

	boost::to_lower(result);
#endif
}

void RBX::Guid::generateRBXGUID(RBX::Guid::Scope& result)
{
	std::string tmp;
	generateRBXGUID(tmp);

	result.set( tmp );
}

// mac: 0x2044D0 -- generateStandardGUID, prepend "RBX", then erase indices
// 40,27,22,17,12,3 in the binary ("RBX{........-....-....-....-............}" is 41 chars;
// erasing 40,27,22,17,12 strips '}' and the four '-', erasing 3 strips '{').
void RBX::Guid::generateRBXGUID(std::string& result)
{
	// Start with a text character (so it conforms to xs:ID requirement)
	generateStandardGUID(result);
	result = "RBX" + result;

	// result looks like this: RBX{c200e360-38c5-11ce-ae62-08002b2b79ef}

	// strip the {}- characters
	result.erase(40, 1);
	result.erase(27, 1);
	result.erase(22, 1);
	result.erase(17, 1);
	result.erase(12, 1);
	result.erase(3, 1);

	// result looks like this: RBXc200e36038c511ceae6208002b2b79ef

	// TODO: This string could be more compact if we included g-z
}

// mac: 0x204618
void RBX::Guid::assign(Data data)
{
	this->data = data;
}

bool RBX::Guid::Data::operator ==(const RBX::Guid::Data& other) const
{
	return index == other.index && scope == other.scope;
}

// mac: 0x20462A -- compares *(int**)this (the Name*) then this[1] (index)
bool RBX::Guid::Data::operator <(const RBX::Guid::Data& other) const
{
	const int compare = scope.compare(other.scope);

	if (compare < 0)
		return true;
	if (compare > 0)
		return false;

	return index < other.index;
}

// mac: 0x204658 -- null pointers fall back to Name::getNullName() / index 0
int RBX::Guid::compare(const Guid* a, const Guid* b)
{
	const Scope& sa = a ? a->data.scope : Scope::null();
	const Scope& sb = b ? b->data.scope : Scope::null();

	const int compare = sa.compare(sb);
	if (compare)
		return compare;
	int ia = a ? a->data.index : 0;
	int ib = b ? b->data.index : 0;
	return ia - ib;
}

// mac: 0x2046AC
int RBX::Guid::compare(const Guid* a0, const Guid* a1, const Guid* b0, const Guid* b1)
{
	int aComp = compare(a0, a1);
	int bComp = compare(b0, b1);

	const Guid* aMax = (aComp > 0) ? a0 : a1;
	const Guid* bMax = (bComp > 0) ? b0 : b1;

	const int compare = Guid::compare(aMax, bMax);
	if (compare)
		return compare;

	const Guid* aMin = (aComp > 0) ? a1 : a0;
	const Guid* bMin = (bComp > 0) ? b1 : b0;

	return Guid::compare(aMin, bMin);
}

#pragma warning(push)
#pragma warning(disable: 4996) //  warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

// mac: 0x204720 -- sprintf("%s_%d", scopeName.substr(min(3,len), min(scopeLength,32)), index);
// or sprintf("%d", index) when scopeLength <= 0.
// FUNCTION: PLAYER 0x006f95f0
// FUNCTION: STUDIO 0x007733f0
std::string RBX::Guid::Data::readableString(int scopeLength) const
{
	char result[64];
	if (scopeLength>0)
	{
		std::string s = scope.getName()->str;
		s = s.substr(std::min((size_t)3,s.size()), std::min(scopeLength,32));
		sprintf(result, "%s_%d", s.c_str(), index);
	}
	else
		sprintf(result, "%d", index);
	return result;
}

#pragma warning(pop)

// TODO(not this module, kept as evidence notes from Util_Guid.cpp.txt):
// - RBX::GuidItem<RBX::Instance>::Registry::unregister 0x5470D6, GuidItem ctor 0x54667E,
//   dtor 0x546736, onGuidChanged 0x4798F0 -- template instantiations, realized in Guid.h.
// - Registry dtor assert "map.size() ==0" references ../App/include/Util/Guid.h line 89.
// - RBX::Primitive::setGuid 0x58FE46 (v8world/Primitive.cpp:152) -- belongs to Primitive.
// - RBX::Network::IdSerializer::serializeId/deserializeId/deserializeIdWithoutDictionary/
//   resolvePendingReferences/deserializeInstanceRef/addPendingRef (0x83FDEE..0x84046E),
//   ClientReplicator/ServerReplicator/Replicator functions, unregisterHelper 0x84E386,
//   Instance::visitDescendants / boost::bind instantiations, and the std::_Rb_tree /
//   std::map<Guid::Data,...> / boost::shared_ptr template out-of-line copies are all
//   instantiations of other modules' code and are not emitted here.
