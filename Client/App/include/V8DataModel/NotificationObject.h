/* V8DataModel/NotificationObject.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<NotificationObject, Frame> 0x652164;
     sNotificationObject = "NotificationObject".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NotificationObject.h */

#pragma once

#include "V8DataModel/Frame.h"

namespace RBX {

	extern const char* const sNotificationObject;

	class NotificationObject
		: public DescribedCreatable<NotificationObject, Frame, sNotificationObject>
	{
	public:
		NotificationObject() {}
	};

} // namespace RBX
