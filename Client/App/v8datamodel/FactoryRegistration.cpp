/* App/v8datamodel/FactoryRegistration.cpp -- October 2013 (class registration TU), partial.
   The 2013 binary records no __FILE__ for this TU; the name is the 2016 tree's (factoryregistration.cpp),
   with an uppercase F so the glob links it ahead of LuaSettings.cpp, as 2013's TU sits ahead of it.
   2013 evidence (PLAYER): a run of ClassRegistrar dynamic initializers (0x00a75a20...) calls
   GlobalAdvancedSettings::classDescriptor (0x005268b0) first and LuaSettings::classDescriptor
   (0x00788570) later; the TU's own classDescriptor copies sit in one block (0x00786d70-0x0078a800:
   PhysicsSettings 0x007874f0, LuaSettings 0x00788570, TimerService 0x00789270) and read each class
   name through its extern, as a TU that does not define the s-name does. Only these four
   registrations are here so far; the rest of the block's classes still register in their own
   .cpp files. */

#include "stdafx.h"

#include "V8DataModel/GlobalAdvancedSettings.h"
#include "V8DataModel/PhysicsSettings.h"
#include "V8DataModel/LuaSettings.h"
#include "V8DataModel/TimerService.h"

namespace RBX {
RBX_REGISTER_CLASS(RBX::GlobalAdvancedSettings);
RBX_REGISTER_CLASS(RBX::PhysicsSettings);
RBX_REGISTER_CLASS(RBX::LuaSettings);
RBX_REGISTER_CLASS(RBX::TimerService);
}
