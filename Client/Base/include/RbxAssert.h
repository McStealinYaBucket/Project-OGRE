// Base/include/RbxAssert.h: October 2013 reconstruction (Project OGRE).
// The assertion hook interface (RBX's copy of G3D's debugAssert hooks; iOS 1029 still carries
// G3D::_internal::_failureHook beside it). rbx/Debug.h's RBXASSERT calls through
// RBX::_internal::_debugHook, which many App files reference directly. Bodies in Base/RbxAssert.cpp.
#ifndef RBX_RBXASSERT_H
#define RBX_RBXASSERT_H

#include <string>
#include "RbxBase.h"

namespace RBX {

// Returns true to request a debugger break.
typedef bool (*AssertionHook)(const char* _expression, const char* filename, int lineNumber);

// The hook a failed debug assert calls; NULL ignores failures.
void setAssertionHook(AssertionHook hook);
AssertionHook assertionHook();

// The hook a failed release-mode assert calls; true makes the program exit.
void setFailureHook(AssertionHook hook);
AssertionHook failureHook();

namespace _internal {
extern AssertionHook _debugHook;
extern AssertionHook _failureHook;
}

}

#endif
