/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

#pragma once

#include "Reflection/Reflection.h"
#include "Reflection/Event.h"
#include "V8Tree/Property.h"
#include "V8Xml/Reference.h"
#include "V8Tree/Verb.h"
#include "util/FeatherweightDict.h"

#include "rbx/Countable.h"
#include "Util/Limits.h"
#include "Util/Guid.h"

#include <vector>
#include <string>
#include "boost/weak_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include <boost/static_assert.hpp>
#include <boost/flyweight.hpp>

namespace RBX {

	class Instance;
	class GuiObject;

// Convenience class
template <
	class Class, 
	class BaseClass, 
	const char* const& sClassName, 
	Reflection::ClassDescriptor::Functionality functionality = Reflection::ClassDescriptor::PERSISTENT, 
	Security::Permissions security = Security::None 
	>
class DescribedCreatable : public Reflection::Described<Class, sClassName, FactoryProduct<Class, BaseClass, sClassName, Instance>, functionality, security>
{
protected:
	inline DescribedCreatable() {}
	template<class Arg0>
	inline DescribedCreatable(Arg0 arg0):Reflection::Described<Class, sClassName, FactoryProduct<Class, BaseClass, sClassName, Instance>, functionality, security>(arg0) {}
	template<class Arg0, class Arg1>
	inline DescribedCreatable(Arg0 arg0, Arg1 arg1):Reflection::Described<Class, sClassName, FactoryProduct<Class, BaseClass, sClassName, Instance>, functionality, security>(arg0, arg1) {}
};

// Convenience class
template <
	class Class, 
	class BaseClass, 
	const char* const& sClassName, 
	Reflection::ClassDescriptor::Functionality functionality = Reflection::ClassDescriptor::PERSISTENT, 
	Security::Permissions security = Security::None 
	>
class DescribedNonCreatable : public Reflection::Described<Class, sClassName, NonFactoryProduct<BaseClass, sClassName>, functionality, security>
{
protected:
	inline DescribedNonCreatable() {}
	template<class Arg0>
	inline DescribedNonCreatable(Arg0 arg0):Reflection::Described<Class, sClassName, NonFactoryProduct<BaseClass, sClassName>, functionality, security>(arg0) {}
	template<class Arg0, class Arg1>
	inline DescribedNonCreatable(Arg0 arg0, Arg1 arg1):Reflection::Described<Class, sClassName, NonFactoryProduct<BaseClass, sClassName>, functionality, security>(arg0, arg1) {}
	template<class Arg0, class Arg1, class Arg2>
	inline DescribedNonCreatable(Arg0 arg0, Arg1 arg1, Arg2 arg2):Reflection::Described<Class, sClassName, NonFactoryProduct<BaseClass, sClassName>, functionality, security>(arg0, arg1, arg2) {}
	template<class Arg0, class Arg1, class Arg2, class Arg3>
	inline DescribedNonCreatable(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3):Reflection::Described<Class, sClassName, NonFactoryProduct<BaseClass, sClassName>, functionality, security>(arg0, arg1, arg2, arg3) {}
};

class Instance;
class ServiceProvider;


/// A child has been added to the Notifier
struct ChildAdded {
public:
	shared_ptr<Instance> const child;
	ChildAdded(Instance* child);
	ChildAdded(const ChildAdded& event);
private:
	ChildAdded& operator=(const ChildAdded&);
};

/// A child has been removed from the Notifier
struct ChildRemoved {
public:
	shared_ptr<Instance> const child;
	ChildRemoved(Instance* child);
	ChildRemoved(const ChildRemoved& event);
private:
	ChildRemoved& operator=(const ChildAdded&);
};

/// A descendant has been added to the Notifier
struct DescendantAdded {
	friend class Instance;
public:
	shared_ptr<Instance> const instance;
	shared_ptr<Instance> const parent;		// The direct parent of the descendant
private:
	DescendantAdded(shared_ptr<Instance> instance, shared_ptr<Instance> parent) 
		:instance(instance),parent(parent)
	{}
	DescendantAdded(Instance* instance, Instance* parent) 
		:instance(shared_from(instance)),parent(shared_from(parent))
	{}
};

/// A descendant of the Notifier is about to be removed
struct DescendantRemoving {
public:
	shared_ptr<Instance> const instance;
	shared_ptr<Instance> const parent;		// The direct parent of the descendant before it is removed
	DescendantRemoving(const shared_ptr<Instance>& instance, const shared_ptr<Instance>& parent) 
		:instance(instance),parent(parent)
	{}
};

struct AncestorChanged {
public:
	Instance* const child;
	Instance* const oldParent;
	Instance* const newParent;
	AncestorChanged(Instance* child, Instance* oldParent, Instance* newParent) 
		:child(child),oldParent(oldParent),newParent(newParent)
	{}
};

extern const char* const sInstance;

typedef std::vector<shared_ptr<Instance> > Instances;

class OnDemandInstance : public Allocator<OnDemandInstance>
{
public:
	rbx::signal<void(shared_ptr<Instance>)> childAddedSignal;
	rbx::signal<void(shared_ptr<Instance>)> childRemovedSignal;
	rbx::signal<void(shared_ptr<Instance>)> descendantAddedSignal;
	rbx::signal<void(shared_ptr<Instance>)> descendantRemovingSignal;
	rbx::signal<void(shared_ptr<Instance>)> instanceClonedSignal;
	
