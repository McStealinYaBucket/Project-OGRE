/* V8DataModel/TypedStatsItem.h -- October 2013 (bound/function stat items).
   Evidence: createChildItem<int>(fn) 0x1ACCB8 news TypedStatsItem<int>
     via Creatable+functor; createBoundChildItem<bool/int>(ptr) news
     TypedStatsItem<T> via Creatable+pointer; createBound(double,double)
     news RunningAverageItemDouble via Creatable+avg-ptr (batch609: all
     three then setName + setParentInternal(false)). No C2 symbols
     DB-wide (header-inline template C2s, inlined at the create sites).
   getValue live-reads the binder (no update loop exists in-tree yet;
   caches would read stale -- the display/update half queues with
   LuaStatsItem::update).
   Factory replaced by direct new+shared (same observable, no
   per-instantiation registrar). Template bodies for StatsItem::create*
   live here (see StatsItem.h note).
   TODO: full decompile (update/display/format*). */

#pragma once

#include "V8DataModel/StatsItem.h"
#include "rbx/RunningAverage.h"

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace RBX {
namespace Stats {

	template<class T>
	class TypedStatsItem : public StatsItem
	{
	public:
		explicit TypedStatsItem(boost::function<T()> getter)
			: getter(getter)
			, bound(NULL)
		{}
		explicit TypedStatsItem(const T* bound)
			: bound(bound)
		{}
		T getValue() const
		{
			if (bound != NULL)
				return *bound;
			if (getter)
				return getter();
			return T();
		}
	private:
		boost::function<T()> getter;
		const T* bound;
	};

} // namespace Stats

	class RunningAverageItemDouble : public StatsItem
	{
	public:
		explicit RunningAverageItemDouble(const RunningAverage<double, double>* average)
			: average(average)
		{}
		double getValue() const
		{
			return average != NULL ? average->value() : 0.0;
		}
	private:
		const RunningAverage<double, double>* average;
	};

	// StatsItem::create* template bodies (batch609 uniform shape:
	// new+shared, setName, setParent(this), return raw
	// -- the parent retains via its children vector). Non-template
	// bodies live in StatsItem.cpp.

	template<class T>
	Stats::TypedStatsItem<T>* StatsItem::createChildItem(const char* name, boost::function<T()> getter)
	{
		boost::shared_ptr<Stats::TypedStatsItem<T> > child(new Stats::TypedStatsItem<T>(getter));
		child->setName(name);
		child->setParent(this);
		return child.get();
	}

	template<class T>
	Stats::TypedStatsItem<T>* StatsItem::createBoundChildItem(const char* name, const T& value)
	{
		boost::shared_ptr<Stats::TypedStatsItem<T> > child(new Stats::TypedStatsItem<T>(&value));
		child->setName(name);
		child->setParent(this);
		return child.get();
	}

} // namespace RBX
