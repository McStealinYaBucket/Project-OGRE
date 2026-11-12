/* App/v8datamodel/Scale9Frame.cpp -- October 2013.
   Self-audit: every symbol below is binary-pinned (dbq24): ctor 0x46FC7A
     (299B), setScaleEdgeSize 0x46FBCE (74B: compare, store @+536,
     raisePropertyChanged), setSlicePrefix 0x46FC18 (87B: compare,
     assign @+540, raisePropertyChanged), getScaleEdgeSize 0x46FEC6,
     getSlicePrefix 0x46FF02.
   DELTAs: the raisePropertyChanged tail is omitted -- the property
     descriptors it needs (@0xFB53B4/0xFB53E0) are unrelocated in dbq24
     and no reflection surface is landed for them; set/get remain
     observable-identical until that table lands. */

#include "stdafx.h"

#include "V8DataModel/Scale9Frame.h"

namespace RBX {

	const char* const sScale9Frame = "Scale9Frame";

	// mac: 0x46FC7A (299B): GuiObject base ctor + zero(+536..539) + empty
	// string @+540 + GuiDrawImage default @+544.
	Scale9Frame::Scale9Frame()
		: scaleEdgeSize(0, 0)
	{
	}

	// mac: 0x46FBCE (74B): full compare vs +536, equal -> return, else store
	// the word pair.
	void Scale9Frame::setScaleEdgeSize(const G3D::Vector2int16& value)
	{
		if (scaleEdgeSize.x == value.x && scaleEdgeSize.y == value.y)
			return;
		scaleEdgeSize.x = value.x;
		scaleEdgeSize.y = value.y;
	}

	// mac: 0x46FC18 (87B): string compare vs +540, equal -> return, else
	// assign.
	void Scale9Frame::setSlicePrefix(const std::string& value)
	{
		if (slicePrefix == value)
			return;
		slicePrefix = value;
	}

} // namespace RBX

using namespace RBX;

// Class registrar (see Instance.cpp/Service.cpp note); the property
// descriptors 0xFB53B4/0xFB53E0 stay unported (unrelocated blob DELTA).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Scale9Frame);
}