	struct ThreadWaitingForChild
	{
		std::string childName;
		boost::function<void(shared_ptr<Instance>)> resumeFunction;
	};
	std::vector<ThreadWaitingForChild> threadsWaitingForChildren;

	virtual ~OnDemandInstance(){};
};

/* 2013 layout (RTTI Complete Object Locator of vftable 0x00b2a6ec; ctors 0x00620da0 /
   0x00620ed0, dtor 0x0061d510). MSVC puts the polymorphic bases first, in declaration
   order, then the others:
     Debugable @0x0 (primary vftable, 28 slots: dump, then Instance's own virtuals)
     GuidItem<Instance> @0x4 (0x14), AbstractFactoryProduct<Instance>/Object @0x18,
     Described/DescribedBase @0x1c (EventSource vfptr), Limits::Countable @0x2c,
     Diagnostics::Countable<Instance> @0x34, boost::noncopyable @0x35.
   Members from 0x38; size 0x60 (ServiceProvider's members start at 0x60, ctor 0x00644290).
   Where 2013 disagrees with 2016: 2016 has no Debugable, Limits::Countable or
   AbstractFactoryProduct base and a different base order; archivable and robloxLocked are
   not Instance members in 2013 but FW bytes (setIsArchivable 0x0061d7b0 reads fw+0x1a,
   setRobloxLocked 0x0061d780 fw+0x19, both through FWValue<bool>::set); there is no
   humanoidChanged virtual (absent from the 2013 vftable and the iOS 10-29 vtable). */
// VTABLE: PLAYER 0x00b2a6ec RBX::Debugable
// VTABLE: STUDIO 0x00bed4ac RBX::Debugable
// SIZE 0x60
class Instance
	: public Limits::Countable
	, public Diagnostics::Countable<Instance>
	, public Debugable
	, public GuidItem<Instance>
	, public AbstractFactoryProduct<Instance>
	, public Reflection::Described<Instance, sInstance>
	, public boost::noncopyable
{
	friend class SetParentSentry;
private:
	static void predelete(Instance* instance);
	void predelete();
	friend class Creatable<Instance>::Deleter;
	// GuiObject::askSetParent 0x3AC1EA negates the parent's askForbidChild
	// through the vtable (cross-object protected call, like the binary).
	friend class GuiObject;


	// 2013: NO isParentLocked member, NO lockParent()/unlockParent() (no
	// mangles anywhere in the 2013 PEs). Lock state is FW b21: setParentInternal
	// mac: 0x542D16 gates on it, setAndLockParent 0x5450AC sets it via FWValue<bool>.

	// 2013 name storage (Instance ctors 0x54433A/0x5446CA, getName 0x545E5E):
	// the name lives ONLY in the flyweight (FWInstance +24, default
	// "Instance"); Instance+0x48 holds shared_ptr<FWBase>. There is NO
	// 2016-style name member in 2013 (migrated 2026-09-25; the flyweight
	// stand-in is gone).

protected:
	// 2013 has exactly these two (iOS 10-29 _ZN3RBX8InstanceC2EPNS_10FWInstanceE and
	// _ZN3RBX8InstanceC2EPKcPNS_10FWInstanceE): a NULL fw gets a fresh FWFinal<FWInstance>
	// through FWBase::init<FWInstance> 0x0061fe70.
	Instance(FWInstance* fw = NULL);
	Instance(const char* name, FWInstance* fw = NULL);

	// Destructor is protected. Call "destroy(instance)" instead of "delete instance".
	virtual ~Instance();

	virtual OnDemandInstance* initOnDemand();

public:
	// FUNCTION: PLAYER 0x006beaa0
	// FUNCTION: STUDIO 0x006ede50
	FWInstance* fw() const { return static_cast<FWInstance*>(fwBase.get()); }	// the slot always holds an FWInstance (or subclass)
	// Live +68 slot as FWRef (layout-identical single shared_ptr): the FW
	// machinery (registerFlyweight, FWValue::set redirect) must act on the
	// LIVE slot, not a local copy -- binary passes +68 directly (clearTerrain
	// mac: 0x514FF2 passes terrain+68; onServiceProvider 0x44523E passes this+68).
	// Public like fw() above (binary has no access control).
	FWRef* fwRef() { return reinterpret_cast<FWRef*>(&fwBase); }
	const OnDemandInstance* onDemandRead() const;
	OnDemandInstance* onDemandWrite();
    
    // onGuidChanged: GuidItem<Instance>'s, not overridden in 2013 (see Instance.cpp).

	// 2013 lock state = FW b21 (see above; no member).
	bool getIsParentLocked() const
	{
		FWInstance* f = fw();
		return f != NULL && f->b21 != 0;
	}
	void securityCheck() const;
	void securityCheck(RBX::Security::Context& context) const;

	static Reflection::PropDescriptor<Instance, bool> propArchivable;
	bool getIsArchivable() const { return fw()->b23 != 0; }	// 0x0061d7b0 reads fw+0x1a
	virtual void setIsArchivable(bool value);

	// Call "destroy" to delete an Instance fully so it can't be used again
	virtual void destroy();
	void remove();
 
	enum SaveFilter
	{
		SAVE_WORLD	= 0,
		SAVE_GAME	= 1,
		SAVE_ALL	= 2,
	};

	static const Reflection::PropDescriptor<Instance, std::string> desc_Name;
	static const Reflection::RefPropDescriptor<Instance, Instance> propParent;
	static const Reflection::PropDescriptor<Instance, bool> propRobloxLocked;

	enum CombinedSignalType
	{
		CHILD_ADDED,
		CHILD_REMOVED,
		PROPERTY_CHANGED,
		EVENT_INVOCATION,
		OUTFIT_CHANGED, 
		ANCESTRY_CHANGED,
		CLUMP_CHANGED,
		SLEEPING_CHANGED,
        HUMANOID_CHANGED
	};

	class ICombinedSignalData
	{
	public:
		virtual ~ICombinedSignalData() {};
	};

	class ChildAddedSignalData : public ICombinedSignalData
	{
	public:
		ChildAddedSignalData(const shared_ptr<Instance>& child)
			: child(child)
		{}
		
		shared_ptr<Instance> child;
	};
	class ChildRemovedSignalData : public ICombinedSignalData
	{
	public:
		ChildRemovedSignalData(const shared_ptr<Instance>& child)
			: child(child)
		{}
		
		shared_ptr<Instance> child;
	};
	class AncestryChangedSignalData: public ICombinedSignalData
	{
	public:
		AncestryChangedSignalData(const shared_ptr<Instance>& child, const shared_ptr<Instance>& newParent)
			: child(child)
			, newParent(newParent)
		{}
		
		shared_ptr<Instance> child;
		shared_ptr<Instance> newParent;
	};
	class OutfitChangedSignalData: public ICombinedSignalData
	{
	public:
		OutfitChangedSignalData()
		{}
	};

	class PropertyChangedSignalData : public ICombinedSignalData
	{
	public:
		PropertyChangedSignalData(const Reflection::PropertyDescriptor* propertyDescriptor)
			:propertyDescriptor(propertyDescriptor)
		{}
		const Reflection::PropertyDescriptor* propertyDescriptor;
	};

	class EventInvocationSignalData : public ICombinedSignalData
	{
	public:
		EventInvocationSignalData(const Reflection::EventDescriptor* eventDescriptor, const Reflection::EventArguments* eventArguments, const SystemAddress* target)
			:eventDescriptor(eventDescriptor)
			,eventArguments(eventArguments)
            ,target(target)
		{}
		const Reflection::EventDescriptor* eventDescriptor;
		const Reflection::EventArguments* eventArguments;
        const SystemAddress* target;
	};

	class HumanoidChangedSignalData: public ICombinedSignalData
	{
	public:
		HumanoidChangedSignalData()
		{}
	};

	void humanoidChanged();


	void childAddedSignal(shared_ptr<Instance>& inst) { if(onDemandRead()) onDemandWrite()->childAddedSignal(inst); }
	rbx::signal<void(shared_ptr<Instance>)>* getOrCreateChildAddedSignal(bool create = true) { return (onDemandRead() || create) ? &onDemandWrite()->childAddedSignal : NULL; };

	void childRemovedSignal(shared_ptr<Instance>& inst) { if(onDemandRead()) onDemandWrite()->childRemovedSignal(inst); }
	rbx::signal<void(shared_ptr<Instance>)>* getOrCreateChildRemovedSignal(bool create = true) { return (onDemandRead() || create) ? &onDemandWrite()->childRemovedSignal : NULL; };

	void descendantAddedSignal(shared_ptr<Instance>& inst) { if(onDemandRead()) onDemandWrite()->descendantAddedSignal(inst); }
	rbx::signal<void(shared_ptr<Instance>)>* getOrCreateDescendantAddedSignal(bool create = true) { return (onDemandRead() || create) ? &onDemandWrite()->descendantAddedSignal : NULL; };

	void descendantRemovingSignal(const shared_ptr<Instance>& inst) { if(onDemandRead()) onDemandWrite()->descendantRemovingSignal(inst); }
	rbx::signal<void(shared_ptr<Instance>)>* getOrCreateDescendantRemovingSignal(bool create = true) { return (onDemandRead() || create) ? &onDemandWrite()->descendantRemovingSignal : NULL; };

private:
	// 2013 member block (ctor 0x00620ed0 stores in this order).
	Instance* parent;						// 0x38  set after initialization by Instance::addChild; setParentInternal 0x00620440
	copy_on_write_ptr<Instances> children;	// 0x3c  findChildIndex 0x0061b1d0
	bool isSettingParent;					// 0x44  setParentInternal 0x00620646
	shared_ptr<FWBase> fwBase;				// 0x48  fw() 0x006beaa0
protected:
	boost::scoped_ptr<OnDemandInstance> onDemandPtr;	// 0x50  onDemandRead 0x006be9e0
public:
	// combinedSignal is an optimization. You could use the regular signals, like childAddedSignal.
	// However, if you are listening to many signals then you can save memory by listening to just the combinedSignal
	rbx::signal<void(CombinedSignalType, const ICombinedSignalData*)> combinedSignal;	// 0x54  raisePropertyChanged 0x0061c7c1
	rbx::signal<void(shared_ptr<Instance>, shared_ptr<Instance>)> ancestryChangedSignal;	// 0x58  by elimination (ctor 0x0061a340)
	rbx::signal<void(const Reflection::PropertyDescriptor*)> propertyChangedSignal;		// 0x5c  raisePropertyChanged 0x0061c7ca

	shared_ptr<Instance> clone(CreatorRole creatorRole);
	virtual shared_ptr<Instance> luaClone(); //Just like regular clone, but it enforces the Instance limits
	static XmlElement* toNewXmlRoot(Instance* instance, RBX::CreatorRole creatorRole);		// DB 12/5/05 - added here to centralize the spawner behavior - to, from XML

	void removeAllChildren();

    // security: this call has checks that prevent it from being called from most code.
	void destroyAllChildrenLua();

	const std::string& getClassNameStr() const { return getClassName().str; }	// used by reflection
	// NOTE (2026-09-26): was `return getClassName().toString()` — a reference
	// to a temporary (C4172). It crashed parsetest section 39 outright.
	// Member-ref return keeps the signature with safe (global-Name) lifetime.

	std::string getReadableDebugId(int scopeLength) const { return getGuid().readableString(scopeLength); }
	std::string getReadableDebugId() const { return getGuid().readableString(); }
	std::string getReadableDebugId(int scopeLength) { return getGuid().readableString(scopeLength); }

	inline Instance* getParent()					{return parent;}
	inline const Instance* getParent() const		{return parent;}
private:
	inline Instance* getParentDangerous() const		{return parent;}			// only used by refProp descriptor

public:
	inline Instance* getRootAncestor() {return parent ? parent->getRootAncestor() : this;}
	inline const Instance* getRootAncestor() const {return parent ? parent->getRootAncestor() : this;}

	static Instance* getRootAncestor(Instance* instance) { return instance ? instance->getRootAncestor() : NULL; }
	static const Instance* getRootAncestor(const Instance* instance) { return instance ? instance->getRootAncestor() : NULL; }

	bool getRobloxLocked() const { return fw()->b22 != 0; }	// 0x0061d780 reads fw+0x19
	void setRobloxLocked(bool value);

	bool contains(const Instance* child) const;		// Recursive. Also returns true if this==child

	void setParent(Instance* instance) { setParentInternal(instance, false); }
	bool setLockedParent(Instance* instance) { return setParentInternal(instance, true); }
	void setParent2(shared_ptr<Instance> instance) {
		setParent(instance.get());
	}
	void promoteChildren();
    static const void* getSetParentAddr();

	// locks the parent and then sets it
	// if this fails to set the parent it will unlock the parent before returning
	// if the parent is successfully set the parent will stay locked
	void setAndLockParent(Instance* instance);

	const std::string& getName() const  // 0x545E5E: FWInstance +24 string
	{
		return static_cast<const FWInstance*>(fwBase.get())->name.value;
	}
	virtual void setName(const std::string& value);  // 0x5451F8

	std::string getFullName() const;							// Render up to (but not including) the root node
	std::string getFullNameForReflection() { return getFullName(); }

	bool isAncestorOf(const Instance* descendant) const
	{
		if (descendant==NULL)
			return false;
		else if (descendant->getParent()==this)
			return true;
		else
			return isAncestorOf(descendant->getParent());
	}

	bool isAncestorOf2(shared_ptr<Instance> descendant) { return isAncestorOf(descendant.get()); }
	bool isDescendantOf2(shared_ptr<Instance> ancestor) { return isDescendantOf(ancestor.get()); }

	// isDescendantOf will return true if parent==NULL (consistent with the fact that a top-level Instance has parent==NULL)
	bool isDescendantOf(const Instance* ancestor) const
	{
		const Instance* parent = getParent();
		if (ancestor==parent)
			return true;
		else if (parent!=NULL)
			return parent->isDescendantOf(ancestor);
		else
			return false;
	}

	template<class Type>
	Type* findFirstAncestorOfType()
	{
		Instance* parent = getParent();
		if (Type* parentType = this->fastDynamicCast<Type>(parent))
			return parentType;
		else if (parent!=NULL)
			return parent->findFirstAncestorOfType<Type>();
		else
			return NULL;
	}

	size_t numChildren() const						{return children ? children->size() : 0;}

	int findChildIndex(const Instance* instance) const;
	virtual int getPersistentDataCost() const ;
	static int computeStringCost(const std::string& value)
	{
		return std::max<int>(1, value.length() / 100);
	}


	Instance* getChild(size_t index)				{return (*children)[index].get();}
	const Instance* getChild(size_t index) const	{return (*children)[index].get();}

	void waitForChild(std::string childName, boost::function<void(shared_ptr<Instance>)> resumeFunction, boost::function<void(std::string)> errorFunction);
	void checkParentWaitingForChildren();

	// Find the first ancestor of a given descendant
	shared_ptr<Instance> findFirstAncestorOf(const Instance* descendant) const;

	// TODO:  findFirstChildByName should return const Instance*

	const Instance* findConstFirstChildByName(const std::string& findName) const;
	Instance* findFirstChildByName(const std::string& findName) {
		return const_cast<Instance*>(findConstFirstChildByName(findName));
	}
	Instance* findFirstChildByNameDangerous(const std::string& findName) const {		// used by reflection - breaking from const to non const
		return const_cast<Instance*>(findConstFirstChildByName(findName));
	}

	// breadth-first search
	Instance* findFirstChildByNameRecursive(const std::string& findName);

	// Used for Reflection:
	shared_ptr<Instance> findFirstChildByName2(std::string findName, bool recursive) {
		return recursive ? shared_from(findFirstChildByNameRecursive(findName)) : shared_from(findFirstChildByName(findName));
	}

	// queryTypedChild
	template<class Type>
	Type* queryTypedChild(int index)	{
		return dynamic_cast<Type*>((*children)[index].get());
	}
	template<class Type>
	const Type* queryTypedChild(int index) const	{
		return dynamic_cast<const Type*>((*children)[index].get());
	}
	
	// getTypedChild
	template<class Type>
	Type* getTypedChild(int index)	{
		return rbx_static_cast<Type*>((*children)[index].get());
	}

	template<class Type>
	const Type* getTypedChild(int index) const	{
		return rbx_static_cast<const Type*>((*children)[index].get());
	}

	// queryTypedParent
	template<class Type>
	Type* queryTypedParent() {
		return dynamic_cast<Type*>(parent);
	}

	template<class Type>
	const Type* queryTypedParent() const {
		return dynamic_cast<const Type*>(parent);
	}

	// getTypedParent
	template<class Type>
	Type* getTypedParent() {
		return rbx_static_cast<Type*>(parent);
	}

	template<class Type>
	const Type* getTypedParent() const {
		return rbx_static_cast<const Type*>(parent);
	}

	// getTypedRoot
	template<class Type>
	Type* getTypedRoot() {
		RBXASSERT(dynamic_cast<Type*>(this));
		if (Type* typedParent = dynamic_cast<Type*>(parent)) {
			return typedParent->template getTypedRoot<Type>();
		}
		else {
			return static_cast<Type*>(this);
		}
	}
	template<class Type>
	const Type* getTypedRoot() const {
		RBXASSERT(dynamic_cast<const Type*>(this));
		if (const Type* typedParent = dynamic_cast<const Type*>(parent)) {
			return typedParent->template getTypedRoot<Type>();
		}
		else {
			return static_cast<const Type*>(this);
		}
	}

	const copy_on_write_ptr<Instances>& getChildren() const { return children; }

	// Used for reflection. Note that it might return NULL (or an empty container)
	// TODO - this is dangerous?  getting const children?
	shared_ptr<const Instances> getChildren2() { return children.read(); }		

	template<class Func>
	inline void visitChildren(const Func& func) const {
		if (children)
		{
			boost::shared_ptr<const Instances> c(children.read());
			Instances::const_iterator end = c->end();
			for (Instances::const_iterator iter = c->begin(); iter!=end; ++iter)
			{
				const_cast<Func&>(func)(*iter);
			}
		}
	}

	template<class Type>
	inline int countDescendantsOfType() const {
		int total = fastDynamicCast<Type>() ? 1 : 0;
		if (children)
		{
			boost::shared_ptr<const Instances> c(children.read());
			Instances::const_iterator end = c->end();
			for (Instances::const_iterator iter = c->begin(); iter!=end; ++iter)
			{
				total += (*iter)->countDescendantsOfType<Type>();
			}
		}
		return total;
	}

	template<class Func>
	inline void visitDescendants(const Func& func) const {
		if (children)
		{
			boost::shared_ptr<const Instances> c(children.read());
			Instances::const_iterator end = c->end();
			for (Instances::const_iterator iter = c->begin(); iter!=end; ++iter)
			{
				const_cast<Func&>(func)(*iter);
				(*iter)->visitDescendants(func);
			}
		}
	}

	template<class C>
	const C* findConstFirstChildOfType() const
	{
		if (children)
		{
			Instances::const_iterator end = children->end();
			for (Instances::const_iterator iter = children->begin(); iter!=end; ++iter) {
				const C* c = fastDynamicCast<C>(iter->get());
				if (c!=NULL)
					return c;
			}
		}
		return NULL;
	}

	template<class C>
	C* findFirstChildOfType()
	{
		return const_cast<C*>(findConstFirstChildOfType<C>());
	}

	Instance* findFirstChildOfType(const std::string& className)
	{
		if (children)
		{
			Instances::const_iterator end = children->end();
			for (Instances::const_iterator iter = children->begin(); iter!=end; ++iter) {
				if(iter->get()->getClassNameStr() == className)
					return iter->get();
			}
		}
		return NULL;
	}

	template<class C>
	C* findFirstDescendantOfType()
	{
		if (children)
		{
			Instances::const_iterator end = children->end();
			for (Instances::const_iterator iter = children->begin(); iter!=end; ++iter) {
				Instance* i = iter->get();
				if (C* foundChild = fastDynamicCast<C>(i)) {
					return foundChild;
				}
				if (C* foundDesc = i->findFirstDescendantOfType<C>()) {
					return foundDesc;
				}
			}
		}
		return NULL;
	}

	template<class C>
	const C* findConstFirstDescendantOfType() const
	{
		if (children)
		{
			Instances::const_iterator end = children->end();
			for (Instances::const_iterator iter = children->begin(); iter!=end; ++iter) {
				const Instance* i = iter->get();
				if (const C* foundChild = fastDynamicCast<C>(i)) {
					return foundChild;
				}
				if (const C* foundDesc = i->findConstFirstDescendantOfType<C>()) {
					return foundDesc;
				}
			}
		}
		return NULL;
	}

	template<class C>
	void destroyDescendantsOfType()
	{
		// Snapshot first (remove() precedent): destroy() unparents through
		// the LIVE list, which would invalidate iterators mid-loop
		// (s427 postmortem: direct iteration AV'd with 3 children).
		boost::shared_ptr<const Instances> c(children.read());
		if (c)
		{
			for (Instances::const_iterator iter = c->begin(); iter != c->end(); ++iter) {
				if (const C* c = fastDynamicCast<C>(iter->get()))
				{
					iter->get()->destroy();
				} else {
					iter->get()->destroyDescendantsOfType<C>();
				}
			}
		}
	}

	static Instance* findCommonNode(Instance* i1, Instance* i2)
	{
		if (i1 == i2)
			return i1;

		if (!i1)
			return NULL;
		if (i1->isAncestorOf(i2))
			return i1;
		
		if (!i2)
			return NULL;
		if (i2->isAncestorOf(i1))
			return i2;
		
		return findCommonNode(i1->getParent(), i2->getParent());
	}

	// Override with class-specific rules about what can be a child or parent
	bool canAddChild(const Instance* instance, bool checkParent = true) const {
		if (instance->contains(this))
			return false;		// No circular ownership, please!
		if (checkParent && instance->getParent() == this)
			return false;		// Already a child!
		if (this->askForbidChild(instance))
			return false;
		if (instance->askForbidParent(this))
			return false;
		if (this->askAddChild(instance))
			return true;
		if (instance->askSetParent(this))
			return true;
		return false;
	}
	bool canAddChild(const shared_ptr<Instance>& instance) const {
		return (instance!=NULL) && canAddChild(instance.get());
	}
	bool canSetParent(const Instance* instance) const {
		return (instance==NULL) || instance->canAddChild(this);
	}

	template<typename Iter>
	bool canSetChildren(Iter first, Iter last) const
	{
		for (; first != last; ++first)
		{
			if (!canAddChild(*first))
				return false;
		}
		return true;
	}

	template<typename Iter>
	void setChildren(Iter first, Iter last)
	{
		for (; first != last; ++first)
		{
			Instance* instance = *first;
			RBXASSERT(canAddChild(instance));
			instance->setParent(this);
		}
	}

	virtual bool canClientCreate() { return false; }
	// This convenience function gets called when this Instances is added to or removed from the DataModel.
	// You can use it to find Services, connect to and disconnect from Notifiers, etc.
	virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider) {}
	
