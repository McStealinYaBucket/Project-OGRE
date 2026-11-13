/* V8DataModel/GuiService.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<GuiService> from DataModel::initializeContents
     0x2D3336; "GuiService" string in both 2013 PEs.
   Bases (DescribedNonCreatable + Service) carried from March 2016 header.
   TODO: full decompile (selection/error-message machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sGuiService;

	class GuiService
		: public DescribedNonCreatable<GuiService, Instance, sGuiService, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		enum CenterDialogType
		{
			CENTER_DIALOG_UNSOLICITED_DIALOG = 1,
			CENTER_DIALOG_PLAYER_INITIATED_DIALOG = 2,
			CENTER_DIALOG_MODAL_DIALOG = 3,
			CENTER_DIALOG_QUIT_DIALOG = 4,
		};

		enum SpecialKey
		{
			KEY_INSERT = 0,
			KEY_HOME = 1,
			KEY_END = 2,
			KEY_PAGEUP = 3,
			KEY_PAGEDOWN = 4,
			KEY_CHATHOTKEY = 5,
		};

		GuiService() {}
	};

} // namespace RBX
