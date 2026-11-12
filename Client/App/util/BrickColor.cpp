/* BrickColor.cpp — decompiled from October 2013 binaries.
   Evidence: F:\2013Decompile\exports\decomp_BrickColor_studio.cpp (symbolled Studio 0.126 Mach-O)
     - BrickMap ctor 0x1EA94C (144 insert calls -> BrickColorTable.inc)
     - query functions 0x1E99D8..0x1E9D9A
     - ReleaseAssert strings embed the original file/line:
       "/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/BrickColor.cpp" lines 559-576
   Cross-checked against March 2016 App/include/Util/BrickColor.h.
   2013 palette = 144 colors; the 300+ Number series was added later than Oct 2013. */

#include "Util/BrickColor.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace RBX {

	//////////////////////////////////////////////////////////////////////////////////
	// BrickColor::BrickMap
	// Binary layout recovered from the decompilation (x86, sizeof(ColorInfo)==28):
	//   +0x00 bool valid; +0x04 G3D::Color3uint8 color; +0x08 G3D::Color4 color4;
	//   +0x18 std::string name
	// The original indexes a std::vector<ColorInfo> by Number (asserts reference
	// "colors.size()" / "colors[number].valid").
	//////////////////////////////////////////////////////////////////////////////////
	class BrickColor::BrickMap
	{
	public:
		struct ColorInfo {
			ColorInfo()
				:valid(false), color(0, 0, 0), color4(0.f, 0.f, 0.f, 1.f)
			{}
			ColorInfo(unsigned char r, unsigned char g, unsigned char b, const std::string& _name)
				:valid(true), color(r, g, b), color4(r / 255.f, g / 255.f, b / 255.f, 1.f), name(_name)
			{}

			bool				valid;
			G3D::Color3uint8	color;
			G3D::Color4			color4;
			std::string			name;
		};
		typedef std::vector<ColorInfo> ColorInfos;

		ColorInfos			colors;				// indexed by Number
		Colors				colorPalette;		// colors shown in UI
		Colors				renderingPalette;	// colors supported by renderer
		std::map<Number, int> closestPaletteIndexMap;

		BrickMap();

		void insert(Number number, unsigned char r, unsigned char g, unsigned char b, const std::string& name);
		void generatePaletteMap(std::map<Number, int>& indexMap, const Colors& palette, Number defaultColor);
		void generatePaletteMap();
		// Caps the renderer palette (2016 curates a hardcoded subset; the 2013
		// subset is unverified — first-N of the 144-color palette here. No
		// callers in the tree yet; exact 2013 list TODO.)
		void setRenderingSupportedPaletteSize(size_t maxSupportedColors)
		{
			if (maxSupportedColors == renderingPalette.size())
				return;
			renderingPalette.clear();
			for (size_t i = 0; i < maxSupportedColors && i < colorPalette.size(); ++i)
				renderingPalette.push_back(colorPalette[i]);
		}

		static BrickMap& singleton();
	};

	BrickColor::BrickMap& BrickColor::BrickMap::singleton()
	{
		static BrickMap theMap;		// guard-protected singleton in the original (__cxa_guard on Mach-O)
		return theMap;
	}

	// STUB: PLAYER 0x00790170
	// STUB: STUDIO 0x00848320
	BrickColor::BrickMap::BrickMap()
	{
		colors.resize(1033);		// highest Number in the Oct 2013 table is roblox_1032

		BrickMap& map = *this;
#include "BrickColorTable.inc"

		// 2013 colorPalette order (64 entries, EA 0x1EE1E5-0x1EE87B push_back
		// run, batch407; followed by setRenderingSupportedPaletteSize).
		// NOTE: an older comment here claimed a 144-color palette -- the
		// binary builds exactly these 64.
		static const Number paletteOrder[] = {
			(Number)119, (Number)24, (Number)106, (Number)21, (Number)104, (Number)23, (Number)107, (Number)37,
			(Number)1001, (Number)1, (Number)208, (Number)1002, (Number)194, (Number)199, (Number)26, (Number)1003,
			(Number)1022, (Number)105, (Number)125, (Number)153, (Number)1023, (Number)135, (Number)102, (Number)151,
			(Number)5, (Number)226, (Number)133, (Number)101, (Number)9, (Number)11, (Number)1018, (Number)29,
			(Number)1030, (Number)1029, (Number)1025, (Number)1016, (Number)1026, (Number)1024, (Number)1027, (Number)1028,
			(Number)1008, (Number)1009, (Number)1017, (Number)1004, (Number)1032, (Number)1010, (Number)1019, (Number)1020,
			(Number)217, (Number)18, (Number)38, (Number)1031, (Number)1006, (Number)1013, (Number)45, (Number)1021,
			(Number)192, (Number)1014, (Number)1007, (Number)1015, (Number)1012, (Number)1011, (Number)28, (Number)141
		};
		for (size_t i = 0; i < sizeof(paletteOrder) / sizeof(paletteOrder[0]); ++i)
			colorPalette.push_back(BrickColor(paletteOrder[i]));
	}

	void BrickColor::BrickMap::insert(Number number, unsigned char r, unsigned char g, unsigned char b, const std::string& name)
	{
		colors[number] = ColorInfo(r, g, b, name);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// BrickColor
	//////////////////////////////////////////////////////////////////////////////////
	// STUB: PLAYER 0x007922b0
	// STUB: STUDIO 0x0084a460
	BrickColor::BrickColor(int number)
	{
		// mac: 0x1E9BDE: validates against the colors vector, falls back to 194 (Medium stone grey)
		const BrickColor::BrickMap::ColorInfos& colors = BrickColor::BrickMap::singleton().colors;
		if (static_cast<unsigned int>(number) < colors.size() && colors[number].valid)
			this->number = static_cast<Number>(number);
		else
			this->number = brick_194;
	}

	const BrickColor::Colors& BrickColor::colorPalette()
	{
		return BrickMap::singleton().colorPalette;
	}

	const BrickColor::Colors& BrickColor::renderingPalette()
	{
		return BrickMap::singleton().renderingPalette;
	}

	BrickColor BrickColor::closest(G3D::Color3uint8 color)
	{
		G3D::Color4uint8 c4;
		c4.r = color.r;
		c4.g = color.g;
		c4.b = color.b;
		return closest(c4);
	}

	BrickColor BrickColor::closest(G3D::Color4uint8 color)
	{
		BrickColor::Number bestMatch = BrickColor().number;
		int closestDistance = 10000;

		const std::vector<BrickMap::ColorInfo>& colors = BrickMap::singleton().colors;

		for (size_t i = 0; i < colors.size(); ++i)
		{
			if (!colors[i].valid)
				continue;

			G3D::Color3uint8 match = colors[i].color;

			int distance = std::abs(match.r - color.r);
			distance += std::abs(match.g - color.g);
			distance += std::abs(match.b - color.b);
			if (distance < closestDistance) {
				bestMatch = static_cast<BrickColor::Number>(i);
				closestDistance = distance;
			}
		}
		return bestMatch;
	}

	BrickColor BrickColor::closest(G3D::Color3 color)
	{
		G3D::Color4 c4;
		c4.r = color.r;
		c4.g = color.g;
		c4.b = color.b;
		return closest(c4);
	}

	BrickColor BrickColor::closest(G3D::Color4 color)
	{
		BrickColor::Number bestMatch = BrickColor().number;
		float closestDistance = 10000;

		const std::vector<BrickMap::ColorInfo>& colors = BrickMap::singleton().colors;

		for (size_t i = 0; i < colors.size(); ++i)
		{
			if (!colors[i].valid)
				continue;

			G3D::Color4 match = colors[i].color4;

			float distance = (float)fabs(match.r - color.r);
			distance += (float)fabs(match.g - color.g);
			distance += (float)fabs(match.b - color.b);
			if (distance < closestDistance) {
				bestMatch = static_cast<BrickColor::Number>(i);
				closestDistance = distance;
			}
		}
		return bestMatch;
	}

	const BrickColor::Colors& BrickColor::allColors()
	{
		// returns colors vector as BrickColors
		static Colors all;
		// TODO(decompile): built from BrickMap colors vector; see 0x1E9? entry
		return all;
	}

	size_t BrickColor::getClosestPaletteIndex() const
	{
		// std::map<Number,int>::operator[] lookup in the original (0x1E99F4)
		std::map<Number, int>::const_iterator it =
			BrickMap::singleton().closestPaletteIndexMap.find(number);
		return it != BrickMap::singleton().closestPaletteIndexMap.end() ? it->second : 0;
	}

	G3D::Color4uint8 BrickColor::color4uint8() const
	{
		// assert: "static_cast<unsigned int>(number) < colors.size() && colors[number].valid" (line 559)
		const BrickColor::BrickMap::ColorInfo& info = BrickMap::singleton().colors[number];
		return G3D::Color4uint8(info.color.r, info.color.g, info.color.b, 255);
	}

	G3D::Color3uint8 BrickColor::color3uint8() const
	{
		G3D::Color4uint8 c4 = color4uint8();
		return G3D::Color3uint8(c4.r, c4.g, c4.b);
	}

	// STUB: PLAYER 0x00792460
	// STUB: STUDIO 0x0084a610
	G3D::Color4 BrickColor::color4() const
	{
		// assert line 576; returns colors[number].color4 (offset +0x08, 16 bytes)
		return BrickMap::singleton().colors[number].color4;
	}

	G3D::Color3 BrickColor::color3() const
	{
		G3D::Color4 c4 = color4();
		return G3D::Color3(c4.r, c4.g, c4.b);
	}

	const std::string& BrickColor::name() const
	{
		// assert line 570; returns colors[number].name (offset +0x18)
		return BrickMap::singleton().colors[number].name;
	}

	BrickColor BrickColor::parse(const char* name)
	{
		// linear scan over colors[].name; defaults to 194 (Medium stone grey) (0x1E9A24)
		const BrickColor::BrickMap::ColorInfos& colors = BrickMap::singleton().colors;
		for (size_t i = 0; i < colors.size(); ++i) {
			if (colors[i].valid && colors[i].name == name)
				return static_cast<Number>(i);
		}
		return BrickColor();
	}

	BrickColor BrickColor::random()
	{
		// picks a random entry from colorPalette (0x1E9AA8)
		const Colors& palette = BrickMap::singleton().colorPalette;
		return palette.empty() ? BrickColor() : palette[G3D::iRandom(0, (int)palette.size() - 1)];
	}

	void BrickColor::setRenderingSupportedPaletteSize(size_t maxSupportedColors)
	{
		const_cast<BrickMap&>(BrickMap::singleton()).setRenderingSupportedPaletteSize(maxSupportedColors);
	}

	std::size_t hash_value(const BrickColor& c)
	{
		return c.number;
	}

} // namespace RBX
