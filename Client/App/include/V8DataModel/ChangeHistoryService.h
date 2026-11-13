/* V8DataModel/ChangeHistoryService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: referenced from DataModel::clearContents 0x2CF1D0 / doCloseDataModel
     0x2D3952 (doCloseDataModel 0x2D3952 finds it + setEnabled(false); clearContents 0x2CF1D0 uses it.); "ChangeHistoryService" string check TODO per service.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ChangeHistoryService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sChangeHistoryService;

	class ChangeHistoryService
		: public DescribedNonCreatable<ChangeHistoryService, Instance, sChangeHistoryService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		enum RuntimeUndoBehavior
		{
			Aggregate = 0,
			Snapshot = 1,
			Hybrid = 2,
		};

		ChangeHistoryService() {}

		void clearWaypoints();  // 0x2CF1D0 calls this (waypoint list at +29/+116). TODO: body.
		void setEnabled(bool enabled);  // 0x2D3952: doCloseDataModel setEnabled(false). TODO: body.
	};

} // namespace RBX