	void readProperties(const XmlElement* container, IReferenceBinder& binder);

	virtual shared_ptr<Instance> createChild(const RBX::Name& className, RBX::CreatorRole creatorRole);
	void read(const XmlElement* element, IReferenceBinder& binder, RBX::CreatorRole creatorRole);
	void readChildren(const XmlElement* element, IReferenceBinder& binder, RBX::CreatorRole creatorRole);
	void readChild(const XmlElement* childElement, IReferenceBinder& binder, RBX::CreatorRole creatorRole);

	virtual XmlElement* writeXml(const boost::function<bool(Instance*)>& isInScope, RBX::CreatorRole creatorRole);

	void writeChildren(XmlElement* container, const boost::function<bool(Instance*)>& isInScope, RBX::CreatorRole creatorRole, const SaveFilter saveFilter = SAVE_ALL);
	void writeChildren(XmlElement* container, RBX::CreatorRole creatorRole, const SaveFilter saveFilter = SAVE_ALL);

	void raisePropertyChanged(const RBX::Reflection::PropertyDescriptor& descriptor);
	void raiseEventInvocation(const RBX::Reflection::EventDescriptor& descriptor, const RBX::Reflection::EventArguments& args, const SystemAddress* target);

	//////////////////////////////////////////////////////////////////////////////////////////

protected:
	//Actively prevent the adding
	virtual void verifySetParent(const Instance* newParent) const {};
	virtual void verifySetAncestor(const Instance* const newParent, const Instance* const instanceGettingNewParent) const;
	//Actively prevent the adding (parent side)
	virtual void verifyAddChild(const Instance* newChild) const {};
	virtual void verifyAddDescendant(const Instance* newParent, const Instance* instanceGettingNewParent) const;

