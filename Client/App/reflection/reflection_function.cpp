/* reflection_function.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\FnDesc_ctors.txt)
     - FunctionDescriptor ctor 0x1569B4: category "Function", SignatureDescriptor at +28,
       declares into container at (classDescriptor + 192)
     - YieldFunctionDescriptor ctor 0x156A6C: category "YieldFunction", container at +280
   2013 divergence from 2016: NO kind/Kind_Default field (2016-only). */

#include "stdafx.h"

#include "Reflection/Function.h"
#include "Reflection/YieldFunction.h"
#include "Reflection/Object.h"

using namespace RBX;
using namespace RBX::Reflection;

// FUNCTION: PLAYER 0x0071a650
// FUNCTION: STUDIO 0x007c3700
FunctionDescriptor::FunctionDescriptor(ClassDescriptor& classDescriptor, const char* name, Security::Permissions security, Attributes attributes)
	:MemberDescriptor(classDescriptor, name, "Function", attributes, security)
	,signature()
{
	// TODO: This could be moved up into MemberDescriptor if it were templated...
	classDescriptor.MemberDescriptorContainer<FunctionDescriptor>::declare(this);
}

// FUNCTION: PLAYER 0x0071a6d0
// FUNCTION: STUDIO 0x007c3780
YieldFunctionDescriptor::YieldFunctionDescriptor(ClassDescriptor& classDescriptor, const char* name, Security::Permissions security, Attributes attributes)
	:MemberDescriptor(classDescriptor, name, "YieldFunction", attributes, security)
	,signature()
{
	// TODO: This could be moved up into MemberDescriptor if it were templated...
	classDescriptor.MemberDescriptorContainer<YieldFunctionDescriptor>::declare(this);
}
