// Base/include/rbx/Declarations.h: October 2013 reconstruction (Project OGRE).
// Class decorations for abstract bases. On MSVC both map to __declspec(novtable): the constructor
// does not store the base's vftable, so such a base gets no vftable of its own in the image.
// Evidence: PLAYER has no Debugable vftable and no constructor stores one (CLAUDE.md, Class
// layout), while Object and AbstractFactoryProduct, which are not decorated, do have theirs.
// A novtable class must not declare a virtual destructor.
#pragma once

#if defined(_WIN32)
#define RBXInterface __declspec(novtable) // pure interface: pure virtuals, maybe trivial members
#define RBXBaseClass __declspec(novtable) // abstract base with real code
#else
#define RBXInterface
#define RBXBaseClass
#endif
