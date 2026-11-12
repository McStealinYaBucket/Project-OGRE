/* Copyright 2003-2013 ROBLOX Corporation, All Rights Reserved */

#pragma once

// Reconstructed from October 2013 binaries.
// Evidence: RBX::Reflection::EnumDesc<RBX::SurfaceType>::EnumDesc (0x57CBDE) registers the
// names in this exact order/value mapping:
//   0 "Smooth", 1 "Glue", 2 "Weld", 3 "Studs", 4 "Inlet", 5 "Universal",
//   6 "Hinge", 7 "Motor", 8 "SteppingMotor", 9 "Unjoinable", 10 "SmoothNoOutlines"
// (plus alias map entries "Spawn" and "Bumps" -> legacy input names).
// Structure matches the March 2016 source header verbatim; the binary confirms it.

namespace RBX {

	// TODO - Joint.cpp uses this ordering - fix up as a class

	typedef enum {	NO_SURFACE = 0,
					GLUE,
					WELD,
					STUDS,
					INLET,
					UNIVERSAL,
					ROTATE,			// special ordering here
					ROTATE_V,
					ROTATE_P,
					NO_JOIN, // new surface type for specifically preventing joining (via ManualWeldHelper)
					NO_SURFACE_NO_OUTLINES,	 // identical to NO_SURFACE, but removes outlines
					NUM_SURF_TYPES} SurfaceType;

	inline bool IsNoSurface(SurfaceType surface)
	{
		return surface == NO_SURFACE || surface == NO_SURFACE_NO_OUTLINES;
	}

	inline bool IsRotate(SurfaceType surface)
	{
		return surface >= ROTATE && surface <= ROTATE_P;
	}

	namespace Legacy {
		// TODO: improve precedence logic
		// LEGACY from when this was separate stuff
		// Binary evidence: std::vector<RBX::Legacy::SurfaceConstraint> /
		// std::map<RBX::Name const*, RBX::Legacy::SurfaceConstraint> symbols (0x42A5C4 etc.)
		typedef enum {	NO_CONSTRAINT = 0,
						ROTATE_LEGACY,
						ROTATE_P_LEGACY,
						ROTATE_V_LEGACY,
						NUM_CONSTRAINT_TYPES} SurfaceConstraint;

	}
} // namespace
