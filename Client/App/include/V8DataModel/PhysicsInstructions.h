/* V8DataModel/PhysicsInstructions.h -- October 2013.
   Evidence: ctor PLAYER 0x0084d2f0 / STUDIO 0x009a4fe0 (two Average<double>(30, 1.0) at +0 and
     +0x18, doubles 0 at +0x30/+0x38, +0x40 from GlobalAdvancedSettingsItem<PhysicsSettings>::
     singleton() 0x00654470 (+0x68), +0x48 0.0, byte +0x50 false, +0x58 1.0); DataModel member at
     0xbe8 (DataModel ctor 0x0064ee60, dtor 0x0064da95). Size 0x60.
   Where 2013 disagrees with 2016: no averageCyclicDt (the 2013 ctor builds two Averages).
   Member names from the 2016 header. Original file: Client/App/v8datamodel/PhysicsInstructions.cpp
   (2013 assert path); the header path follows the tree. */

#pragma once

#include "Util/Average.h"

namespace RBX {

	// SIZE 0x60
	class PhysicsInstructions
	{
	private:
		Average<double> averageDt;			// 0x00
		Average<double> averageDutyDt;		// 0x18
		double timeSinceLastRadiusChange;	// 0x30
		double throttleTimer;				// 0x38
		double throttleAdjustTime;			// 0x40

	public:
		double requestedDutyPercent;		// 0x48
		bool bandwidthExceeded;				// 0x50
		double networkBufferHealth;			// 0x58

		PhysicsInstructions();
	};

} // namespace RBX
