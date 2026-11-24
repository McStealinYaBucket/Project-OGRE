/* App/v8datamodel/StatsItem.cpp -- October 2013 (child factories + display).
   Non-template create* + display bodies (batches 609/611). Template
   create* bodies live in TypedStatsItem.h. TODO: full decompile
   (formatMem needs RBX::Log::formatMem dump). */

#include "stdafx.h"

#include "V8DataModel/StatsItem.h"
#include "V8DataModel/TypedStatsItem.h"

#include "rbx/Log.h"

#include "RbxFormat.h"

#include <cstdarg>

namespace RBX {
namespace Stats {
	const char* const sStatsItem = "StatsItem";
}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Stats::Item);
}

namespace RBX {

	// mac: 0x4A2F5A.
	StatsItem* StatsItem::createChildItem(const char* name)
	{
		boost::shared_ptr<StatsItem> child(new StatsItem());
		child->setName(name);
		child->setParent(this);
		return child.get();
	}

	// createBoundChildItem<double,double> 0x4A3478.
	RunningAverageItemDouble* StatsItem::createBoundChildItem(const char* name,
		const RunningAverage<double, double>& average)
	{
		boost::shared_ptr<RunningAverageItemDouble> child(new RunningAverageItemDouble(&average));
		child->setName(name);
		child->setParent(this);
		return child.get();
	}

	// getValue 0x4A3FBC: refresh, then read +0x5C.
	double StatsItem::getValue()
	{
		update();
		return statValue;
	}

	// getStringValue2 0x4A3F8C: refresh, then read the +0x64 display.
	std::string StatsItem::getStringValue2()
	{
		update();
		return displayString;
	}

	// formatMem 0x4A2D80: Log::formatMem into the display (no value store).
	// STUB: PLAYER 0x0073b300
	// STUB: STUDIO 0x007c39a0
	void StatsItem::formatMem(unsigned int bytes)
	{
		displayString.assign(RBX::Log::formatMem(bytes));
	}

	// formatValue(double,fmt,...) 0x4A2EA8: store +0x60, vformat into +0x68.
	// STUB: PLAYER 0x0073b380
	// STUB: STUDIO 0x007c3a20
	void StatsItem::formatValue(double value, const char* fmt, ...)
	{
		statValue = value;
		va_list args;
		va_start(args, fmt);
		displayString.assign(RBX::vformat(fmt, args));
		va_end(args);
	}

	// formatValue<double> 0x4A305A ("%.3g").
	// FUNCTION: PLAYER 0x0073b3f0
	// FUNCTION: STUDIO 0x007c3a90
	template<>
	void StatsItem::formatValue<double>(const double& value)
	{
		formatValue(value, "%.3g", value);
	}

	// formatValue<float> 0x4A3094 ("%.3g" of the widened float).
	// FUNCTION: PLAYER 0x0073b420
	// FUNCTION: STUDIO 0x007c3ac0
	template<>
	void StatsItem::formatValue<float>(const float& value)
	{
		double wide = value;
		formatValue(wide, "%.3g", wide);
	}

	// formatValue<unsigned long> 0x4A3108 ("%d", unsigned-long flavor).
	template<>
	void StatsItem::formatValue<unsigned long>(const unsigned long& value)
	{
		formatValue((double)value, "%d", (int)value);
	}

	// formatValue<int> 0x4A30CE ("%d").
	// FUNCTION: PLAYER 0x0073b450
	// FUNCTION: STUDIO 0x007c3af0
	template<>
	void StatsItem::formatValue<int>(const int& value)
	{
		formatValue((double)value, "%d", value);
	}

	// formatValue<unsigned long long> 0x4A3152 ("%llu", PLAYER string 0x00b50b30).
	// FUNCTION: PLAYER 0x0073b480
	// FUNCTION: STUDIO 0x007c3b20
	template<>
	void StatsItem::formatValue<unsigned long long>(const unsigned long long& value)
	{
		formatValue((double)value, "%llu", value);
	}

	// formatValue<unsigned int> 0x4A31A6 ("%d" of the raw value; the double gets 2^32 added when negative).
	// FUNCTION: PLAYER 0x0073b4e0
	// FUNCTION: STUDIO 0x007c3b80
	template<>
	void StatsItem::formatValue<unsigned int>(const unsigned int& value)
	{
		formatValue((double)value, "%d", value);
	}

	// formatValue<bool> 0x4A31F0: true/false display + 1.0/0.0 store
	// (does NOT route through the core).
	// STUB: PLAYER 0x0073b510
	// STUB: STUDIO 0x007c3bb0
	template<>
	void StatsItem::formatValue<bool>(const bool& value)
	{
		displayString.assign(value ? "true" : "false");
		statValue = value ? 1.0 : 0.0;
	}

	// formatRate 0x4A2E40: rate + CV math + "%.1f/s (%.0f%%CV)".
	// The CV reads assume [+0x2C]=variance/mean pair (i.e. our
	// coefficient_of_variation); C26EB8=100.0 VERIFIED by batch621
	// rodata read (percent scaling).
	// STUB: PLAYER 0x0073c670
	// STUB: STUDIO 0x007c4ba0
	void StatsItem::formatRate(const RunningAverageTimeInterval<>& average)
	{
		double rate = average.rate();
		double cv = average.coefficient_of_variation() * 100.0;
		formatValue(rate, "%.1f/s (%.0f%%CV)", rate, cv);
	}

	// Item::update: refresh the parent item, if it is one.
	// FUNCTION: PLAYER 0x0073ec00
	// FUNCTION: STUDIO 0x007c6b50
	void StatsItem::update()
	{
		if (Item* parent = fastDynamicCast<Item>(getParent()))
			parent->update();
	}

} // namespace RBX
