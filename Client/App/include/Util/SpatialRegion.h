/* Util/SpatialRegion.h -- October 2013 reconstruction.
   Fixed, origin-aligned partition of voxel space into regions of 32 x 16 x 32
   cells (shifts 5/4/5), used by the terrain mesher and replication.
   2013 evidence: iOS 10-29 has RBX::SpatialRegion::Id with the nested
   Id::boost_compatible_hash_value hasher (boost::unordered_set<Id, ...>
   instances) and EdgeSpewV2::handleCells(SpatialRegion::Id const&). The
   inline helpers below are header-only (no symbols); their arithmetic is
   kept as callers compile it, including the function-local static in
   centerOfRegionInGlobalCoordStuds. */
#pragma once

#include "Util/G3DCore.h"
#include "Util/Region3int16.h"
#include "Util/Vector3int32.h"

namespace RBX {
namespace SpatialRegion {

// Compile-time sizes (const int so they fold).
namespace Constants {
	const int kRegionXDimensionInVoxelsAsBitShift = 5;
	const int kRegionYDimensionInVoxelsAsBitShift = 4;
	const int kRegionZDimensionInVoxelsAsBitShift = 5;
	const int kRegionInVoxelsAsBitShift =
		kRegionXDimensionInVoxelsAsBitShift + kRegionYDimensionInVoxelsAsBitShift + kRegionZDimensionInVoxelsAsBitShift;

	const int kRegionXDimensionInVoxels = 1 << kRegionXDimensionInVoxelsAsBitShift;
	const int kRegionYDimensionInVoxels = 1 << kRegionYDimensionInVoxelsAsBitShift;
	const int kRegionZDimensionInVoxels = 1 << kRegionZDimensionInVoxelsAsBitShift;

	const int kMaxXVoxelOffsetInsideRegion = kRegionXDimensionInVoxels - 1;
	const int kMaxYVoxelOffsetInsideRegion = kRegionYDimensionInVoxels - 1;
	const int kMaxZVoxelOffsetInsideRegion = kRegionZDimensionInVoxels - 1;
}

namespace _PrivateConstants {
	extern const Vector3int16 kRegionDimensionInStudsAsBitShifts; // defined in the .cpp
}

// A region's index: its cell-space corner shifted down by the region size.
// Equal ids mean the same region, ordering follows world position, and
// neighbours differ by one on the shared axis.
class Id
{
	Vector3int16 v;

public:
	explicit Id(const Vector3int16& value) : v(value) {}
	Id(int x, int y, int z) : v(Vector3int16(x, y, z)) {}

	const Vector3int16& value() const { return v; }

	Id operator+(const Vector3int16& d) const { return Id(v + d); }
	bool operator==(const Id& rhs) const { return v == rhs.v; }
	bool operator!=(const Id& rhs) const { return v != rhs.v; }

	struct boost_compatible_hash_value
	{
		size_t operator()(const Id& id) const
		{
			return Vector3int16::boost_compatible_hash_value()(id.v);
		}
	};
};

std::size_t hash_value(const Id& key);

inline Vector3int16 getRegionDimensionsInVoxels()
{
	return Vector3int16(Constants::kRegionXDimensionInVoxels,
	                    Constants::kRegionYDimensionInVoxels,
	                    Constants::kRegionZDimensionInVoxels);
}

inline Vector3int16 getRegionDimensionInVoxelsAsBitShifts()
{
	return Vector3int16(Constants::kRegionXDimensionInVoxelsAsBitShift,
	                    Constants::kRegionYDimensionInVoxelsAsBitShift,
	                    Constants::kRegionZDimensionInVoxelsAsBitShift);
}

inline Vector3int16 getMaxVoxelOffsetInsideRegion()
{
	return Vector3int16(Constants::kMaxXVoxelOffsetInsideRegion,
	                    Constants::kMaxYVoxelOffsetInsideRegion,
	                    Constants::kMaxZVoxelOffsetInsideRegion);
}

// cell -> region, and cell -> offset inside its region
inline Id regionContainingVoxel(const Vector3int16& cell)
{
	return Id(cell >> getRegionDimensionInVoxelsAsBitShifts());
}

inline Vector3int16 voxelCoordinateRelativeToEnclosingRegion(const Vector3int16& cell)
{
	return cell & getMaxVoxelOffsetInsideRegion();
}

// (region, offset) -> cell
inline Vector3int16 globalVoxelCoordinateFromRegionAndRelativeCoordinate(const Id& id, const Vector3int16& offset)
{
	return (id.value() << getRegionDimensionInVoxelsAsBitShifts()) + offset;
}

// all cells of a region, both ends inclusive
inline Region3int16 inclusiveVoxelExtentsOfRegion(const Id& id)
{
	Vector3int16 first(id.value() << getRegionDimensionInVoxelsAsBitShifts());
	return Region3int16(first, first + getMaxVoxelOffsetInsideRegion());
}

// region corners and centre in studs
inline Vector3int32 smallestCornerOfRegionInGlobalCoordStuds(const Id& id)
{
	return Vector3int32(id.value()) << _PrivateConstants::kRegionDimensionInStudsAsBitShifts;
}

inline Vector3int32 centerOfRegionInGlobalCoordStuds(const Id& id)
{
	static const Vector3int32 halfSize(
		Vector3int32::one() << (_PrivateConstants::kRegionDimensionInStudsAsBitShifts - Vector3int16::one()));
	return smallestCornerOfRegionInGlobalCoordStuds(id) + halfSize;
}

inline Vector3int32 largestCornerOfRegionInGlobalCoordStuds(const Id& id)
{
	return smallestCornerOfRegionInGlobalCoordStuds(Id(id.value() + Vector3int16::one()));
}

} // namespace SpatialRegion
} // namespace RBX
