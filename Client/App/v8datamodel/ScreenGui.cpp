/* App/v8datamodel/ScreenGui.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"

#include "V8DataModel/ScreenGui.h"
#include "decomp.h"

namespace RBX {
	const char* const sScreenGui = "ScreenGui";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ScreenGui);
}

DECOMP_SIZE_ASSERT(RBX::ScreenGui, 0xec);

namespace RBX {

	// FUNCTION: PLAYER 0x008b1e20
	// FUNCTION: STUDIO 0x009601f0
	void ScreenGui::onHeartbeat(const Heartbeat& event)
	{
		if (m_unk0xcc)
		{
			handleResize(windowRect, false);
			m_unk0xcc = false;
		}
	}

	// FUNCTION: PLAYER 0x008b2980
	// STUB: STUDIO 0x00960c80
	ScreenGui::ScreenGui()
		: Super("ScreenGui")
		, windowRect(G3D::Rect2D::xywh(0.0f, 0.0f, 800.0f, 600.0f))
		, m_unk0xcc(false)
		, m_unk0xcd(false)
	{
	}

	// FUNCTION: PLAYER 0x008b2ae0
	// STUB: STUDIO 0x00960de0
	ScreenGui::ScreenGui(const char* name)
		: Super(name)
		, m_unk0xcc(false)
		, m_unk0xcd(false)
	{
	}

} // namespace RBX
