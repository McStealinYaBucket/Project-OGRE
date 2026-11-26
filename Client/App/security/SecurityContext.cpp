#include "stdafx.h"
#include "security/SecurityContext.h"
#include "rbx/Thread.hpp"
#include "rbx/Debug.h"
namespace RBX {
	namespace Security {
		// FUNCTION: PLAYER 0x0079f650
		// FUNCTION: STUDIO 0x008b5c60
		bool Context::isInRole(Identities identity, Permissions p)
		{
			if (p == None)
				return true;
			switch (identity)
			{
			case Anonymous:
			case GameScript_:
				return false;
			case GameScriptInRobloxPlace_:
				return                p == RobloxPlace;
			case RobloxGameScript_:
				return p == RobloxPlace || p == LocalUser || p == RobloxScript;
			case LocalGUI_:
			case CmdLine_:
			case StudioPlugin:
				return p == RobloxPlace || p == LocalUser;
			case Replicator_:
				return                p == RobloxPlace ||                   p == WritePlayer  || p == RobloxScript;
			case COM:
			case WebService:
				return true;
			// any other identity is a bug (assert on 2013 line 33)
			default:
				RBXASSERT(false);
				return false;
			}
		}




		void Context::tssCleanup(Context*)
		{

		}

		boost::thread_specific_ptr<Context>& Context::ptr()
		{
			static boost::thread_specific_ptr<Context> value(tssCleanup);
			return value;
		}

		// STUB: PLAYER 0x0041cf40
		// STUB: STUDIO 0x00461ce0
		Impersonator::Impersonator(Identities identity)
		{
			Context* context = new Context(identity);
			previous = Context::ptr().release();
			Context::ptr().reset(context);
		}

		// STUB: PLAYER 0x0041cf80
		// STUB: STUDIO 0x004619a0
		Impersonator::~Impersonator()
		{
			Context::ptr().reset(previous);
		}



	}
}
