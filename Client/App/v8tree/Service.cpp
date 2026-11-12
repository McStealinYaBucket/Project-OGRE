/* App/v8tree/Service.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\Svc_ServiceProvider.cpp.txt)
      - 0x392F80 ServiceProvider::ServiceProvider(void): Instance(this, nullptr);
        NO (const char* name) overload exists in the 2013 binary (only the default
        ctor appears in studio_function_list.txt; the const char* overload below is
        a 2016-compat shim, not from the binary).
      - 0x551FC2 newIndex: RbxInterlockedIncrementAcquire(&dword_F7F624) — a process-
        wide static counter, first index 0. (2016 uses a function-local rbx::atomic;
        same semantics; implemented here with InterlockedIncrement.)
      - 0x551DAE findPublicServiceByClassNameString / 0x551EB8 getPublicServiceByClassNameString:
        Name::lookup + nullName pointer check + "'%s' is not a valid Service name"
        runtime_error + dynamic_cast<Service*> + isPublic gate. Matches 2016 exactly
        (modulo 2013 Name* pointer API).
      - 0x551FE2 create(Instance*, Name const&): walk parents via getParent, isA
        against the ServiceProvider ClassDescriptor, createChild, dynamic_cast to
        Service, setAndLockParent. Matches 2016.
      - 0x552106 findServiceByClassName: std::map<const Name*, shared_ptr> lookup.
      - 0x552170 onDescendantRemoving: instance->onServiceProvider(this, NULL) then
        Super::onDescendantRemoving. 0x55219C onDescendantAdded: Super then
        instance->onServiceProvider(NULL, this). Match 2016.
      - 0x5521D2 onChildRemoving: serviceMap lookup by &instance->getClassName(),
        fire serviceRemovingSignal. (Super::onChildRemoving is Instance:: empty no-op.)
      - 0x552282 clearServices: RBXASSERT(numChildren()==0) at
        /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8tree/Service.cpp:88,
        then serviceArray/serviceMap clear. Matches 2016.
      - 0x552330 onChildAdded: RBXASSERT fastDynamicCast<ServiceProvider>==NULL at
        Service.cpp:96, then dynamic_cast<Service*>, serviceMap[&className]=shared,
        serviceAddedSignal. DIVERGENCE from 2016: NO lockParent() call, NO
        RBXASSERT_SLOW(findServiceByClassName==NULL) — both absent from the 2013
        pseudocode. (Super::onChildAdded is Instance:: empty no-op.)
      - 0x5524E0 createChild: findServiceByClassName hit returns existing service,
        else AbstractFactoryProduct<Instance>::create. Matches 2016.
      - 0x552620 askAddChild: dynamic_cast<const Service*> != NULL. Matches 2016.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8tree/Service.cpp
   2016 source used as style template only; binary wins everywhere above. */

#include "stdafx.h"
#include "decomp.h"

#include "V8Tree/Service.h"
#include "Util/Name.h"
#include "RbxAssert.h"
#include <boost/cast.hpp>

namespace RBX
{

	const char* const sServiceProvider = "ServiceProvider";

	REFLECTION_BEGIN();
	Reflection::BoundFuncDesc<ServiceProvider, shared_ptr<Instance>(std::string)> ServiceProvider::func_FindService(&ServiceProvider::findPublicServiceByClassNameString, "FindService", "className", Security::None);
	Reflection::BoundFuncDesc<ServiceProvider, shared_ptr<Instance>(std::string)> ServiceProvider::func_GetService(&ServiceProvider::getPublicServiceByClassNameString, "GetService", "className", Security::None);
	Reflection::BoundFuncDesc<ServiceProvider, shared_ptr<Instance>(std::string)> ServiceProvider::dep_service(&ServiceProvider::getPublicServiceByClassNameString, "service", "className", Security::None, Reflection::Descriptor::Attributes::deprecated(ServiceProvider::func_GetService));
	Reflection::BoundFuncDesc<ServiceProvider, shared_ptr<Instance>(std::string)> ServiceProvider::dep_GetService(&ServiceProvider::getPublicServiceByClassNameString, "getService", "className", Security::None, Reflection::Descriptor::Attributes::deprecated(ServiceProvider::func_GetService));

	static Reflection::EventDesc<ServiceProvider, void()> event_Closing(&ServiceProvider::closingSignal, "Close");
	static Reflection::EventDesc<ServiceProvider, void()> event_ClosingLate(&ServiceProvider::closingLateSignal, "CloseLate", Security::LocalUser);
	static Reflection::EventDesc<ServiceProvider, void(shared_ptr<Instance>)> event_ServiceAdded(&ServiceProvider::serviceAddedSignal, "ServiceAdded", "service");
	static Reflection::EventDesc<ServiceProvider, void(shared_ptr<Instance>)> event_ServiceRemoving(&ServiceProvider::serviceRemovingSignal, "ServiceRemoving", "service");
	REFLECTION_END();

	/* mac: 0x392F80 */
	// FUNCTION: PLAYER 0x00644290
	ServiceProvider::ServiceProvider()
	{
	}

	/* 2016-compat shim — NOT in the 2013 binary (no const char* ctor at any EA).
	   Kept so 2016-derived code that names a provider still compiles. */
	ServiceProvider::ServiceProvider(const char* name): Super(name)
	{
	}

	/* mac: 0x551FC2 — binary uses a process-wide static via RbxInterlockedIncrementAcquire */
	size_t ServiceProvider::newIndex()
	{
		static volatile LONG index = -1;
		return static_cast<size_t>(InterlockedIncrement(&index));
	}

