/* Util/UserInputBase.h -- October 2013. The engine half of the shell's UserInput
   (RTTI .?AVUserInput@RBX@@ : UserInputBase, single inheritance; vftable PLAYER 0x00be4d9c).
   UserInputBase has no Complete Object Locator, so it is novtable (RBXBaseClass); its ctor
   (PLAYER 0x0072af80, out of line in App/Util/UserInputBase.cpp) stores no vptr.

   Size 0x38: UserInput's own members start at +0x38 (its ctor 0x0045e300 calls 0x0072af80, then
   stores +0x38/+0x3c). Members from the ctor and setCursorId 0x0072af30:
     +0x04 ContentId currentCursorId   (string + Name*, getNullName)
     +0x24 TextureProxyBaseRef currentCursor   (reset by setCursorId)
     +0x2c TextureProxyBaseRef fallbackCursor  (getGameCursor 0x0072ad90; assert line 52)
     +0x34 rbx::signal<void()> cursorIdChangedSignal (fired by setCursorId)
   2016's unbindResourceSignal connection does not exist yet.

   Eight virtuals, in UserInput's vftable order: getCursorPosition (slot 0), getGameCursor (1,
   UserInput overrides it and calls this one), removeJobs (2, UserInput's dtor calls its own),
   centerCursor (3), keyDown (4), setKeyState (5), setCursorId (6, 0x0072af30, not overridden),
   renderGameCursor (7, 0x0072aaf0, not overridden). Names and parameter types from ios 1029
   (getGameCursor, setCursorId, renderGameCursor, removeJobs, getNavKeys) and 2016 for the pure
   virtuals; the slot bodies fit them (slot 3 zeroes a Vector2, slot 4 takes a KeyCode and
   returns bool). No virtual destructor (2016 neither; ios's ~UserInputBase is non-virtual). */

#pragma once

#include "rbx/Declarations.h"
#include "rbx/signal.h"
#include "Util/ContentId.h"
#include "Util/KeyCode.h"

#include <boost/shared_ptr.hpp>
#include <G3D/Vector2.h>

namespace RBX {

	class Adorn;
	class NavKeys;
	class TextureProxyBase;
	typedef boost::shared_ptr<TextureProxyBase> TextureProxyBaseRef;

	// SIZE 0x38
	class RBXBaseClass UserInputBase
	{
	private:
		ContentId currentCursorId;			// 0x04
		TextureProxyBaseRef currentCursor;	// 0x24
		TextureProxyBaseRef fallbackCursor;	// 0x2c

	protected:
		virtual G3D::Vector2 getCursorPosition() = 0;				// vtable+0x00
		virtual TextureProxyBaseRef getGameCursor(Adorn* adorn);	// vtable+0x04  0x0072ad90
		TextureProxyBaseRef getCurrentCursor() { return currentCursor; }
		TextureProxyBaseRef getFallbackCursor() { return fallbackCursor; }

	public:
		UserInputBase();
		~UserInputBase() {}

		rbx::signal<void()> cursorIdChangedSignal;	// 0x34

		virtual void removeJobs() {}				// vtable+0x08
		virtual void centerCursor() = 0;			// vtable+0x0c
		virtual bool keyDown(KeyCode code) const = 0;	// vtable+0x10

		void getNavKeys(NavKeys& navKeys, bool shouldSuppressNavKeys) const;

		bool altKeyDown() const { return keyDown(SDLK_RALT) || keyDown(SDLK_LALT); }
		bool shiftKeyDown() const { return keyDown(SDLK_RSHIFT) || keyDown(SDLK_LSHIFT); }
		bool ctrlKeyDown() const { return keyDown(SDLK_RCTRL) || keyDown(SDLK_LCTRL); }

		virtual void setKeyState(KeyCode code, ModCode modCode, char modifiedKey, bool isDown) = 0;	// vtable+0x14

		ContentId getCurrentCursorId() { return currentCursorId; }
		virtual bool setCursorId(Adorn* adorn, const TextureId& id);	// vtable+0x18  0x0072af30
		virtual void renderGameCursor(Adorn* adorn);					// vtable+0x1c  0x0072aaf0
	};

} // namespace RBX
