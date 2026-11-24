/* V8DataModel/StatsItem.h -- October 2013 (Stats::Item, flattened).
   Evidence: SIMPLIFIED NonFactoryProduct<Stats::Item, Instance> 0x51A120 (flattened from Stats ns; doDeclare 0x1A8FF0);
     sStatsItem = "StatsItem". Item C2 0x1AD148 (batch609): Instance::C2 +
     +0x5C double (zero) + display string +0x64 (empty) (+0x30 descriptor
     cache skipped: our descriptor is static; vtable secondaries skipped
     per the MSVC-MI rule). createChildItem(char*) 0x4A2F5A /
     createChildItem<int>(fn) 0x1ACCB8 / createBoundChildItem<bool> /
     <int> / <double,double avg> (batch609: factory-create the typed
     child + setName + setParent (public setParentInternal forward);
     factory replaced by direct new+shared -- same observable, no
     per-instantiation registrar needed).
   Display: formatValue(double,fmt,...) stores +0x60 and vformats into +0x68; the
     formatValue<T> specializations forward (%.3g/%.3g/%d/%llu/%d); <bool> writes
     true/false + 0.0/1.0; formatRate (rate + CV, "%.1f/s (%.0f%%CV)"). update()
     refreshes the parent Item (PLAYER 0x0073ec00); TypedStatsItem<T> overrides it.
     Layout (2013 Item ctor PLAYER 0x0048e470): double 0x60, string 0x68, size 0x88.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StatsItem.h */

#pragma once

#include "V8Tree/Instance.h"

#include "rbx/RunningAverage.h"

#include <boost/function.hpp>
#include <string>

namespace RBX {

	// 2013 names: RBX::Stats::Item and RBX::Stats::sStatsItem (RTTI
	// .?AV?$NonFactoryProduct@VInstance@RBX@@$E?sStatsItem@Stats@2@3QBDB@RBX@@,
	// .?AV?$DescribedNonCreatable@VItem@Stats@RBX@@...). RBX::StatsItem is kept as a typedef
	// for the users outside the Stats files.
	namespace Stats {
		extern const char* const sStatsItem;
		class Item;
		template<class T> class TypedStatsItem;
	}
	typedef Stats::Item StatsItem;

	class RunningAverageItemDouble;

namespace Stats {

	class Item
		: public DescribedNonCreatable<Item, Instance, sStatsItem>
	{
	public:
		// fldz into +0x60, then the +0x68 string (STUDIO's older revision differs).
		// FUNCTION: PLAYER 0x0048e470
		// STUB: STUDIO 0x0043e560
		Item()
			: statValue(0.0)
		{}

		// mac: 0x4A2F5A: factory-create a plain child + setName + setParent.
		Item* createChildItem(const char* name);
		// mac: 0x1ACCB8: same shape for a functor-fed TypedStatsItem<int>.
		template<class T>
		Stats::TypedStatsItem<T>* createChildItem(const char* name, boost::function<T()> getter);
		// createBoundChildItem<bool/int/...>: same shape bound to a live
		// reference (stored as pointer; getValue live-reads).
		template<class T>
		Stats::TypedStatsItem<T>* createBoundChildItem(const char* name, const T& value);
		// createBoundChildItem<double,double> 0x4A3478: same shape for a
		// RunningAverage-fed child (non-template overload -- MSVC-safe;
		// binary spells a (T,T0) template).
		RunningAverageItemDouble* createBoundChildItem(const char* name,
			const RunningAverage<double, double>& average);

		// update() (PLAYER 0x0073ec00) refreshes the parent Stats::Item; TypedStatsItem<T>
		// overrides it. getValue refreshes then reads +0x60; getStringValue2 refreshes then
		// reads +0x68.
		virtual void update();  // vtable+0x70, the only virtual Stats::Item adds (2013 vftable 0x00b01ba4, 29 slots)
		double getValue();  // not virtual in 2013 (no slot after update)
		std::string getStringValue2();
		void formatValue(double value, const char* fmt, ...);
		// ios: Item::formatValue<T>(T const&). Only explicit specializations exist
		// (StatsItem.cpp): double, float, int, unsigned long long, unsigned int, bool,
		// unsigned long.
		template<class T>
		void formatValue(const T& value);
		// formatMem 0x4A2D80: Log::formatMem into +0x68 (no +0x60 store).
		void formatMem(unsigned int bytes);
		void formatRate(const RunningAverageTimeInterval<>& average);

	protected:
		// vtable+0x3c: only Stats::Items may be children.
		// FUNCTION: PLAYER 0x0048e4e0
		// FUNCTION: STUDIO 0x00443f80
		/*override*/ virtual bool askAddChild(const Instance* instance) const
		{
			return fastDynamicCast<Item>(instance) != NULL;
		}

	private:
		double statValue;            // 0x60 (formatValue stores; getValue reads)
		std::string displayString;   // 0x68 (format* writes; getStringValue2 reads)
	};                               // SIZE 0x88 (TypedStatsItem<int> is new 0xa8, function at 0x88)

} // namespace Stats
} // namespace RBX

// Template bodies live in TypedStatsItem.h (included here, after the
// class is complete -- StatsItem.h cannot include it up top without a
// cycle, since TypedStatsItem derives from StatsItem).
#include "V8DataModel/TypedStatsItem.h"
