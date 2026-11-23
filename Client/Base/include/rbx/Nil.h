// Base/include/rbx/Nil.h: October 2013 reconstruction (Project OGRE).
// Mac only: Objective-C headers define a `nil` macro that collides with RBX identifiers. Nothing in
// this tree includes it and the Windows build compiles it to nothing.
#if defined(__APPLE__) && defined(nil)
#undef nil
#endif
