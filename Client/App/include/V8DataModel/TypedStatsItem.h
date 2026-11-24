/* V8DataModel/TypedStatsItem.h -- October 2013 (function-fed stat items).
   2013 TypedStatsItem<T> holds only a boost::function<T()> at +0x88 (new 0xa8 in the
   TypedStatsItem<int> creator PLAYER 0x0048e840; sizeof(boost::function0<int>) is 0x20 under
   VC9) and overrides update() (PLAYER 0x0048e7a0 int, 0x0048e7d0 bool, 0x0048e810 double):
   call the function, hand the value to Item::formatValue<T>. A bound child (ios:
   Creatable<Instance>::create<TypedStatsItem<bool>, bool const*>) binds the static deref
   (ios: TypedStatsItem<T>::deref(T const*), PLAYER folded with `mov eax,[esp+4]; ret`
   0x004a20c0) into that function. The ctor from boost::function0<T> is ios-named.
   Factory replaced by direct new+shared (same observable, no per-instantiation registrar).
   Template bodies for StatsItem::create* live here (see StatsItem.h note). */

#pragma once

#include "V8DataModel/StatsItem.h"
#include "rbx/RunningAverage.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace RBX {
namespace Stats {

	template<class T>
	class TypedStatsItem : public Item
	{
	public:
		explicit TypedStatsItem(boost::function<T()> func)
			: func(func)
		{}
		explicit TypedStatsItem(const T* value)
			: func(boost::bind(&TypedStatsItem::deref, value))
		{}

		// clang-format off
		// TEMPLATE: PLAYER 0x0048e7a0
		// TEMPLATE: STUDIO 0x0043ef20
		// ?update@?$TypedStatsItem@H@Stats@RBX@@UAEXXZ

		// TEMPLATE: PLAYER 0x0048e7d0
		// TEMPLATE: STUDIO 0x0043ef50
		// ?update@?$TypedStatsItem@_N@Stats@RBX@@UAEXXZ
		// clang-format on
		/*override*/ virtual void update()
		{
			formatValue(func());
		}

	private:
		static const T& deref(const T* value)
		{
			return *value;
		}

		boost::function<T()> func;   // 0x88
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
