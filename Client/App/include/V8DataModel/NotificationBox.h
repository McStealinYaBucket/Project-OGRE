/* V8DataModel/NotificationBox.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<NotificationBox, GuiObject> 0x650248;
     sNotificationBox = "NotificationBox".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NotificationBox.h */

#pragma once

#include "V8DataModel/GuiObject.h"

namespace RBX {

	extern const char* const sNotificationBox;

	class NotificationBox
		: public DescribedCreatable<NotificationBox, GuiObject, sNotificationBox>
	{
	public:
		NotificationBox() {}
	};

} // namespace RBX
