/* Util/CellID.h -- October 2013 (first import, section 227).
   Evidence: layout pinned by Bridge<CellID>::on_index disasm (batch367:
     isNil byte @+0, G3D::Vector3 location @+4/+8/+12, shared_ptr<Instance>
     terrainPart @+16/+20; 24B total) + on_gc calls CellID::~CellID +
     on_eq calls CellID::operator==. No C1/C2/D1 EAs banked for the class
     itself (no sweep yet); shapes carried from March 2016 Util/CellID.h
     with one binary-pinned DELTA: location is G3D::Vector3 (the bridge
     pushes it via Bridge<G3D::Vector3>::pushNewObject), not 2016's
     RBX::Vector3 (absent in this tree).
   Original file: Client/App/include/util/CellID.h */

#pragma once

#include "G3D/Vector3.h"
#include "V8Tree/Instance.h"

namespace RBX
{
	class CellID
	{
	private:
		bool isNil;
		G3D::Vector3 location;
		shared_ptr<Instance> terrainPart;
	public:
		CellID();
		CellID(bool isNil, const G3D::Vector3& location, shared_ptr<Instance> terrainPart)
			:isNil(isNil)
			,location(location)
			,terrainPart(terrainPart)
		{
		}
		CellID(bool newIsNil, float newLocation[3], shared_ptr<Instance> newTerrainPart);
		~CellID();

		bool operator ==(const CellID& other) const
		{
			if (isNil != other.isNil)
				return false;
			if (location != other.location)
				return false;
			if (terrainPart != other.terrainPart)
				return false;
			return true;
		}

		bool getIsNil() const { return isNil; }
		void setIsNil(bool newIsNil) { isNil = newIsNil; }

		G3D::Vector3 getLocation() const { return location; }
		void setLocation(G3D::Vector3 newLocation) { location = newLocation; }

		shared_ptr<Instance> getTerrainPart() const { return terrainPart; }
		void setTerrainPart(shared_ptr<Instance> newTerrainPart) { terrainPart = newTerrainPart; }

		static CellID fromParameters(bool newIsNil, float newLocation[3], shared_ptr<Instance> newTerrainPart)
		{
			return CellID(newIsNil, newLocation, newTerrainPart);
		}
	};
}
