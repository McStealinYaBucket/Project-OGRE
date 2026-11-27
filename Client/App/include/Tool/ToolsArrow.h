/* Tool/ToolsArrow.h -- October 2013, partial. Only AdvArrowToolBase's three statics so far, which the Commands.cpp
   grid and manual-joint verbs read and write (SetGridToOne::doIt PLAYER 0x008569e0, SetManualJointToWeak::doIt
   0x00856bb0, TurnOnManualJointCreation::doIt 0x00856900). Names: iOS 10-29 (RBX::AdvArrowToolBase::advGridMode,
   advManualJointMode, advManualJointType); enum names: 2016 tool/DragTypes.h, the values match the 2013 stores.
   No 2013 __FILE__ names this header; the path follows 2016. */

#pragma once

namespace RBX {

	namespace DRAG {
		typedef enum { WEAK_MANUAL_JOINT, STRONG_MANUAL_JOINT, INFINITE_MANUAL_JOINT } ManualJointType;
		typedef enum { ONE_STUD, QUARTER_STUD, OFF } DraggerGridMode;
	}

	class AdvArrowToolBase
	{
	public:
		static bool advManualJointMode;					// PLAYER 0x00d98b40 (.data, true)
		static DRAG::ManualJointType advManualJointType;	// PLAYER 0x00d98b44 (.data, STRONG_MANUAL_JOINT)
		static DRAG::DraggerGridMode advGridMode;			// PLAYER 0x00e14628 (.bss, ONE_STUD)
	};

} // namespace RBX
