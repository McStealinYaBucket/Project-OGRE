/**
 @file debugAssert.cpp

 Windows implementation of assertion routines.

 @maintainer Morgan McGuire, graphics3d.com

 @created 2001-08-26
 @edited 2006-02-02
 */

/* Base/RbxAssert.cpp — vendored from March 2016 reference (tiny, version-neutral:
   assertion-hook plumbing only). */

#include "RbxPlatform.h" // includes <windows.h>

#include "RbxAssert.h"
#include "RbxFormat.h"
#include <string>

using namespace std;

namespace RBX {

namespace _internal {
    // GLOBAL: PLAYER 0x00e1aac8
    // GLOBAL: STUDIO 0x00f9dcb8
    AssertionHook _debugHook;
    AssertionHook _failureHook;
} // internal

void setAssertionHook(AssertionHook hook) {
	RBX::_internal::_debugHook = hook;
}

AssertionHook assertionHook() {
	return 	RBX::_internal::_debugHook;
}

void setFailureHook(AssertionHook hook) {
	RBX::_internal::_failureHook = hook;
}

AssertionHook failureHook() {
	return RBX::_internal::_failureHook;
}


} // namespace
