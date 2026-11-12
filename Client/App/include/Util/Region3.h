/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */
#pragma once

#include "G3D/Vector3.h"
#include "G3D/CoordinateFrame.h"

namespace RBX {
	class Extents;

	// Reconstructed from Oct 2013 studio binary (october2013symbolledrobloxstudio.i64).
	// Layout verified by decompilation: cframe (CoordinateFrame, 48 bytes) then size (Vector3, 12 bytes).
	// ctors: 0x62DF88, 0x62DFB4, 0x62E124; init: 0x62E014; minPos: 0x62E154; maxPos: 0x62E1B0
	class Region3 {
	private:
		G3D::CoordinateFrame cframe;
		Vector3 size;
		void init(const Extents &extents);	// 0x62E014

	public:
		Region3();							// 0x62DF88
		Region3(const Vector3& min, const Vector3& max);	// 0x62DFB4
		explicit Region3(const Extents &extents);			// 0x62E124

		~Region3() {}

		const G3D::CoordinateFrame& getCFrame() const { return cframe; }
		const Vector3& getSize() const { return size; }

		Vector3 minPos() const;				// 0x62E154
		Vector3 maxPos() const;				// 0x62E1B0

		inline bool operator==(const Region3& other) const {
			return (size == other.size) && (cframe == other.cframe);
		}

		inline bool operator!=(const Region3& other) const {
			return !(*this == other);
		}
	};
}
