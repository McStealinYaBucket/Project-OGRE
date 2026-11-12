/* V8DataModel/GuiBase.h — October 2013 (gui root shell).
   Evidence: GuiBase C2 0x3949CA (Instance(name, NULL) + members +25/+26/+27
     = -1, +28/+29 = 0; Described<GuiBase, sGuiBase, Instance, 27>).
     sGuiBase 69 hits in the 0.125 PE string table.
   TODO: full decompile (member identities + gui surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiBase.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sGuiBase;

	// Render-queue ids (2016 GuiBase.h shape carried: the 0x6E5C74 ctor
	// pre-sizes 2 rows x 11 Z-layers, i.e. Z 0..10 per queue; TODO verify
	// 2013 enum/Z helpers from their EAs).
	enum GuiQueue
	{
		GUIQUEUE_GENERAL = 0,
		GUIQUEUE_TEXT,
		GUIQUEUE_COUNT
	};

	class GuiBase
		: public DescribedNonCreatable<GuiBase, Instance, sGuiBase, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		// mac: 0x3949CA C2(name); MSVC2012 has no delegating ctors, so the
		// default flows through the NULL default arg.
		GuiBase(const char* name = NULL);

		static int minZIndex() { return 0; }
		static int maxZIndex() { return 10; }
		static int minZIndex2d() { return minZIndex() + 1; }
		static int maxZIndex2d() { return maxZIndex(); }

	private:
		// mac: 0x3949CA members (Mach-O dword offsets; TODO identities).
		int unknown25;
		int unknown26;
		int unknown27;
		int unknown28;
		int unknown29;
	};

} // namespace RBX
