/* V8DataModel/OverlayDataModel.h -- October 2013 (DataModel overlay shell).
   Evidence: Described<OverlayDataModel, sOverlayDataModel,
     NonFactoryProduct<DataModel, sOverlayDataModel>, Functionality 27
     (= PERSISTENT: 0x1+0x2+0x8+0x10), Permissions 0 (= None) -- openState
     0x18BB89 game-globals gate reads it (batches 632/668/671).
   TODO: s-string VALUE unconfirmed (symbol sOverlayDataModel exists;
     content carries the C++-name convention, flagged per unverifiable-
     literal policy -- no test pins it); full decompile queued.
   Original file: Client/App/v8datamodel/OverlayDataModel.h */

#pragma once

#include "V8DataModel/DataModel.h"

namespace RBX {

	extern const char* const sOverlayDataModel;

	class OverlayDataModel
		: public DescribedNonCreatable<OverlayDataModel, DataModel, sOverlayDataModel>
	{
	public:
		// No default ctor (mirrors DataModel(Verb*, DataModel*); binary
		// has no DataModelC1Ev either -- NonFactoryProduct default arm
		// never instantiates).
		OverlayDataModel(Verb* lockVerb, DataModel* parentDM)
			: DescribedNonCreatable<OverlayDataModel, DataModel, sOverlayDataModel>(lockVerb, parentDM) {}

		// iOS name and arguments (PLAYER 0x00693310, called by Game::Game).
		static shared_ptr<OverlayDataModel> createOverlayDataModel(bool flag, Verb* lockVerb);

		// Unnamed (inline in 2013; its copy sits in Game.obj): stores the game DataModel (a weak_ptr
		// argument, released at the end) at +0xe58. Game::Game calls it right after createDataModel.
		void FUN_00652f00(weak_ptr<DataModel> dataModel);
	};

} // namespace RBX
