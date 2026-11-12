/* V8Kernel/BodyPvSetter.h -- October 2013 (body velocity-write token).
   Carried verbatim from March 2016 first-party BodyPvSetter.h: empty
   tag struct -- Primitive descends from it to gate Body::setVelocity
   from others. 2013 Body::setVelocity 0x5A07xx takes it (batch261)
   but never reads it (overload tag only).
   Original file: Client/App/include/V8Kernel/BodyPvSetter.h (path
   from 2016). */

#pragma once

namespace RBX {

	// Stub class - primitive descends from this to protect body::setCoordinateFrame() from others
	class BodyPvSetter
	{
	};

} // namespace RBX
