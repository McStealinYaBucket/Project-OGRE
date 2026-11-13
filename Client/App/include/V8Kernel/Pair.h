/* V8Kernel/Pair.h -- October 2013 (pair parameter block + geo pair).
   PairParams carried verbatim from March 2016 first-party source.
   GeoPair: 2013 shape is {type + offset0 + offset1} (28B, +116..+144 per
   the ContactConnector C2 0x... batch275); bodies live OUTSIDE at
   +144/+148 (unlike 2016, which embeds them). The compute* engine
   (computePointPlane/EdgeEdge...) queues with contact solving
   (stepper era); only the C2 is live here.
   Original file: Client/App/include/V8Kernel/Pair.h (path from 2016). */

#pragma once

#include "G3D/Vector3.h"
#include "rbx/Debug.h"

namespace RBX {

	class Body;

	class PairParams {
	public:
		G3D::Vector3 normal;
		union {
			float length;
			float rotation;
		};
		G3D::Vector3 position;
		PairParams() {
			normal = position = G3D::Vector3::zero();
			length = 0.0f;
		}
		bool operator==(const PairParams& other) {
			return (length == other.length && position == other.position && normal == other.normal);
		}
	};

	typedef enum {
		BALL_POINT_PAIR,
		BALL_EDGE_PAIR,
		BALL_PLANE_PAIR,
		POINT_PLANE_PAIR,
		EDGE_EDGE_PLANE_PAIR,
		EDGE_EDGE_PAIR,
		VERTEX_PLANE_CONNECTOR,
		EDGE_EDGE_CONNECTOR,
		EDGE_EDGE_PLANE_CONNECTOR,
		BALL_VERTEX_CONNECTOR,
		BALL_EDGE_CONNECTOR,
		BALL_PLANE_CONNECTOR,
		BULLET_SHAPE_CONNECTOR,
		BULLET_SHAPE_CELL_CONNECTOR
	} GeoPairType;

	class GeoPair {
	public:
		GeoPairType geoPairType;  // +0 (C2: pending batch276).
		// Polarity notes (2016 comments, first-party).
		G3D::Vector3 offset0;     // +4.
		G3D::Vector3 offset1;     // +16.
		Body* body0;              // +28 (binary +144).
		Body* body1;              // +32 (binary +148).
		float edgeLength0;        // +36.
		float edgeLength1;        // +40.
		struct {
			union {
				int normalID0;
				float radius0;
			};
			union {
				int normalID1;
				float radiusSum;
			};
			union {
				int planeID;
				int point0ID;
			};
		} pairData;               // +44 (12B; binary +160).

		GeoPair();
		// 2016 shapes (first-party builder stores; binary EAs queued).
		void setPointPlane(const G3D::Vector3& offsetPoint,
			const G3D::Vector3& offsetPlane, int pointID, int planeNormalID)
		{
			offset0 = offsetPoint;
			offset1 = offsetPlane;
			pairData.point0ID = pointID;
			pairData.normalID1 = planeNormalID;
			geoPairType = POINT_PLANE_PAIR;
		}
		void setEdgeEdgePlane(const G3D::Vector3& edge0, const G3D::Vector3& edge1,
			int normal0, int normal1, int planeID, float edgeLength0Arg, float edgeLength1Arg)
		{
			offset0 = edge0;
			offset1 = edge1;
			pairData.normalID0 = normal0;
			pairData.normalID1 = normal1;
			pairData.planeID = planeID;
			edgeLength0 = edgeLength0Arg;
			edgeLength1 = edgeLength1Arg;
			geoPairType = EDGE_EDGE_PLANE_PAIR;
		}
		void setEdgeEdge(const G3D::Vector3& edge0, const G3D::Vector3& edge1,
			int normal0, int normal1)
		{
			offset0 = edge0;
			offset1 = edge1;
			pairData.normalID0 = normal0;
			pairData.normalID1 = normal1;
			geoPairType = EDGE_EDGE_PAIR;
		}
		// mac: 0x53179E (550B, batch308; 2016 text carried op-for-op).
		void computePointPlane(PairParams& params);
	// mac: 0x5325AA (993B, batch308; lane note at the definition).
	void computeEdgeEdge(PairParams& params);
	// mac: 0x562E24 (289B, batch308: +28/+32 bodies with +44/+48 ids,
	// except POINT_PLANE which keys +52/+48; other types assert
	// EDGE_EDGE (Pair.h:133) then match either orientation, with the
	// swapped arm cross-comparing +48==id0 && +44==id1, verbatim).
	// Defined here as in 2013 (assert Pair.h:133; the prior tree had it in Pair.cpp).
	// STUB: PLAYER 0x00850480
	// STUB: STUDIO 0x009a8170
	bool match(Body* b0, Body* b1, GeoPairType pairType, int id0, int id1)
	{
		if (pairType == EDGE_EDGE_PLANE_PAIR)
		{
			return body0 == b0 && body1 == b1
				&& pairData.normalID0 == id0 && pairData.normalID1 == id1;
		}
		if (pairType == POINT_PLANE_PAIR)
		{
			return body0 == b0 && body1 == b1
				&& pairData.planeID == id0 && pairData.normalID1 == id1;
		}
		RBXASSERT(pairType == EDGE_EDGE_PAIR);  // Pair.h:133
		if (body0 == b0 && body1 == b1
			&& pairData.normalID0 == id0 && pairData.normalID1 == id1)
			return true;
		return body1 == b0 && body0 == b1
			&& pairData.normalID1 == id0 && pairData.normalID0 == id1;
	}
	// mac: 0x5319C4 (3046B, batch308). 2016 computeEdgeEdgePlane2 text
	// (value-identical; the binary inlines the frame helpers, so
	// float assoc on non-identity frames may differ lane-wise --
	// loud DELTA, exact on identity frames). Vector3::operator[]
	// is absent in-tree, so the two lane reads use a local select.
	void GeoPair::computeEdgeEdgePlane2(PairParams& params);
		// Compute engine + match predicates queue with contact solving
		// (stepper era).
	};

} // namespace RBX
