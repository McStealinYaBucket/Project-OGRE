/* Voxel/CellChangeListener.h -- October 2013.
   The terrain-change callback interface ContactManager derives from: one vftable slot, no virtual
   destructor (ContactManager vftable PLAYER 0x00b64cc4). No 2013 __FILE__ string names this header;
   the path is the 2016 tree's. CellChangeInfo's 2013 layout is unproven, so it stays incomplete. */

#pragma once

namespace RBX {
	namespace Voxel {

		struct CellChangeInfo;

		class CellChangeListener
		{
		public:
			virtual void terrainCellChanged(const CellChangeInfo& info) = 0;
		};

	} // namespace Voxel
} // namespace RBX
