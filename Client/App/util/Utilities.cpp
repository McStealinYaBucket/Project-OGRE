/* App/util/Utilities.cpp — October 2013 primitive StringConverter specializations.
   Ported from March 2016 App/util/Utilities.cpp (bool/int/uint/float/double bodies;
   INF/-INF/NAN handling, %d/%u/%.20g/%.9g formats — version-neutral primitives).
   TODO: verify against 2013 EAs; remaining Utilities.cpp contents (Http CSRF, rot13,
   string converters, isCamel home) pending. */

#include "stdafx.h"

#include "Reflection/Type.h"
#include "Util/utilities.h"
#include "RbxAssert.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <limits>
#include <cstdlib>

namespace RBX {

	template<>
	std::string StringConverter<bool>::convertToString(const bool& value)
	{
		return value ? "true" : "false";
	}

	template<>
	bool StringConverter<bool>::convertToValue(const std::string& text, bool& value)
	{
		if (text=="true" || text=="True" || text=="TRUE")
			value = true;
		else if (text=="false" || text=="False" || text=="FALSE")
			value = false;
		else
			return false;
		return true;
	}

	template<>
	std::string StringConverter<int>::convertToString(const int& value)
	{
		char szText[16];
		_snprintf(szText, 16, "%d", value);
		return szText;
	}

	template<>
	bool StringConverter<int>::convertToValue(const std::string& text, int& value)
	{
		if (text.size()==0) {
			return false;
		}
		else {
			const char* s = text.c_str();
			for (int i = (int)text.size() - 1; i >= 0; --i)
			{
				char c = s[i];
				if (i == 0 && c == '-')
					continue;
				if (c > '9')
					return false;
				if (c < '0')
					return false;
			}
			value = atoi(s);
			return true;
		}
	}

	template<>
	std::string StringConverter<unsigned int>::convertToString(const unsigned int& value)
	{
		char szText[16];
		_snprintf(szText, 16, "%u", value);
		return szText;
	}

	template<>
	bool StringConverter<unsigned int>::convertToValue(const std::string& text, unsigned int& value)
	{
		if (text.size()==0)
			return false;
		try
		{
			value = boost::lexical_cast<unsigned int>(text);
		}
		catch(boost::bad_lexical_cast &)
		{
			return false;
		}
		return true;
	}

	template<>
	std::string StringConverter<long>::convertToString(const long& value)
	{
		char szText[16];
		_snprintf(szText, 16, "%ld", value);
		return szText;
	}

	template<>
	bool StringConverter<long>::convertToValue(const std::string& text, long& value)
	{
		if (text.size()==0)
			return false;
		try
		{
			value = boost::lexical_cast<long>(text);
		}
		catch(boost::bad_lexical_cast &)
		{
			return false;
		}
		return true;
	}

	template<>
	bool StringConverter<double>::convertToValue(const std::string& text, double& value)
	{
		if (text.size()==0)
			return false;
		if (text=="INF")
			value = std::numeric_limits<double>::infinity();
		else if (text=="-INF")
			value = -std::numeric_limits<double>::infinity();
		else if (text=="NAN")
			value = std::numeric_limits<double>::quiet_NaN();
		else
			value = atof(text.c_str());
		return true;
	}

	template<>
	std::string StringConverter<double>::convertToString(const double& value)
	{
		if (value==std::numeric_limits<double>::infinity())
			return "INF";
		if (value==-std::numeric_limits<double>::infinity())
			return "-INF";
		if (!((value < 0.0) || (value >= 0.0)))
			return "NAN";

		char szText[64];
		_snprintf(szText, 64, "%.20g", value);
		return szText;
	}

	template<>
	bool StringConverter<float>::convertToValue(const std::string& text, float& value)
	{
		if (text.size()==0)
			return false;
		if (text=="INF")
			value = std::numeric_limits<float>::infinity();
		else if (text=="-INF")
			value = -std::numeric_limits<float>::infinity();
		else if (text=="NAN")
			value = std::numeric_limits<float>::quiet_NaN();
		else
		{
			char* endPtr;
			value = static_cast<float>(::strtod(text.c_str(), &endPtr));
		}
		return true;
	}

	template<>
	std::string StringConverter<float>::convertToString(const float& value)
	{
		if (value==std::numeric_limits<float>::infinity())
			return "INF";
		if (value==-std::numeric_limits<float>::infinity())
			return "-INF";
		if (!((value < 0.0) || (value >= 0.0)))
			return "NAN";

		char szText[32];
		_snprintf(szText, 32, "%.9g", value);
		return szText;
	}

	bool isCamel(const char* name)
	{
		std::string temp(name);
		boost::to_upper(temp);
		return std::string(name)[0] != temp[0];
	}

} // namespace RBX