	/* mac: 0x551EB8 */
	shared_ptr<Instance> ServiceProvider::getPublicServiceByClassNameString(std::string sName)
	{
		const Name* serviceName = RBX::Name::lookup(sName);
		if (serviceName == &RBX::Name::getNullName())
			throw RBX::runtime_error("'%s' is not a valid Service name", sName.c_str());

		shared_ptr<Instance> result = ServiceProvider::create(this, *serviceName);
		if (Service* service = dynamic_cast<Service*>(result.get())) {
			if (service->isPublic) {
				return result;
			}
		}
		return shared_ptr<Instance>();
	}

	/* mac: 0x551DAE */
	shared_ptr<Instance> ServiceProvider::findPublicServiceByClassNameString(std::string sName)
	{
		const Name* serviceName = RBX::Name::lookup(sName);
		if (serviceName == &RBX::Name::getNullName())
			throw RBX::runtime_error("'%s' is not a valid Service name", sName.c_str());

		shared_ptr<Instance> result = findServiceByClassName(*serviceName);
		if (Service* service = dynamic_cast<Service*>(result.get())) {
			if (service->isPublic) {
				return result;
			}
		}
		return shared_ptr<Instance>();
	}

	/* mac: 0x552106 */
	shared_ptr<Instance> ServiceProvider::findServiceByClassName(const RBX::Name& className) const
	{
		std::map<const RBX::Name*, shared_ptr<Instance> >::const_iterator iter = serviceMap.find(&className);
		if (iter != serviceMap.end())
			return iter->second;
		else
			return shared_ptr<Instance>();
	}

	/* mac: 0x552170 */
	// FUNCTION: PLAYER 0x006a9330
	// FUNCTION: STUDIO 0x0076f370
	void ServiceProvider::onDescendantRemoving(const shared_ptr<Instance>& instance)
	{
		instance->onServiceProvider(this, NULL);
		Super::onDescendantRemoving(instance);
	}

	/* mac: 0x55219C */
	// FUNCTION: PLAYER 0x006a9310
	// FUNCTION: STUDIO 0x0076f350
	void ServiceProvider::onDescendantAdded(Instance* instance)
	{
		Super::onDescendantAdded(instance);
		instance->onServiceProvider(NULL, this);
	}

	/* mac: 0x5521D2 */
	// FUNCTION: PLAYER 0x006a95e0
	// FUNCTION: STUDIO 0x0076f620
	void ServiceProvider::onChildRemoving(Instance* instance)
	{
		std::map<const RBX::Name*, shared_ptr<Instance> >::iterator iter = serviceMap.find(&instance->getClassName());
		if (iter != serviceMap.end())
			serviceRemovingSignal(iter->second);
		Super::onChildRemoving(instance);
	}

	/* mac: 0x552282 — asserts Service.cpp:88 "numChildren()==0" */
	// STUB: PLAYER 0x006a9540
	// STUB: STUDIO 0x0076f580
	void ServiceProvider::clearServices()
	{
		RBXASSERT(numChildren()==0);
		serviceArray.clear();
		serviceMap.clear();
	}

	/* mac: 0x552330 — NOTE: 2013 has NO lockParent() and NO RBXASSERT_SLOW here (2016-only) */
	// STUB: PLAYER 0x006a9770
	// STUB: STUDIO 0x0076f7b0
	void ServiceProvider::onChildAdded(Instance* instance)
	{
		// There should be only one ServiceProvider, I think
		RBXASSERT(Instance::fastDynamicCast<ServiceProvider>(instance)==NULL);

		Super::onChildAdded(instance);

		Service* service = dynamic_cast<Service*>(instance);
		if (service!=NULL)
		{
			shared_ptr<Instance> i = shared_from(instance);
			if (instance->getClassName()!=RBX::Name::getNullName())
			{
				serviceMap[&instance->getClassName()] = i;
			}
			serviceAddedSignal(i);
		}
	}

	/* mac: 0x5524E0 */
	// STUB: PLAYER 0x006a9650
	// STUB: STUDIO 0x0076f690
	shared_ptr<Instance> ServiceProvider::createChild(const RBX::Name& className, RBX::CreatorRole creatorRole)
	{
		// If the childElement is data for a Service, then have an existing Service read the data
		// rather than creating a new Service.
		shared_ptr<Instance> instance = findServiceByClassName(className);
		if (instance)
			return instance;
		else
			return Super::createChild(className, creatorRole);
	}

	/* mac: 0x551FE2 */
	shared_ptr<Instance> ServiceProvider::create(Instance* context, const RBX::Name& name)
	{
		while (context!=NULL)
		{
			ServiceProvider* serviceProvider = context->fastDynamicCast<ServiceProvider>();
			if (serviceProvider!=NULL)
			{
				shared_ptr<Instance> child = serviceProvider->createChild(name, EngineCreator);
				if (dynamic_cast<Service*>(child.get())) {
					child->setParent(serviceProvider);
					return child;
				}
				return shared_ptr<Instance>();
			}
			context = context->getParent();
		}
		return shared_ptr<Instance>();
	}

}

// Class registrars (2016 centralizes these in factoryregistration.cpp; defined
// here until that module is reconstructed).
namespace RBX {
RBX_REGISTER_CLASS(ServiceProvider);
}

DECOMP_SIZE_ASSERT(RBX::ServiceProvider, 0x8c);
