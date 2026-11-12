/* App/util/Units.cpp -- October 2013 (KMS <-> RBX conversions).
   2016-verbatim (EAs queue with the stepper). */

#include "stdafx.h"

#include "Util/Units.h"

namespace RBX {

	G3D::Vector3 Units::kmsVelocityToRbx(const G3D::Vector3& kmsVelocity)
	{
		return kmsVelocity * rbxPerM();
	}

	G3D::Vector3 Units::kmsAccelerationToRbx(const G3D::Vector3& kmsAccel)
	{
		return kmsAccel * rbxPerM();
	}

	float Units::kmsAccelerationToRbx(float kmsAccel)
	{
		return kmsAccel * rbxPerM();
	}

	G3D::Vector3 Units::kmsForceToRbx(const G3D::Vector3& kmsForce)
	{
		return kmsForce * rbxPerM();
	}

	float Units::kmsForceToRbx(float kmsForce)
	{
		return kmsForce * rbxPerM();
	}

	G3D::Vector3 Units::kmsTorqueToRbx(const G3D::Vector3& kmsTorque)
	{
		return kmsTorque * rbxPerM() * rbxPerM();
	}

	G3D::Vector3 Units::kmsKRotToRbx(const G3D::Vector3& kmsKRot)
	{
		return kmsKRot * rbxPerM() * rbxPerM();
	}

	G3D::Vector3 Units::kmsKRotDampToRbx(const G3D::Vector3& kmsKRotDamp)
	{
		return kmsKRotDamp * rbxPerM() * rbxPerM();
	}

	float Units::kmsDensityToRbx(float kmsDensity)
	{
		return kmsDensity * rbxPerM() * rbxPerM() * rbxPerM();
	}

} // namespace
