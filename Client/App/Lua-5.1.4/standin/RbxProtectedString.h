/* Stand-in for the 2013 RBX headers the Roblox additions to lauxlib.c/lbaselib.c include
** (Util/ProtectedString.h, Util/MD5Hasher.h, V8DataModel/DataModel.h, V8DataModel/HackDefines.h):
** only the declarations those additions use, with the 2013 shapes PLAYER shows.
**   ProtectedString: 0x20 bytes = std::string source + std::string* originalHash (+0x1c), dtor
**     0x00408950 out of line; fromTrustedSource 0x0066f320, calculateHash 0x0066f060,
**     getOriginalHash 0x0066ef40 (returns *(+0x1c)), getSalt 0x0066ef10, readUnprotectedChar 0x0066ef50.
**   MD5Hasher: vftable dtor, addData(const char*, size_t) +4, addData(const std::string&) +8,
**     addData(std::istream&) +0xc, toString() +0x10 (MSVC orders the addData overloads reversed);
**     create() 0x00652bf0.
**   DataModel::sendStats at 0x00de4764; HATE_LUA_SCRIPT_HASH_CHANGED 0x200, HATE_LUA_HASH_CHANGED 0x4000. */
#pragma once
#include <string>
#include <iosfwd>

namespace RBX
{
	class ProtectedString
	{
	public:
		~ProtectedString();
		static ProtectedString fromTrustedSource(const std::string& source);
		void calculateHash(std::string* result) const;
		const std::string& getOriginalHash() const;
		const char* getSalt() const;
		bool readUnprotectedChar(int index, char* out) const;
	private:
		std::string source;
		std::string* originalHash;
	};

	class MD5Hasher
	{
	public:
		virtual ~MD5Hasher() {}
		virtual void addData(std::istream& data) = 0;
		virtual void addData(const std::string& data) = 0;
		virtual void addData(const char* data, size_t nBytes) = 0;
		virtual std::string toString() = 0;
		static MD5Hasher* create();
	};

	class DataModel
	{
	public:
		static unsigned int sendStats;
	};
}

#define HATE_LUA_SCRIPT_HASH_CHANGED	0x200
#define HATE_LUA_HASH_CHANGED			0x4000
