/* v8datamodel/ChangeHistory.cpp -- October 2013 (named by its asserts, e.g. "action==Create" at line 267).
   Only the EnumDesc<ChangeHistoryService::RuntimeUndoBehavior> ctor is written so far. */

#include "stdafx.h"

#include "V8DataModel/ChangeHistoryService.h"

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00836010
// STUB: STUDIO 0x006d57d0
template<>
EnumDesc<ChangeHistoryService::RuntimeUndoBehavior>::EnumDesc()
	: EnumDescriptor("RuntimeUndoBehavior", typeid(ChangeHistoryService::RuntimeUndoBehavior))
{
	addPair(ChangeHistoryService::Aggregate, "Aggregate");
	addPair(ChangeHistoryService::Snapshot, "Snapshot");
	addPair(ChangeHistoryService::Hybrid, "Hybrid");
}

template<>
const Type& Type::getSingleton<ChangeHistoryService::RuntimeUndoBehavior>()
{
	return EnumDesc<ChangeHistoryService::RuntimeUndoBehavior>::singleton();
}

template<> EnumRegistrar<ChangeHistoryService::RuntimeUndoBehavior> EnumRegistrar<ChangeHistoryService::RuntimeUndoBehavior>::registrar(0);
template<> TypeRegistrar<ChangeHistoryService::RuntimeUndoBehavior> TypeRegistrar<ChangeHistoryService::RuntimeUndoBehavior>::registrar(0);

} // namespace Reflection
} // namespace RBX
