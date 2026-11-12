/* V8DataModel/Dialog.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<DialogRoot, Instance> 0x32F6B0 (Creator 0x331DEC); sDialogRoot="Dialog";
     sDialogRoot = "Dialog".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Dialog.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDialogRoot;

	class Dialog
		: public DescribedCreatable<Dialog, Instance, sDialogRoot>
	{
	public:
		Dialog() {}
	};

} // namespace RBX
