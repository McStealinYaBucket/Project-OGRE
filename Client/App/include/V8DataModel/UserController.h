/* V8DataModel/UserController.h -- October 2013, ControllerService only so far.
   The 2013 asserts at v8datamodel/UserController.cpp:73 name ServiceProvider::find<ControllerService>;
   RTTI DescribedNonCreatable<ControllerService, Instance, sControllerService, 1, 0> gives the base
   (functionality INTERNAL_LOCAL). Size 0x68 (Creatable::create PLAYER 0x00469490: new(0x68)). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "decomp.h"

namespace RBX {

	extern const char* const sControllerService;

	// SIZE 0x68
	class ControllerService
		: public DescribedNonCreatable<ControllerService, Instance, sControllerService, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		ControllerService();

	private:
		undefined4 m_unk0x64;	// 0x64 zeroed by the ctor
	};

} // namespace RBX
