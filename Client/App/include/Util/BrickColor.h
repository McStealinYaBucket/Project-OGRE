/* Decompile of October 2013 RBX::BrickColor (evidence: decomp_BrickColor_studio.cpp, brickcolor_table.txt)
   Cross-checked against March 2016 source: App/include/Util/BrickColor.h */

#pragma once

#include "G3D/Color3.h"
#include "G3D/Color4.h"
#include "G3D/Color3uint8.h"
#include "G3D/Color4uint8.h"

#include <vector>
#include <string>

namespace RBX {

	class BrickColor
	{
		class BrickMap;
	public:
		enum Number {
			brick_1 = 1,
			brick_2 = 2,
			brick_3 = 3,
			brick_5 = 5,
			brick_6 = 6,
			brick_9 = 9,
			brick_11 = 11,
			brick_12 = 12,
			brick_18 = 18,
			brick_21 = 21,
			brick_22 = 22,
			brick_23 = 23,
			brick_24 = 24,
			brick_25 = 25,
			brick_26 = 26,
			brick_27 = 27,
			brick_28 = 28,
			brick_29 = 29,
			brick_36 = 36,
			brick_37 = 37,
			brick_38 = 38,
			brick_39 = 39,
			brick_40 = 40,
			brick_41 = 41,
			brick_42 = 42,
			brick_43 = 43,
			brick_44 = 44,
			brick_45 = 45,
			brick_47 = 47,
			brick_48 = 48,
			brick_49 = 49,
			brick_50 = 50,
			brick_100 = 100,
			brick_101 = 101,
			brick_102 = 102,
			brick_103 = 103,
			brick_104 = 104,
			brick_105 = 105,
			brick_106 = 106,
			brick_107 = 107,
			brick_108 = 108,
			brick_110 = 110,
			brick_111 = 111,
			brick_112 = 112,
			brick_113 = 113,
			brick_115 = 115,
			brick_116 = 116,
			brick_118 = 118,
			brick_119 = 119,
			brick_120 = 120,
			brick_121 = 121,
			brick_123 = 123,
			brick_124 = 124,
			brick_125 = 125,
			brick_126 = 126,
			brick_127 = 127,
			brick_128 = 128,
			brick_131 = 131,
			brick_133 = 133,
			brick_134 = 134,
			brick_135 = 135,
			brick_136 = 136,
			brick_137 = 137,
			brick_138 = 138,
			brick_140 = 140,
			brick_141 = 141,
			brick_143 = 143,
			brick_145 = 145,
			brick_146 = 146,
			brick_147 = 147,
			brick_148 = 148,
			brick_149 = 149,
			brick_150 = 150,
			brick_151 = 151,
			brick_153 = 153,
			brick_154 = 154,
			brick_157 = 157,
			brick_158 = 158,
			brick_168 = 168,
			brick_176 = 176,
			brick_178 = 178,
			brick_179 = 179,
			brick_180 = 180,
			brick_190 = 190,
			brick_191 = 191,
			brick_192 = 192,
			brick_193 = 193,
			brick_194 = 194,
			brick_195 = 195,
			brick_196 = 196,
			brick_198 = 198,
			brick_199 = 199,
			brick_200 = 200,
			brick_208 = 208,
			brick_209 = 209,
			brick_210 = 210,
			brick_211 = 211,
			brick_212 = 212,
			brick_213 = 213,
			brick_216 = 216,
			brick_217 = 217,
			brick_218 = 218,
			brick_219 = 219,
			brick_220 = 220,
			brick_221 = 221,
			brick_222 = 222,
			brick_223 = 223,
			brick_224 = 224,
			brick_225 = 225,
			brick_226 = 226,
			brick_232 = 232,
			brick_268 = 268,
			roblox_1001 = 1001,
			roblox_1002 = 1002,
			roblox_1003 = 1003,
			roblox_1004 = 1004,
			roblox_1005 = 1005,
			roblox_1006 = 1006,
			roblox_1007 = 1007,
			roblox_1008 = 1008,
			roblox_1009 = 1009,
			roblox_1010 = 1010,
			roblox_1011 = 1011,
			roblox_1012 = 1012,
			roblox_1013 = 1013,
			roblox_1014 = 1014,
			roblox_1015 = 1015,
			roblox_1016 = 1016,
			roblox_1017 = 1017,
			roblox_1018 = 1018,
			roblox_1019 = 1019,
			roblox_1020 = 1020,
			roblox_1021 = 1021,
			roblox_1022 = 1022,
			roblox_1023 = 1023,
			roblox_1024 = 1024,
			roblox_1025 = 1025,
			roblox_1026 = 1026,
			roblox_1027 = 1027,
			roblox_1028 = 1028,
			roblox_1029 = 1029,
			roblox_1030 = 1030,
			roblox_1031 = 1031,
			roblox_1032 = 1032
		};
		Number number;
		typedef std::vector< BrickColor > Colors;
		static const Colors& colorPalette();
		static const Colors& renderingPalette();
		static const Colors& allColors();

		size_t getClosestRenderingPaletteIndex() const;
		size_t getClosestPaletteIndex() const;
		static const size_t paletteSize = 128;
		static const size_t paletteSizeMSB = 7;

		static void setRenderingSupportedPaletteSize(size_t maxSupportedColors);

		BrickColor(Number number):number(number) {}
		BrickColor():number(brick_194) {}
		explicit BrickColor(int number);
		static BrickColor closest(G3D::Color3uint8 color);
		static BrickColor closest(G3D::Color4uint8 color);
		static BrickColor closest(G3D::Color3 color);
		static BrickColor closest(G3D::Color4 color);
		static BrickColor parse(const char* name);
		static BrickColor random();

		G3D::Color4uint8 color4uint8() const;
		G3D::Color3uint8 color3uint8() const;
		G3D::Color4 color4() const;
		G3D::Color3 color3() const;
		const std::string& name() const;
		int asInt() const { return number; }

		bool operator==(const BrickColor& other) const { return number==other.number; }
		bool operator!=(const BrickColor& other) const { return number!=other.number; }
		bool operator>(const BrickColor& other) const { return number>other.number; }
		bool operator<(const BrickColor& other) const { return number<other.number; }
	};

	std::size_t hash_value(const BrickColor& c);

} // namespace
