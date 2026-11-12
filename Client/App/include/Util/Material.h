/* Util/Material.h — October 2013 part-material enum.
   Evidence: EnumDesc<Material> 0x441DC4 (F:\2013Decompile\exports\decomp\
     Enum_Material.txt): 15 addPair values in verbatim order + 2 legacy
     aliases ("Corroded Metal" -> CorrodedMetal, "Aluminum" -> Foil) stuffed
     into the name map (spaces bar addPair, hence legacy slots).
     DIVERGENCE from March 2016 (PartMaterial with SMOOTH_PLASTIC/NEON/
     WOODPLANKS/etc.): 2013 has 15 materials; PLASTIC = 256 matches the
     FWPartInstance default (0x44AABA).
   Original file: (2013 path TODO; values binary-exact). */

#pragma once

namespace RBX {

	enum Material
	{
		Plastic = 256,
		Wood = 512,
		Slate = 800,
		Concrete = 816,
		CorrodedMetal = 1040,
		DiamondPlate = 1056,
		Foil = 1072,
		Grass = 1280,
		Ice = 1536,
		Marble = 784,
		Granite = 832,
		Brick = 848,
		Pebble = 864,
		Sand = 1296,
		Fabric = 1312
	};

} // namespace RBX
