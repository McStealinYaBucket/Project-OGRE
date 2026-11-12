/* Callback.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Callback_ctor.txt)
     - CallbackDescriptor ctor 0x5F8946: category NULL (empty name), SignatureDescriptor at +28,
       declares into container at (classDescriptor + 368), NO async bool (2016-only).
   2013 divergence from 2016: no async parameter, no Sync/Async subclass split visible
   in the ctor (kept for header compatibility until the 2013 Callback.h is verified). */

#include "stdafx.h"

#include "Reflection/Callback.h"
#include "Reflection/Object.h"

using namespace RBX;
using namespace RBX::Reflection;

CallbackDescriptor::CallbackDescriptor(ClassDescriptor& classDescriptor, const char* name, Attributes attributes, Security::Permissions security, bool /*async — 2016-only param; 2013 ctor has none*/)
	: MemberDescriptor(classDescriptor, name, NULL, attributes, security)
	, signature()
	, async(false)
{
	// TODO: This could be moved up into MemberDescriptor if it were templated...
	classDescriptor.MemberDescriptorContainer<CallbackDescriptor>::declare(this);
}

SyncCallbackDescriptor::SyncCallbackDescriptor(ClassDescriptor& classDescriptor, const char* name, Attributes attributes, Security::Permissions security)
	: CallbackDescriptor(classDescriptor, name, attributes, security, false)
{
}

void SyncCallbackDescriptor::setGenericCallbackHelper(DescribedBase* object, const GenericFunction& function) const
{
	shared_ptr<GenericFunction> ptr(new GenericFunction(function));
	setGenericCallback(object, ptr);
}

AsyncCallbackDescriptor::AsyncCallbackDescriptor(ClassDescriptor& classDescriptor, const char* name, Attributes attributes, Security::Permissions security)
	: CallbackDescriptor(classDescriptor, name, attributes, security, true)
{
}

void AsyncCallbackDescriptor::setGenericCallbackHelper(DescribedBase* object, const GenericFunction& function) const
{
	shared_ptr<GenericFunction> ptr(new GenericFunction(function));
	setGenericCallback(object, ptr);
}

void AsyncCallbackDescriptor::callGenericImpl(shared_ptr<AsyncCallbackDescriptor::GenericFunction> function, shared_ptr<Tuple> args,
    AsyncCallbackDescriptor::ResumeFunction resumeFunction, AsyncCallbackDescriptor::ErrorFunction errorFunction)
{
    try
    {
        (*function)(args, resumeFunction, errorFunction);
    }
    catch (const RBX::base_exception& e)
    {
        errorFunction(e.what());
    }
}
