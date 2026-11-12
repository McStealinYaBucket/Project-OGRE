/* V8Xml/Serializer.h — October 2013 (global Serializer filter entry point).
   Evidence: __ZN10Serializer13canWriteChildE (0x706B22) — global class, decompiled
     in BinHelpers.cpp.txt. Full filter map: FW+23 gate, SAVE_GAME(1) =
     {StarterGuiService, StarterPackService, ServerScriptService}, SAVE_WORLD(0) =
     {Workspace, Lighting, SoundService, ServerStorage, ReplicatedStorage},
     SAVE_ALL(2) = everything (see SerializerBinary.cpp).
   (Prior placeholder note superseded: the 2016 Serializer.h content is NOT used.) */

#pragma once

#include "V8Xml/SerializerV2.h"

class Serializer {
public:
	// mac: 0x706B22. saveFilter mapping TODO-verify (see SerializerBinary.cpp).
	static bool canWriteChild(shared_ptr<RBX::Instance> child, RBX::Instance::SaveFilter saveFilter);
};
