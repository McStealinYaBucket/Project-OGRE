/* App/v8kernel/Body.cpp -- October 2013 (rigid body node). Laid out on the 2013 asserts: ~Body :35-38 and
   :48-49 (D1 0x008ee400). Pooled through Allocator<T> (rbx/memory.h). */
#include "stdafx.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/SimBody.h"
#include "V8Kernel/BodyPvSetter.h"
#include "Util/Velocity.h"
#include "Util/PV.h"
#include <windows.h>
namespace RBX {
	Body* Body::worldBody = NULL;
	unsigned int Body::globalId = 0;
	Body::Body()
		: unknown28()  // tbb::spin_mutex at +0x1c, sharing the empty Allocator<Body> base (C2 0x008ee2c0)
		, leafBodyIndex(-1)
		, connectorUseCount(0)
		, root(this)
		, cofm(NULL)
		, simBody(NULL)
		, link(NULL)
		, canThrottle(true)
		, mass(0.0f)
		, stateIndex(++globalId)
	{
		moment = G3D::Matrix3::identity();
		simBody = new SimBody(this);
	}
	// FUNCTION: PLAYER 0x008ee400
	// FUNCTION: STUDIO 0x00a0c590
	Body::~Body()
	{



		RBXASSERT(connectorUseCount == 0);
		RBXASSERT(!link);
		RBXASSERT(simBody);
		RBXASSERT(leafBodyIndex == -1);
		if (cofm) {
			delete cofm;
			cofm = NULL;
		}
		delete simBody;
		simBody = NULL;



		RBXASSERT(!cofm);
		RBXASSERT(root == this);
	}























	// FUNCTION: PLAYER 0x008ece70
	// FUNCTION: STUDIO 0x00a0b060
	bool Body::validateParentCofmDirty()
	{
		RBXASSERT(cofm);
		RBXASSERT(cofm->getIsDirty());
		if (Body* parent = getTypedParent<Body>()) {
			parent->validateParentCofmDirty();
		}
		return true;
	}



	// FUNCTION: PLAYER 0x008ecf10
	// FUNCTION: STUDIO 0x00a0b100
	void Body::makeCofmDirty()
	{
		if (cofm && cofm->getIsDirty())
		{
			RBXASSERT(this->validateParentCofmDirty());
			RBXASSERT(this->getRootSimBody()->getDirty());
		}
		else
		{
			if (getTypedParent<Body>()) {
				RBXASSERT(!simBody);  // :100
				getTypedParent<Body>()->makeCofmDirty();
			}
			else {
				RBXASSERT(root == this);  // :104
				if (simBody) {
					simBody->makeDirty();
				}
			}

			if (cofm) {
				cofm->makeDirty();
				RBXASSERT((numChildren() > 0) || (cofmOffset != Vector3::zero()));
			}
			else {
				RBXASSERT(numChildren() == 0);  // :115
			}
		}
	}
	// FUNCTION: PLAYER 0x008ed5c0
	// FUNCTION: STUDIO 0x00a0b7a0
	void Body::resetRoot(Body* newRoot) {
		RBXASSERT(newRoot == calcRoot());
		root = newRoot;
		for (int i = 0; i < numChildren(); ++i) {
			getTypedChild<Body>(i)->resetRoot(newRoot);
		}
	}












































	// FUNCTION: PLAYER 0x008edf30
	// FUNCTION: STUDIO 0x00a0c0c0
	void Body::refreshCofm()
	{
		bool hasChildren = (numChildren() > 0);
		bool hasCofmOffset = (cofmOffset != G3D::Vector3::zero());

		bool needsCofm = (hasChildren || hasCofmOffset);

		if (needsCofm) {
			if (!cofm) {
				cofm = new Cofm(this);
				RBXASSERT(cofm->getIsDirty());  // :184
			}
		}
		else {
			if (cofm) {
				delete cofm;
				cofm = NULL;
			}
		}
	}

	// Batch261: ++globalId stamp into stateIndex (+0xa0).
	// FUNCTION: PLAYER 0x008ecdf0
	// FUNCTION: STUDIO 0x00a0afe0
	void Body::advanceStateIndex()
	{
		stateIndex = ++globalId;
	}

	// mac: 0x523BFE (24B, batch289).
	unsigned int Body::getNextStateIndex()
	{
		return ++globalId;
	}

	// Batch261: parent-NULL (+4) gate via the public typed-parent
	// accessor (parent itself is private in IndexedTree); linVel/angVel
	// stores; serial bump; sim dirty. Setter is an overload tag (unread).
	void Body::setVelocity(const Velocity& vel, const BodyPvSetter& setter)
	{
		(void)setter;
		if (getTypedParent<Body>() == NULL)
		{
			linVel = vel.linear;
			angVel = vel.rotational;
			advanceStateIndex();
			if (simBody != NULL)
				simBody->makeDirty();
		}
	}









































