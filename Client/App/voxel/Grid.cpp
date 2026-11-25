/* voxel/Grid.cpp -- October 2013 (2013 __FILE__: Client\App\voxel\Grid.cpp, asserts at lines 39 and 131).
   Only the Region3int16 Type singleton is written so far: in both targets it is the last function
   of Grid.obj (PLAYER 0x00793b60 after the Grid code at 0x00792cb0..; its TType ctor 0x00793a90
   asserts "include\Reflection/Type.h", the spelling this file uses). The Grid class is not written. */

#include "stdafx.h"

#include "Reflection/Type.h"
#include "Util/Region3int16.h"

namespace RBX {
namespace Reflection {

	// FUNCTION: PLAYER 0x00793b60
	// FUNCTION: STUDIO 0x00847020
	template<>
	const Type& Type::getSingleton<RBX::Region3int16>()
	{
		static TType<RBX::Region3int16> type("Region3int16");
		return type;
	}

} // namespace Reflection
} // namespace RBX
