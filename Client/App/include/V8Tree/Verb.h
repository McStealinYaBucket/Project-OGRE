/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

#pragma once

#include <map>
#include <boost/function.hpp>

#include "rbx/Debug.h"
#include "security/FuzzyTokens.h"
#include "v8datamodel/HackDefines.h"

LOGGROUP(Verbs)

namespace RBX {

class Name;
class VerbContainer;

class RBXInterface IDataState
{
public:
	// Sets the document's "dirty" bit
	virtual void setDirty(bool dirty) = 0;
	virtual bool isDirty() const = 0;
};

// 2013 (iOS 10-29 vtable __ZTVN3RBX4VerbE: Debugable::dump first): Verb derives from
// Debugable, so its vftable is dump, dtor, isEnabled, isChecked (+0xc), isSelected,
// getText, doIt (+0x18) -- DataModel::close 0x0062d7b0 and toggleToolsOff 0x0062da7c call
// +0xc/+0x18. Fields: vfptr @0, name @4, container @8 (VerbContainer::removeVerb
// 0x006f9190 reads verb+4, ~Verb reads verb+8). 2013 has only the
// (VerbContainer*, std::string const&) ctor and no whitelist/blacklist or verbSecurity.
class RBXBaseClass Verb : public Debugable
{
	const Name& name;					// 0x04
protected:
	VerbContainer* const container;		// 0x08
	Verb(VerbContainer* container, const std::string& name);
public:
	virtual ~Verb();
	virtual bool isEnabled() const { return true; }
	virtual bool isChecked() const { return false; }
	virtual bool isSelected() const { return false; }

	virtual std::string getText() const { return std::string(); }
	const Name& getName() const {return name;}

	virtual void doIt(IDataState* dataState) = 0;

	VerbContainer* getContainer() const { return container; }

    static inline void doItWithChecks(Verb* const verb, IDataState* dataState)
    {
        /* 2013: no HATE/hackFlag system (no getVerbSecurity/RBX_BUILDSEED in the Oct 2013 binaries) */
        {
            verb->doIt(dataState);
        }
    }
};

// This version of Verb lets you pass in callbacks for the implementation
class BoundVerb : public Verb
{
	boost::function<void(VerbContainer*)> doItFunction;
	boost::function<bool(VerbContainer*)> isEnabledFunction;
	boost::function<bool(VerbContainer*)> isCheckedFunction;
	boost::function<std::string(VerbContainer*)> getTextFunction;

public:
	BoundVerb(VerbContainer* container, const char* name,
		boost::function<void(VerbContainer*)> doItFunction,
		boost::function<bool(VerbContainer*)> isEnabledFunction = NULL,
		boost::function<bool(VerbContainer*)> isCheckedFunction = NULL,
		boost::function<std::string(VerbContainer*)> getTextFunction = NULL)
		: Verb(container, name)
		, doItFunction(doItFunction)
		, isEnabledFunction(isEnabledFunction)
		, isCheckedFunction(isCheckedFunction)
		, getTextFunction(getTextFunction)
	{
	}

	virtual bool isEnabled() const
	{
		if (!isEnabledFunction)
			return Verb::isEnabled();
		return isEnabledFunction(container);
	}

	virtual bool isChecked() const
	{
		if (!isCheckedFunction)
			return Verb::isChecked();
		return isCheckedFunction(container);
	}

	virtual std::string getText() const
	{
		if (!getTextFunction)
			return Verb::getText();
		return getTextFunction(container);
	}

	virtual void doIt(IDataState* dataState)
	{
		if (doItFunction)
			doItFunction(container);
	}

};

class NullVerb : public Verb
{
public:
	NullVerb(VerbContainer* container, const std::string& name)
		: Verb(container, name)
	{
	}

	/*override*/ bool isEnabled() const { return false; }
	/*override*/ void doIt(IDataState* dataState) {}
};

// 2013 layout (ctor PLAYER 0x006f9200): vfptr @0, one std::map verbs @0x4, parent @0x10;
// size 0x14 (DataModel's IDataState base follows it at 0x8c + 0x14 = 0xa0). 2016 split the
// map into whitelist and blacklist; 2013 has addVerb 0x006f9270 / removeVerb 0x006f9190.
// SIZE 0x14
class VerbContainer
{
	friend class Verb;
	typedef std::map<const Name*, Verb*> Verbs;
	Verbs verbs;				// 0x04
	VerbContainer* parent;		// 0x10

	void addVerb(Verb* verb);
	void removeVerb(Verb* verb);

public:
	VerbContainer(VerbContainer* parent);
	virtual ~VerbContainer();
	Verb* getVerb(const Name& name);
	Verb* getVerb(const std::string& name);
	void setVerbParent(VerbContainer* parent);
	VerbContainer* getVerbParent() const { return parent; }

	template<class F>
	void eachVerbName(F f, bool includeParent = true)
	{
		for (Verbs::const_iterator iter = verbs.begin(); iter != verbs.end(); ++iter)
			f(iter->first);
		if (includeParent && parent)
			parent->eachVerbName(f);
	}
};

}  // namespace RBX
