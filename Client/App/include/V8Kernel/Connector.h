/* V8Kernel/Connector.h -- October 2013 (kernel connector base).
   Evidence: KernelData::addConnector/removeConnector dispatch via
   vtbl+0 (getConnectorKernelType) + vtbl+28 (getBody(0/1))
   (batch221b/222 disasm-pinned slots); 5 index members +4..+20
   (humanoid/realTime/secondPass/joint/contact, C2 -1 -- RotateConnector
   C2 0x5264CA inits them, batch244); no base C2 symbol (deriveds init
   directly -- ours provides a default C2 with the same values so a
   future derived can't leave garbage).
   Shapes: 2016 Connector.h carried (first-party) minus buoyancy
   (2013 has 5 lists/indices; KernelType CONTACT 0 / JOINT 1 /
   HUMANOID 2 / KERNEL_JOINT 3 -- usage-pinned: 3->secondPass,
   1->joint, 2->humanoid, 0->contact).
   Stepper virtuals (computeForce/computeImpulse/stepGoals): stepper
   flow, not declared here.
   Original file: Client/App/include/V8Kernel/Connector.h (path from
   2016 + KernelData.cpp assert strings). */

#pragma once

namespace RBX {

	class Body;
	class Kernel;
	class KernelData;

	class Connector
	{
		friend class KernelData;
		friend class Kernel;

	public:
		typedef enum {
			CONTACT,
			JOINT,
			HUMANOID,
			KERNEL_JOINT
		} KernelType;

		typedef enum { body0, body1 } BodyIndex;

		Connector()
			: humanoidIndex(-1)
			, realTimeIndex(-1)
			, secondPassIndex(-1)
			, jointIndex(-1)
			, contactIndex(-1)
		{}

		virtual ~Connector() {}

		virtual KernelType getConnectorKernelType() const = 0;
		virtual Body* getBody(BodyIndex id) = 0;
		// Stepper dispatch (binary calls computeForce via vtbl+16 in
		// stepWorldThrottled; 2016 declares both here -- ContactConnector
		// overrides both).
		virtual void computeForce(bool throttling) = 0;
		virtual bool computeImpulse(float& residualVelocity) { return false; }

		// mac: 0x526464 (batch257: both bodies' Body+56 canThrottle AND).
		// Non-virtual in the binary (no vtbl slot; called directly).
		bool computeCanThrottle();

		bool isHumanoid() const { return humanoidIndex >= 0; }
		bool isRealTime() const { return realTimeIndex >= 0; }
		bool isSecondPass() const { return secondPassIndex >= 0; }
		bool isJoint() const { return jointIndex >= 0; }
		bool isContact() const { return contactIndex >= 0; }
		bool isInKernel() const {
			return isHumanoid() || isRealTime() || isSecondPass()
				|| isJoint() || isContact();
		}

	private:
		int humanoidIndex;    // +4.
		int realTimeIndex;    // +8.
		int secondPassIndex;  // +12.
		int jointIndex;       // +16.
		int contactIndex;     // +20.
	};

} // namespace RBX
