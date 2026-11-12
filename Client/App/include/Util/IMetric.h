/* Util/IMetric.h -- October 2013.
   Evidence: DataModel's RTTI (vftable 0x00b2ffa4 for `RBX::IMetric`, base at DataModel+0xa4)
   and the iOS 10-29 DataModel vtable group at -180: dtor, getMetric(std::string const&) const,
   getMetricValue(std::string const&) const. Same shape as the 2016 header of this name.
   Original file: Client/App/include/Util/IMetric.h (2016 path; 2013 records none). */

#pragma once

#include <string>

namespace RBX {

	// SIZE 0x4
	class RBXInterface IMetric
	{
	public:
		IMetric() {}
		virtual ~IMetric() {}														// vtable+0x00

		virtual std::string getMetric(const std::string& metric) const = 0;		// vtable+0x04
		virtual double getMetricValue(const std::string& metric) const = 0;		// vtable+0x08
	};

} // namespace RBX
