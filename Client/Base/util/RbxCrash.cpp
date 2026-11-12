/*
 *  RbxCrash.cpp
 *  MacClient
 *
 *  Created by elebel on 7/20/10.
 *  Copyright 2010 Roblox. All rights reserved.
 *
 */

/* Base/util/RbxCrash.cpp — vendored from March 2016 reference (tiny, version-neutral). */

#include "rbx/Debug.h"

void RBXCRASH()
{
	RBX::Debugable::doCrash();
}

// FUNCTION: PLAYER 0x009b9090
// FUNCTION: STUDIO 0x00a68e10
void RBXCRASH(const char* message)
{
	RBX::Debugable::doCrash(message);
}
