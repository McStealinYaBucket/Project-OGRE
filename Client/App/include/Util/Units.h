/* Util/Units.h -- October 2013 (KMS <-> RBX unit conversions).
   2016-verbatim first-party header (EAs queue with the stepper; read by
   SimBody::update 0x534E72). */

#pragma once

#include "G3D/Vector3.h"

namespace RBX {

	class Units
	{
	public:
		static float mPerRbx() { return 0.05f; }
		static float rbxPerM() { return 20.0f; }
		static float rbxPerMM() { return rbxPerM() * 0.001f; }
		static G3D::Vector3 kmsVelocityToRbx(const G3D::Vector3& kmsVelocity);
		static float kmsAccelerationToRbx(float kmsAccel);
		static G3D::Vector3 kmsAccelerationToRbx(const G3D::Vector3& kmsAccel);
		static float kmsForceToRbx(float kmsForce);
		static G3D::Vector3 kmsForceToRbx(const G3D::Vector3& kmsForce);
		static G3D::Vector3 kmsTorqueToRbx(const G3D::Vector3& kmsTorque);
		static float kmsDensityToRbx(float kmsDensity);
		static G3D::Vector3 kmsKRotToRbx(const G3D::Vector3& kmsKRot);
		static G3D::Vector3 kmsKRotDampToRbx(const G3D::Vector3& kmsKRotDamp);
	};
}	// namespace
