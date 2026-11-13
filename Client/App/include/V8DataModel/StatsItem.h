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
   Display (batch611): formatValue(double,fmt,...) 0x4A2EA8 stores
     +0x5C + vformats into +0x64; typed Id/If/Ii/Im/Iy/Ij forward
     (%.3g/%.3g/%d/%d/%llu/%d + converted double); Ib writes
     true/false + 0.0/1.0; formatRate(avg) 0x4A2E40 (rate + CV math +
     "%.1f/s (%.0f%%CV)"); getStringValue2 0x4A3F8C (refresh + read
     display); getValue 0x4A3FBC (refresh + read +0x5C); update base
     forwards to the +0x34 delegate (no writer evidenced -- always
     NULL in-tree). formatMem queues (needs RBX::Log::formatMem dump).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StatsItem.h */

#pragma once

#include "V8Tree/Instance.h"

#include "rbx/RunningAverage.h"

#include <boost/function.hpp>
#include <string>

namespace RBX {

	extern const char* const sStatsItem;

	namespace Stats { template<class T> class TypedStatsItem; }

	class RunningAverageItemDouble;

	class StatsItem
		: public DescribedNonCreatable<StatsItem, Instance, sStatsItem>
	{
	public:
		StatsItem()
			: statValue(0.0)
			, delegate(NULL)
		{}

		// mac: 0x4A2F5A: factory-create a plain child + setName + setParent.
		StatsItem* createChildItem(const char* name);
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

		// Display (batch611). update() forwards to the +0x34 delegate
		// (always NULL in-tree: no writer evidenced); getValue refreshes
		// then reads +0x5C; getStringValue2 refreshes then reads +0x64.
		virtual void update();  // vtable+0x70, the only virtual Stats::Item adds (2013 vftable 0x00b01ba4, 29 slots)
		double getValue();  // not virtual in 2013 (no slot after update)
		std::string getStringValue2();
		void formatValue(double value, const char* fmt, ...);
		void formatValueId(const double& value);
		void formatValueIf(const float& value);
		void formatValueIi(const int& value);
		void formatValueIm(unsigned long value);
		void formatValueIy(const unsigned long long& value);
		void formatValueIj(unsigned int value);
		void formatValueIb(const bool& value);
		// formatMem 0x4A2D80: Log::formatMem into +0x64 (no +0x5C store).
		void formatMem(unsigned int bytes);
		void formatRate(const RunningAverageTimeInterval<>& average);

	protected:
		// vtable+0x3c: only Stats::Items may be children.
		// FUNCTION: PLAYER 0x0048e4e0
		// FUNCTION: STUDIO 0x00443f80
		/*override*/ virtual bool askAddChild(const Instance* instance) const
		{
			return fastDynamicCast<StatsItem>(instance) != NULL;
		}

	private:
		// +0x5C role: numeric value (formatValue stores; getValue reads).
		double statValue;
		// +0x64 role: display string (format* writes; getStringValue2 reads).
		std::string displayString;
		// +0x34 role: refresh delegate (no writer evidenced; NULL).
		StatsItem* delegate;
	};

} // namespace RBX

// Template bodies live in TypedStatsItem.h (included here, after the
// class is complete -- StatsItem.h cannot include it up top without a
// cycle, since TypedStatsItem derives from StatsItem).
#include "V8DataModel/TypedStatsItem.h"