	// Don't call this directly. Call canAddChild
	virtual bool askAddChild(const Instance* instance) const;
	// Don't call this directly. Call canAddChild
	virtual bool askForbidChild(const Instance* instance) const;

	// Don't call this directly. Call canSetParent
	virtual bool askForbidParent(const Instance* instance) const;
	// Don't call this directly. Call canSetParent
	virtual bool askSetParent(const Instance* instance) const;
	

	virtual void onAncestorChanged(const AncestorChanged& event);

	virtual void onDescendantAdded(Instance* instance);
	virtual void onDescendantRemoving(const shared_ptr<Instance>& instance);

	virtual void onChildAdded(Instance* child) {}
	virtual void onChildRemoving(Instance* child) {}
	virtual void onChildRemoved(Instance* child) {}
	virtual void onChildChanged(Instance* instance, const PropertyChanged& event);

	virtual void onPropertyChanged(const Reflection::PropertyDescriptor& descriptor)	 {}
	


	virtual void readProperty(const XmlElement* propertyElement, IReferenceBinder& binder);

	void raiseChanged(const RBX::Reflection::PropertyDescriptor& descriptor)
	{
		raisePropertyChanged(descriptor);
	}

private:
	static void signalDescendantAdded(Instance* instance, Instance* beginParent, Instance* endParent);
	static void signalDescendantRemoving(const shared_ptr<Instance>& instance, Instance* beginParent, Instance* endParent);

	void writeProperties(XmlElement* container) const;
	bool setParentInternal(Instance* instance, bool ignoreLock);
};

} // namespace RBX
