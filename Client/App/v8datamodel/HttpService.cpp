/* App/v8datamodel/HttpService.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/HttpService.h"

namespace RBX {
	const char* const sHttpService = "HttpService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::HttpService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00916eb0
// FUNCTION: STUDIO 0x007d4540
template<>
EnumDesc<HttpService::HttpContentType>::EnumDesc()
	: EnumDescriptor("HttpContentType", typeid(HttpService::HttpContentType))
{
	addPair(HttpService::APPLICATION_JSON, "ApplicationJson");
	addPair(HttpService::TEXT_PLAIN, "TextPlain");
}

template<>
const Type& Type::getSingleton<HttpService::HttpContentType>()
{
	return EnumDesc<HttpService::HttpContentType>::singleton();
}

template<> EnumRegistrar<HttpService::HttpContentType> EnumRegistrar<HttpService::HttpContentType>::registrar(0);
template<> TypeRegistrar<HttpService::HttpContentType> TypeRegistrar<HttpService::HttpContentType>::registrar(0);

} // namespace Reflection
} // namespace RBX
