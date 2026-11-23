/* security/FuzzyTokens.h -- October 2013 reconstruction.
   Anti-tamper report bits: the client ORs flags into a 64-bit tag that is
   TEA-encrypted before it is sent, and the server decrypts it.
   No 2013 trace found: iOS 10-29 has no teaEncrypt/teaDecrypt, Tokens:: or
   *FuzzySecurityToken symbol (the scheme may be Windows-only or later).
   Kept because security/ApiSecurity.h, Util/Object.h and V8Tree/Verb.h
   include it and ApiSecurity's inline reporters call addFlagFast/addFlagSafe
   on Tokens::sendStatsToken / apiToken and OR into Tokens::simpleToken.
   Layouts are the prior tree's (tag, prevTag, savedTag, then the mutex). */
#pragma once

#include "boost/thread/mutex.hpp"

#ifdef WIN32
#define RBX_TOKEN_INLINE __forceinline
#else
#define RBX_TOKEN_INLINE inline
#endif

namespace RBX {

namespace Security {
	unsigned long long teaEncrypt(unsigned long long plain);
	unsigned long long teaDecrypt(unsigned long long cipher);
}

namespace Tokens {
	// one 64-bit tag, or its two 32-bit halves
	union binaryTag
	{
		unsigned int asHalf[2];
		unsigned long long asFull;
	};
}

class ClientFuzzySecurityToken
{
	Tokens::binaryTag tag;
	Tokens::binaryTag prevTag;
	Tokens::binaryTag savedTag;
	boost::mutex tokenMutex;

public:
	ClientFuzzySecurityToken(unsigned long long initial);
	void set(unsigned long long value);

	// unlocked OR, for callers already serialised
	RBX_TOKEN_INLINE void addFlagFast(unsigned long long bits) { tag.asFull |= bits; }

	// OR under tokenMutex
	RBX_TOKEN_INLINE void addFlagSafe(unsigned long long bits)
	{
		boost::mutex::scoped_lock hold(tokenMutex);
		tag.asFull |= bits;
	}

	unsigned long long crypt();
	unsigned long long getPrev() { return prevTag.asFull; }
};

// Server side: a 1 bit sent by the client may arrive as 0, never the reverse.
class ServerFuzzySecurityToken
{
	Tokens::binaryTag lastTag;
	Tokens::binaryTag tag;
	unsigned int ignoreFlags;

public:
	ServerFuzzySecurityToken(unsigned long long initial, unsigned int ignoreFlags = 0);
	void setLastTag(unsigned long long value) { lastTag.asFull = value; }
	unsigned long long decrypt(unsigned long long cipher);
};

namespace Tokens {
	extern ClientFuzzySecurityToken sendStatsToken;
	extern unsigned int simpleToken;
	extern ClientFuzzySecurityToken apiToken;
}

} // namespace RBX

#undef RBX_TOKEN_INLINE
