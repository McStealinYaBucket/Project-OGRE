
#pragma once
#include "rbxformat.h"
#include "rbx/boost.hpp"
#include "g3d/format.h"
#include <boost/thread/tss.hpp>	// Context::current() is inline (see there)

namespace RBX
{
	namespace Security
	{
		// 2013: 10 identities in both targets, StudioPlugin unconditional (isInRole PLAYER 0x0079f650 and
		// STUDIO 0x008b5c60: cmp identity,9; slot 6 shares LocalGUI_/CmdLine_'s arm).
		typedef enum { 
			Anonymous = 0, 
			LocalGUI_,					// Any action initiated by Roblox Studio or the mouse 
			GameScript_,				// Execution of a BaseScript object inside any DataModel
			GameScriptInRobloxPlace_,	// Execution of a BaseScript object inside any DataModel, if the place was authored by Roblox
			RobloxGameScript_,			// Execution of a BaseScript object written by Roblox inside any DataModel
			CmdLine_,					// Any script executed from the Studio command line
			StudioPlugin,				// Any Studio plug-in script
			COM,						// Scripts executed via the COM API (usually comes from roblox.com)
			WebService,					// Scripts executed via the Web Service API (usually comes from roblox.com)
			Replicator_,				// Receiving data via replication
			COUNT_Identities            // Not a true identity. Used for enumeration
		} Identities;

		// 2013 values (isInRole 0x0079f650 jump table; ReflectionMetadata writer 0x00448034): no Plugin.
		typedef enum { 
			None				=0,     // Any identity can access this feature, including in-game scripts
			RobloxPlace			=1,     // A Roblox place that we own. Therefore scripts are more trusted and we allow
										// preliminary features
			LocalUser			=2,     // non-game permission. Usually for IDE
			WritePlayer			=3,		// Permissions for changing player name, userId, etc.
			RobloxScript		=4,     // A script, such as a CoreScript, that we run inside a game
			Roblox				=5,     // Highest level of permission

#ifdef RBX_TEST_BUILD
			TestLocalUser       =None,  //For exposing Lua functions to the ReleaseTest build
#else
			TestLocalUser       =LocalUser,
#endif
		} Permissions;
        
        // different classes of VM that derive from the permission level
        typedef enum {
            VM_Default = 0,             // most scripts go here
#if defined(RBX_STUDIO_BUILD)
			VM_StudioPlugin,            // Sandbox for studio plugin scripts
#endif
            VM_RobloxScriptPlus,            // scripts with the permission level of RobloxScript or higher go here
            COUNT_VM_Classes
        } VMClasses;

		class Impersonator;

		class Context
		{
			friend class Impersonator;
					
		public:
			const Identities identity;

			// Inline in 2013: the kept copy is Network's (PLAYER 0x0053ac40, between the Player.cpp
			// functions). A thread without a context gets a heap Context(Anonymous).
			// FUNCTION: PLAYER 0x0053ac40
			// FUNCTION: STUDIO 0x005e9cb0
			static Context& current()
			{
				Context* t = ptr().get();
				if (!t)
				{
					t = new Context(Anonymous);
					ptr().reset(t);
				}
				return *t;
			}

			// Throws an exception if the current thread's Context doesn't have the requested Role.
			// mac: 0x546150 (126B, batch439); always throws an EMPTY runtime_error. Inline here: Windows
			// inlines it into Bridge<EventInstance>::on_index 0x0087ee40 and keeps an out-of-line copy.
			// FUNCTION: PLAYER 0x00527310
			// FUNCTION: STUDIO 0x005d7ec0
			void requirePermission(Permissions permission, const char* operation = 0) const
			{
				if (!isInRole(identity, permission))
					throw RBX::runtime_error("");
			}

			bool hasPermission(Permissions permission)
			{
				return isInRole(identity, permission);
			}

			static bool isInRole(Identities identity, Permissions permission);

			static void tssCleanup(Context*);

		private:
			Context(Identities identity):identity(identity) {}
			static boost::thread_specific_ptr<Context>& ptr();
		};

		// Impersonates an identity for the lifetime of the object
		// 2013: one member (PLAYER 0x0041cf40 stores only `previous`). Its out-of-line bodies sit in the /GL
		// shell (PLAYER 0x0041cf40 / 0x0041cf80, STUDIO 0x00461ce0 / 0x004619a0) and the engine calls them
		// (Game::doClearVerbs 0x006542d7), so they are not inline here.
		class Impersonator
		{
			Context* previous;
		public:
			Impersonator(Identities identity);
			~Impersonator();
		};
	}
}

