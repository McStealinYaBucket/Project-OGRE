/* Voxel/Util.h -- October 2013 reconstruction (see the note at the end for evidence). */
#pragma once

#include "Util/G3DCore.h"
#include "Voxel/Cell.h"
#include "rbx/Debug.h"
#include "Util/Extents.h"
#include "Util/Region3int16.h"

namespace RBX { namespace Voxel {

// Materials pack two cells per byte, low nibble first, stored as material - 1;
// an empty block always reads as water.
inline CellMaterial readMaterial(const unsigned char* materials, const unsigned int cellIndex, const Cell cell) {
	return (CellMaterial)(cell.solid.getBlock() == CELL_BLOCK_Empty
		? CELL_MATERIAL_Water
		: ((materials[cellIndex >> 1] >> (4 * (cellIndex & 0x1))) & 0x0f) + 1);
}

// newMaterial is 1-based (0 is not a storable material)
inline void writeMaterial(unsigned char* materials, unsigned int cellIndex, const CellMaterial newMaterial) {
	RBXASSERT(newMaterial > 0);
	unsigned char& packed = materials[cellIndex >> 1];
	unsigned int shift = (4 * (cellIndex & 0x1));
	unsigned char mask = 0x0f << shift;
	packed &= (~mask);
	packed |= (((newMaterial - 1) << shift) & mask);
}

// pre-2013 cell format: material in the low three bits of the cell byte
inline CellMaterial getCellMaterial_Deprecated(unsigned char cell) { return (CellMaterial)(cell & 0x07); }
inline void setCellMaterial_Deprecated(unsigned char& cell, CellMaterial material) { cell = (cell & 0xf8) | ((int)material & 0x07); }

// Order matters: OrientedFaceMap is indexed cell * 6 + FaceDirection.
enum FaceDirection
{
	PlusX = 0,
	PlusZ = 1,
	MinusX = 2,
	MinusZ = 3,
	PlusY = 4,
	MinusY = 5,
	Invalid = 6
};

// One face of a (possibly wedge-shaped) block: which corner, if any, is cut away.
struct BlockAxisFace
{
	enum SkippedCorner
	{
		TopRight = 0,
		TopLeft = 1,
		BottomLeft = 2,
		BottomRight = 3,
		EmptyAllSkipped = 4,
		FullNoneSkipped = 5
	};

	SkippedCorner skippedCorner;

	// quarter turns; the two non-corner states are unchanged
	static inline SkippedCorner rotate(SkippedCorner c, const CellOrientation orient) {
		return (SkippedCorner)(c < 4 ? (c + orient) % 4 : c);
	}

	static inline bool divideTopLeftToBottomRight(SkippedCorner c) {
		return c == TopRight || c == BottomLeft || c == FullNoneSkipped;
	}

	// The three tables are function-local statics; iOS 10-29 names them
	// XZAxisMirror(...)::MIRROR, YAxisMirror(...)::MIRROR and inverse(...)::OPPOSITE_CORNER.
	static inline SkippedCorner XZAxisMirror(SkippedCorner c) {
		static SkippedCorner MIRROR[6] = { TopLeft, TopRight, BottomRight, BottomLeft, EmptyAllSkipped, FullNoneSkipped };
		return MIRROR[c];
	}

	static inline SkippedCorner YAxisMirror(SkippedCorner c) {
		static SkippedCorner MIRROR[6] = { BottomRight, BottomLeft, TopLeft, TopRight, EmptyAllSkipped, FullNoneSkipped };
		return MIRROR[c];
	}

	static BlockAxisFace inverse(const BlockAxisFace f) {
		static const SkippedCorner OPPOSITE_CORNER[6] = {
			BottomLeft, BottomRight, TopRight, TopLeft, FullNoneSkipped, EmptyAllSkipped
		};
		BlockAxisFace r;
		r.skippedCorner = OPPOSITE_CORNER[f.skippedCorner];
		return r;
	}
};

struct BlockFaceInfo
{
	BlockAxisFace faces[6]; // by FaceDirection
};

extern const BlockFaceInfo UnOrientedBlockFaceInfos[6];
extern BlockAxisFace OrientedFaceMap[1536]; // 256 cell values * 6 directions

// Fills OrientedFaceMap (ComputeOrientedFace stays private to the .cpp).
void initBlockOrientationFaceMap();

inline const BlockAxisFace& GetOrientedFace(Cell cell, FaceDirection f)
{
	return OrientedFaceMap[Cell::asUnsignedCharForDeprecatedUses(cell) * 6 + f];
}

inline bool isWedgeSideNotFull(Cell voxel, FaceDirection f)
{
	return GetOrientedFace(voxel, f).skippedCorner != BlockAxisFace::FullNoneSkipped;
}

// World <-> cell conversions (kCELL_SIZE studs per cell). The zero
// kXZOffset constants are left in: they are part of the expressions callers
// were compiled from.
inline Vector3int16 worldToCell_floor(const Vector3& p)
{
	const int kXZOffset = 0;
	return Vector3int16((int)(floorf(p.x / kCELL_SIZE)) + kXZOffset,
	                    (int)(floorf(p.y / kCELL_SIZE)),
	                    (int)(floorf(p.z / kCELL_SIZE)) + kXZOffset);
}

inline Vector3 worldSpaceToCellSpace(const Vector3& p)
{
	return Vector3((p.x * (1.0f / kCELL_SIZE)), (p.y * (1.0f / kCELL_SIZE)), (p.z * (1.0f / kCELL_SIZE)));
}

inline Vector3 cellSpaceToWorldSpace(const Vector3& p)
{
	return Vector3((p.x * kCELL_SIZE), (p.y * kCELL_SIZE), (p.z * kCELL_SIZE));
}

inline Vector3 cellToWorld_smallestCorner(const Vector3int16& c)
{
	const int kXZOffset = 0;
	return Vector3((c.x - kXZOffset) * kCELL_SIZE, c.y * kCELL_SIZE, (c.z - kXZOffset) * kCELL_SIZE);
}

inline Vector3 cellToWorld_center(const Vector3int16& c)
{
	Vector3 corner = cellToWorld_smallestCorner(c);
	return corner + Vector3(kHALF_CELL, kHALF_CELL, kHALF_CELL);
}

inline Vector3 cellToWorld_largestCorner(const Vector3int16& c)
{
	return cellToWorld_smallestCorner(c + Vector3int16(1, 1, 1));
}

// The terrain grid: +-32000 cells on each axis.
inline Region3int16 getTerrainExtentsInCells()
{
	const int kRadius = 32000;
	return Region3int16(Vector3int16(-kRadius, -kRadius, -kRadius), Vector3int16(kRadius, kRadius, kRadius));
}

inline Extents getTerrainExtents()
{
	Region3int16 cells = getTerrainExtentsInCells();
	return Extents(cellToWorld_smallestCorner(cells.getMinPos()), cellToWorld_largestCorner(cells.getMaxPos()));
}

} } // namespace RBX::Voxel

/* Evidence. The writeMaterial assert is recorded in PLAYER and STUDIO as
   "newMaterial > 0 file: ...\Voxel/Util.h line: 22" and must stay on line 22,
   which is why this note sits at the end. iOS 10-29 has
   initBlockOrientationFaceMap, OrientedFaceMap, UnOrientedBlockFaceInfos,
   ComputeOrientedFace(BlockFaceInfo const&, FaceDirection, CellOrientation)
   and the three static tables named above. The rest is inline and header-only. */
