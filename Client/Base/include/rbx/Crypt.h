// Base/include/rbx/Crypt.h: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbols RBX::Crypt::Crypt(), ~Crypt() and verifySignatureBase64(std::string,
// std::string) (both by value). On Windows the class holds a CryptoAPI provider and key; the
// 2016 Durango (Xbox) exclusion is dropped, since no 2013 build targets it.
#pragma once

#if defined(_WIN32)
#include <windows.h>
#include <wincrypt.h>
#endif

#include <string>

namespace RBX {

class Crypt
{
#if defined(_WIN32)
	HCRYPTPROV context;
	HCRYPTKEY key;
#endif
public:
	Crypt();
	~Crypt();
	// Throws when the signature does not verify.
	void verifySignatureBase64(std::string message, std::string signatureBase64);
};

}
