/* Win/VideoControl.cpp -- October 2013 client shell (PLAYER and STUDIO compile it; see
   VideoControl.h). The ctor (asserts on lines 36 and 39) and startRecording (111, 127) are not
   written yet, so nothing here asserts. The frame callbacks are in the 2013 .obj order (slot 2
   first). */

#include "VideoControl.h"

namespace RBX {

// FUNCTION: PLAYER 0x00464dd0
// FUNCTION: STUDIO 0x0048c940
VideoControl::~VideoControl()
{
}

// FUNCTION: PLAYER 0x00464e50
// FUNCTION: STUDIO 0x0048c9c0
void VideoControl::VTable0x08(void* p_unk)
{
	if (capture->isRunning())
		capture->VTable0x18(p_unk, verb);
}

// FUNCTION: PLAYER 0x00464e90
// FUNCTION: STUDIO 0x0048ca00
void VideoControl::VTable0x04(void* p_unk)
{
	if (capture->isRunning())
		capture->VTable0x14(p_unk, verb);
}

// FUNCTION: PLAYER 0x00464ed0
// FUNCTION: STUDIO 0x0048ca40
void VideoControl::VTable0x0c()
{
	if (capture->isRunning())
		capture->VTable0x20();
}

// FUNCTION: PLAYER 0x00464f00
// FUNCTION: STUDIO 0x0048ca70
void VideoControl::VTable0x10()
{
	if (capture->isRunning())
		capture->VTable0x24();
}

} // namespace RBX
