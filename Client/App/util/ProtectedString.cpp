// ProtectedString.cpp - reconstructed from October 2013 binary.
// Class functions decompiled from the symbolled Mach-O Studio database.
// Everything else that showed up in this decompilation unit (boost::flyweight
// internals, TypedPropertyDescriptor<ProtectedString>, Variant conversions,
// Script/ScriptContext/LibraryService methods) belongs to other modules and
// is noted in the TODO block at the bottom.

#include "Util/ProtectedString.h"
#include "util/MD5Hasher.h"
#include "Reflection/Type.h"

namespace RBX {
namespace Reflection
{

	// Type singleton (section 258b; binary 0x234BAC). Needed by the
	// on_index ProtectedString arm's Type compare; registrar pre-touches
	// it pre-main per the 246 lockdown prescription.
	// FUNCTION: PLAYER 0x0066f200
	// FUNCTION: STUDIO 0x0077fdf0
	template<>
	const Type& Type::getSingleton<ProtectedString>()
	{
		static TType<ProtectedString> type("ProtectedString");
		return type;
	}

	template<> TypeRegistrar<ProtectedString>
		TypeRegistrar<ProtectedString>::registrar(0);

}

	// NOTE: the 2013 binary stores the salt as the literal "19:14:55"
	// (seen at 0x234658 in calculateHash and returned by getSalt @0x234862).
	static const char* const kProtectedStringSalt = "19:14:55";

	const ProtectedString ProtectedString::emptyString;

	ProtectedString::ProtectedString()
		: source()
		, hash(new std::string())
	{
		source.assign("");
		calculateHash(hash.get());
	}

	// FUNCTION: PLAYER 0x0066f170
	// FUNCTION: STUDIO 0x0077fd60
	ProtectedString::ProtectedString(const ProtectedString& other)
		: source(other.source)
		, hash(new std::string(*other.hash))
	{
	}

	ProtectedString::~ProtectedString()
	{
	}

	ProtectedString ProtectedString::fromTrustedSource(const std::string& stringRef) // 0x2345E2
	{
		ProtectedString result;
		result.source.assign(stringRef);
		result.calculateHash(result.hash.get());
		return result;
	}

	const std::string& ProtectedString::getOriginalHash() const // 0x234856
	{
		return *hash;
	}

	void ProtectedString::calculateHash(std::string* out) const // 0x234658
	{
		if (!source.empty())
		{
			MD5Hasher* hasher = MD5Hasher::create();
			// order matters: salt is fed first, then the source
			hasher->addData(std::string(getSalt()));
			hasher->addData(source);
			*out = hasher->toString();
			delete hasher;
		}
		else
		{
			out->assign("");
		}
	}

	const char* ProtectedString::getSalt() // 0x234862
	{
		return kProtectedStringSalt;
	}

	bool ProtectedString::readUnprotectedChar(int index, char* outChar) const // 0x234874
	{
		if (index >= 0 && static_cast<int>(source.size()) > index)
		{
			*outChar = source[index];
			return true;
		}
		return false;
	}

	bool ProtectedString::operator==(const ProtectedString& other) const // 0x23489A
	{
		return source.compare(other.source) == 0;
	}

	bool ProtectedString::operator!=(const ProtectedString& other) const
	{
		return !(*this == other);
	}

	ProtectedString& ProtectedString::operator=(const ProtectedString& other) // 0x2348C0
	{
		if (this != &other)
		{
			source.assign(other.source);
			calculateHash(hash.get());
		}
		return *this;
	}

	size_t hash_value(const ProtectedString& str) // boost::hash<ProtectedString> @0x161368
	{
		size_t result = 0;
		const std::string& s = str.getStringForImmediateUse();
		for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
		{
			result ^= static_cast<unsigned char>(*it) + (result >> 2) + (result << 6) - 1640531527u;
		}
		return result;
	}

}

// TODO (belonging to other modules, decompiled in this export):
//  - boost::flyweight<ProtectedString> core/insert_value/erase/hash et al.
//    (0x161098, 0x1612C8, 0x161320, 0x161560, 0x188D96, 0x1891BA, 0x634E6C,
//    mac: 0x634EF0, 0x634FA8, 0x636E70, ...) - covered by vendored boost::flyweights.
//  - RBX::Script::setEmbeddedCode / EmbeddedSource::setCode (0x180F08, 0x181790)
//  - RBX::ScriptContext::executeInNewThread / addLibrary (0x190DEE, 0x190F98, 0x19271A)
//  - RBX::LibraryService::contentReadyLocal (0x62FE44)
//  - luaL_loadprotected (0x639C9C) - belongs to lua bridge module
//  - RBX::Reflection::Type::getSingleton<ProtectedString> (0x234BAC),
//    Type::Type<ProtectedString> (0x235742, asserts at include/reflection/Type.h:66),
//    TypedPropertyDescriptor<ProtectedString> ctors/dtor/getVariant/setVariant/
//    readValue/writeValue/getStringValue/setStringValue/getDataSize/hasStringValue/
//    equalValues/copyValue/isReadOnly/isWriteOnly (0x1882F6 - 0x188C9A,
//    mac: 0x234C24, 0x234D54, 0x234F84 - 0x23522E) - reflection module.
//  - RBX::Reflection::Variant::convert/genericConvert<ProtectedString>
//    (0x234EB6, 0x2353A2) - Variant module.
//  - rbx::implementation::typed_holder<ProtectedString> (0x188BA8, 0x2356C0, 0x23571C)
//  - RBX::StringConverter<ProtectedString>::convertToValue (0x234A8C) and
//    XmlNameValuePair::getValue<ProtectedString> (0x2348F0) - serializer module.
