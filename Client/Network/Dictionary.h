/* Network/Dictionary.h -- October 2013, partial: the shapes IdSerializer's scope dictionary needs.
   Evidence: its ctor PLAYER 0x005aa320 (SenderDictionary ctor 0x005aa0b0, then a 128 x std::string
   eh-vector ctor at +0x21c) and dtor 0x00599140 (the strings, then the unordered_map's delete_nodes
   0x00892af0). iOS 10-29 names SenderDictionary<RBX::Name const*> and ReceiverDictionary<std::string>.
   Nothing here is used by a matched function yet. */

#pragma once

#include "rbx/Debug.h"
#include <boost/unordered_map.hpp>
#include <string>

namespace RBX {
	class Name;

namespace Network {

	// 128 entries: 0x200 bytes of items, 0xe00 bytes of strings.
	#define DICTIONARY_SIZE 128

	template<class T>
	class SenderDictionary;

	// Ctor 0x005aa0b0: the inline unordered_map ctor (11 buckets from the prime list, mlf 1.0f),
	// items left uninitialized, nextIndex = 1.
	template<>
	class SenderDictionary<const RBX::Name*>
	{
		typedef boost::unordered_map<const RBX::Name*, unsigned char> Dictionary;
		Dictionary dictionary;                  // 0x0
		RBX::Name* items[DICTIONARY_SIZE];      // 0x18
		int nextIndex;                          // 0x218
	public:
		SenderDictionary()
			: nextIndex(1)
		{
		}
	};

	template<class T>
	class ReceiverDictionary
	{
		T dictionary[DICTIONARY_SIZE];
	};

	// IdSerializer's scope-name dictionary (sender of Name pointers, receiver of strings), size 0x101c.
	// 2016 spells it SharedDictionary<Guid::Scope> with one parameter for both halves; 2013's halves
	// differ, so the name is a placeholder until a symbol names it.
	class SharedScopeDictionary
		: public SenderDictionary<const RBX::Name*>
		, public ReceiverDictionary<std::string>
	{
	};

} // namespace Network
} // namespace RBX
