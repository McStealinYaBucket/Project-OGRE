/* V8World/Enum.h -- October 2013 (simulation state enums).
   Carried from March 2016 first-party Enum.h (values pinned by usage:
   SleepStage::changeContactState 0x59Fxxx branches on edge states
   1 (STEPPING) / 3 (CONTACTING); changeJointState branches on 1;
   SleepStage arrays index by throttle 0/1 (NUM_THROTTLE_TYPE=2);
   onEdgeRemoving resets state to 0/3 (UNDEFINED/UNDEFINED_THROTTLE).
   DELTA vs 2016: plain (int-sized) enums -- 2013 disasm reads/writes
   Edge+8/+12 as DWORDs, while 2016 uses `enum : unsigned char` on
   _WIN32. AssemblyState order matches the 6-set SleepStage layout
   (recursiveWakePending .. removing, ANCHORED excluded from the sets).
   Original file: Client/App/include/V8World/Enum.h (path from 2016). */

#pragma once

namespace RBX {

	namespace Sim {

		typedef enum {		ANCHORED,
							RECURSIVE_WAKE_PENDING,
							WAKE_PENDING,
							AWAKE,
							SLEEPING_CHECKING,
							SLEEPING_DEEPLY,
							REMOVING		}	AssemblyState;

		inline bool isMovingAssemblyState(AssemblyState state) {
			return ((state == AWAKE) || (state == RECURSIVE_WAKE_PENDING) || (state == WAKE_PENDING));
		}

		inline bool isSleepingAssemblyState(AssemblyState state) {
			return ((state == SLEEPING_CHECKING) || (state == SLEEPING_DEEPLY));
		}

		inline bool outOfKernelAssemblyState(AssemblyState state) {
			return (isSleepingAssemblyState(state) || (state == REMOVING));
		}

		// One byte each on Windows (MSVC's `enum : unsigned char` extension, C4480; 2016 keeps it
		// under #ifdef _WIN32): SleepStage 0x008e7e00 tests `byte [edge+8]` (edgeState) and
		// `byte [edge+9]` == 3 (throttleType), and the Edge dtor 0x0081f840 finds prim0..index1 at
		// +0x0c..+0x18. The Mac build keeps 4-byte enums (its Edge has them at +8/+12).
		typedef enum : unsigned char {		CAN_NOT_THROTTLE = 0,
							CAN_THROTTLE,
							NUM_THROTTLE_TYPE,
							UNDEFINED_THROTTLE	} ThrottleType;

		typedef enum : unsigned char {		UNDEFINED,
							STEPPING,
							SLEEPING,
							CONTACTING,
							CONTACTING_SLEEPING} EdgeState;

	} // namespace Sim

} // namespace RBX
