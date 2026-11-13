/* App/v8datamodel/MarketplaceService.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/MarketplaceService.h"

namespace RBX {
	const char* const sMarketplaceService = "MarketplaceService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::MarketplaceService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00807250
// FUNCTION: STUDIO 0x008f6320
template<>
EnumDesc<MarketplaceService::CurrencyType>::EnumDesc()
	: EnumDescriptor("CurrencyType", typeid(MarketplaceService::CurrencyType))
{
	addPair(MarketplaceService::CURRENCY_DEFAULT, "Default");
	addPair(MarketplaceService::CURRENCY_ROBUX, "Robux");
	addPair(MarketplaceService::CURRENCY_TIX, "Tix");
}

template<>
const Type& Type::getSingleton<MarketplaceService::CurrencyType>()
{
	return EnumDesc<MarketplaceService::CurrencyType>::singleton();
}

template<> EnumRegistrar<MarketplaceService::CurrencyType> EnumRegistrar<MarketplaceService::CurrencyType>::registrar(0);
template<> TypeRegistrar<MarketplaceService::CurrencyType> TypeRegistrar<MarketplaceService::CurrencyType>::registrar(0);

} // namespace Reflection
} // namespace RBX
