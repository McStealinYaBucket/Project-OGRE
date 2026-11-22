/* V8Kernel/Link.h -- October 2013 (joint-body link state).
   Layout (Link::reset 0x008e37a0, dirty 0x008e3740, the computeChildInParent bodies): vptr +0, body +4,
   parentCoord +8, childCoord +0x38, childCoordInverse +0x68, childInParent +0x98, stateIndex +0xc8
   (sizeof 0xcc). Names from the 2016 Link.h, confirmed by dirty's assert "body->getLink() == this".
   The 2013 vftables (RevoluteLink 0x00bab4f4, D6Link 0x00bab34c) hold one slot, computeChildInParent:
   no virtual destructor. Both subclasses add Allocator<T> as an empty base at +0xcc (RTTI). */

#pragma once

#include "G3D/CoordinateFrame.h"
#include "rbx/memory.h"
#include "rbx/Declarations.h"

namespace RBX {

	class Body;

	class RBXBaseClass Link
	{
	protected:
		Body* body;                              // +0x04
		G3D::CoordinateFrame parentCoord;        // +0x08
		G3D::CoordinateFrame childCoord;         // +0x38
		G3D::CoordinateFrame childCoordInverse;  // +0x68
		G3D::CoordinateFrame childInParent;      // +0x98
		unsigned int stateIndex;                 // +0xc8

		virtual void computeChildInParent(G3D::CoordinateFrame& answer) const = 0;  // vtable+0x00

	public:
		Link();
		~Link();

		void dirty();
		void reset(const G3D::CoordinateFrame& parentC, const G3D::CoordinateFrame& childC);

		Body* getBody() const { return body; }
		void setBody(Body* value) { body = value; }
	};

	class RevoluteLink : public Link, public Allocator<RevoluteLink>
	{
	private:
		float jointAngle;  // +0xcc

		/*override*/ virtual void computeChildInParent(G3D::CoordinateFrame& answer) const;

	public:
		RevoluteLink() : jointAngle(0.0f) {}
		float getJointAngle() const { return jointAngle; }
		void setJointAngle(float value) { jointAngle = value; }
	};

	class D6Link : public Link, public Allocator<D6Link>
	{
	private:
		G3D::CoordinateFrame offsetCFrame;  // +0xcc

		/*override*/ virtual void computeChildInParent(G3D::CoordinateFrame& answer) const;

	public:
		D6Link() {}
	};

} // namespace RBX