	// mac: 0x524B7E (batch268).
	// STUB: PLAYER 0x008ed320
	// STUB: STUDIO 0x00a0b510
	void Body::setPv(const PV& pv, const BodyPvSetter& setter)
	{
		(void)setter;
		if (getTypedParent<Body>() != NULL)
		{
			RBXASSERT(false);  // :273 "0"
			return;
		}
		worldFrame = pv.position;
		linVel = pv.velocity.linear;
		angVel = pv.velocity.rotational;
		if (simBody != NULL)
			simBody->makeDirty();
		advanceStateIndex();
	}

	// mac: 0x5927EA (batch258).
	// FUNCTION: PLAYER 0x007554e0
	// FUNCTION: STUDIO 0x00894800
	PV* Body::getPV_Spin_Lock()
	{
		// OGRE vc9: was a hand-rolled _InterlockedCompareExchange8 spin; the PLAYER bytes are a
		// tbb::spin_mutex::scoped_lock (see Body.h unknown28).
		tbb::spin_mutex::scoped_lock lock(unknown28);
		updatePV();
		return reinterpret_cast<PV*>(&worldFrame);
	}

	// mac: 0x525570 (batch294).
	G3D::Matrix3 Body::getIWorld()
	{
		G3D::Matrix3 iBody = moment;
		updatePV();
		return Math::momentToWorldSpace(iBody, worldFrame.rotation);
	}
















































	// mac: 0x52500E (batch291).
	// FUNCTION: PLAYER 0x008ed4b0
	// FUNCTION: STUDIO 0x00a0b6a0
	const G3D::Vector3& Body::getBranchCofmOffset()
	{
		RBXASSERT(simBody);
		RBXASSERT((numChildren() > 0) || !getCofm() || getCofm()->getCofmInBody() == cofmOffset);

		return cofm ? cofm->getCofmInBody() : cofmOffset;
	}

	// mac: 0x524F12 (batch292).
	G3D::Vector3 Body::getBranchCofmPos()
	{
		updatePV();
		return cofm ? worldFrame.pointToWorldSpace(cofm->getCofmInBody())
			: getPos();
	}

	// mac: 0x525140 (batch292).
	G3D::CoordinateFrame Body::getBranchCofmCoordinateFrame()
	{
		updatePV();
		G3D::Vector3 pos = getBranchCofmPos();
		return G3D::CoordinateFrame(worldFrame.rotation, pos);
	}

	// mac: 0x524DEE (batch293).
	G3D::Matrix3 Body::getIBodyAtPoint(const G3D::Vector3& point)
	{
		return Math::getIBodyAtPoint(point, moment, mass);
	}

	// mac: 0x524E38 (batch293).
	G3D::Matrix3 Body::getIWorldAtPoint(const G3D::Vector3& point)
	{
		updatePV();
		return Math::getIWorldAtPoint(worldFrame.translation, point,
			getIWorld(), mass);
	}

	// mac: 0x524E94 (batch293).
	G3D::Matrix3 Body::getBranchIWorldAtPoint(const G3D::Vector3& point)
	{
		return Math::getIWorldAtPoint(getBranchCofmPos(), point,
			getBranchIWorld(), getBranchMass());
	}

	// mac: 0x524D5C (batch294).
	void Body::setMass(float _mass)
	{
		if (mass != _mass) {
			makeCofmDirty();
			mass = _mass;
		}
	}

	// mac: 0x524D90 (batch294).
	void Body::setMoment(const G3D::Matrix3& _momentInBody)
	{
		if (moment != _momentInBody) {
			makeCofmDirty();
			moment = _momentInBody;
		}
	}

	// mac: 0x524760 (batch294).
	// FUNCTION: PLAYER 0x008ee060
	// FUNCTION: STUDIO 0x00a0c1f0
	void Body::setCofmOffset(const G3D::Vector3& _cofmOffset)
	{
		if (cofmOffset != _cofmOffset) {
			cofmOffset = _cofmOffset;
			refreshCofm();
			makeCofmDirty();
		}
	}

	// mac: 0x524632 (batch294).

	// mac: 0x5242B6 (batch294).
	// calcRoot DELTA: 2016's calcRoot() walks the parent chain to the
	// top -- identical to this tree's getRoot<Body>() -- so the :122
	// assert rides it directly. The binary's per-child
	// indexOf(array[n])==n assert lives inside IndexArray::operator[]
	// (see Util/IndexArray.h), reached via getChild().



	// mac: 0x524628 (batch294).
	// FUNCTION: PLAYER 0x008ed150
	// FUNCTION: STUDIO 0x00a0b340
	void Body::onChildAdding(IndexedTree* child)
	{
		(void)child;
		makeCofmDirty();
	}

} // namespace RBX
