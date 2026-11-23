/* V8DataModel/Lighting.h — October 2013.
   Layout from the PLAYER constructor 0x00707ba0 (STUDIO 0x007d39e0; operator new 0x1f0 in
   Creatable<Instance>::create<Lighting> 0x0044b0f0) and the setters it pairs with. Names from
   iOS 10-29 (`setShadowColor(G3D::Color4)`, `setShadowColor3(G3D::Color3)`, `getMoonPhase()`, ...).
   2013 differs from 2016: the clear, shadow and fog colors are G3D::Color4 (alpha 1.0 at
   0x170/0x180/0x190) behind Color3 property accessors; setGeographicLatitude stores
   geoLatitude directly (no setLatitude/setTime); setGlobalShadows has no analytics call_once;
   there is no GlobalAmbient setter in PLAYER (STUDIO has one, 0x007d2610).
   Original file: Client/App/v8datamodel/Lighting.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "decomp.h"
#include "G3D/Color3.h"
#include "G3D/Color4.h"
#include "G3D/Vector3.h"
#include "G3D/LightingParameters.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace RBX {

	extern const char* const sLighting;

	class Sky;

	// 2013's G3D::LightingParameters is 0xf8 bytes: setTime 0x009e3b70 never writes past +0xf4
	// (geoLatitude) and Lighting::hasSky sits at 0x68 + 0xf8. Our G3D fork's header adds
	// skyAmbient2 (0x108 bytes); that header belongs to the Rendering side. Every other offset
	// agrees (lightColor +0x24, physicallyCorrect +0x4c, moonPhase +0x80, geoLatitude +0xf4), so
	// this holder keeps 2013's size and hands out the G3D type. Replace it with a plain
	// G3D::LightingParameters member once the G3D header drops skyAmbient2.
	class SkyParameters2013
	{
		union
		{
			double align;					// LightingParameters::moonPhase is a double
			char bytes[0xf8];
		} storage;

	public:
		SkyParameters2013()
		{
			get().G3D::LightingParameters::LightingParameters();
		}
		G3D::LightingParameters& get() { return *reinterpret_cast<G3D::LightingParameters*>(&storage); }
		const G3D::LightingParameters& get() const { return *reinterpret_cast<const G3D::LightingParameters*>(&storage); }
		G3D::LightingParameters* operator->() { return &get(); }
		const G3D::LightingParameters* operator->() const { return &get(); }
	};

	// 2013 RTTI says functionality 11 (PERSISTENT_HIDDEN, as 2016: DescribedCreatable<...$0L@...>).
	// With it, the Lighting template bodies fold with Teams' identical ones in our link and four
	// 100% functions plus FactoryProduct<Lighting>'s vftable lose their pairing (measured
	// 2026-11-13), so the default stays until the fold issue is solved.
	// SIZE 0x1f0
	class Lighting
		: public DescribedCreatable<Lighting, Instance, sLighting>
		, public Service
	{
	private:
		typedef DescribedCreatable<Lighting, Instance, sLighting> Super;

		SkyParameters2013 skyParameters;			// 0x68  ctor 0x00707bf1
		bool hasSky;								// 0x160
		G3D::Color4 clearColor;						// 0x164  setClearColor 0x007067e0
		G3D::Color4 shadowColor;					// 0x174  setShadowColor 0x00706860
		G3D::Color4 fogColor;						// 0x184  setFogColor3 0x007057b0
		float fogStart;								// 0x194  setFogStart 0x00704f80
		float fogEnd;								// 0x198  setFogEnd 0x00704fb0
		boost::posix_time::time_duration timeOfDay;	// 0x1a0  setTime 0x00706a30
		G3D::Color3 topColorShift;					// 0x1a8  BoundProp ColorShift_Top
		G3D::Color3 bottomColorShift;				// 0x1b4  BoundProp ColorShift_Bottom
		float globalBrightness;						// 0x1c0  BoundProp Brightness (initializer 0x00a70470)
		G3D::Color3 globalAmbient;					// 0x1c4  BoundProp Ambient
		G3D::Color3 globalOutdoorAmbient;			// 0x1d0  BoundProp OutdoorAmbient
		bool globalShadows;							// 0x1dc  setGlobalShadows 0x00704fe0
		bool outlines;								// 0x1dd  BoundProp Outlines

	public:
		Lighting();

		rbx::signal<void(bool)> lightingChangedSignal;		// 0x1e0 (arg=skyboxChanged)
		shared_ptr<Sky> sky;								// 0x1e4
		// 0x1ec: padding (the class is 8-aligned through timeOfDay)

		static Reflection::BoundProp<float> desc_GlobalBrightness;
		static Reflection::BoundProp<G3D::Color3> desc_TopColorShift;
		static Reflection::BoundProp<G3D::Color3> desc_BottomColorShift;
		static Reflection::BoundProp<G3D::Color3> desc_GlobalAmbient;
		static Reflection::BoundProp<G3D::Color3> desc_GlobalOutdoorAmbient;
		static Reflection::BoundProp<bool> desc_Outlines;

		const G3D::LightingParameters& getSkyParameters() const { return skyParameters.get(); }
		bool isSkySuppressed() const { return !hasSky; }

		std::string getTimeStr() const;
		void setTimeStr(const std::string& value);
		void setTime(const boost::posix_time::time_duration& value);
		G3D::GameTime getGameTime() const { return timeOfDay.total_seconds(); }
		double getMinutesAfterMidnight();
		void setMinutesAfterMidnight(double value);

		// FUNCTION: PLAYER 0x00704ed0
		// FUNCTION: STUDIO 0x007d0b20
		float getMoonPhase() { return (float)skyParameters->moonPhase; }
		G3D::Vector3 getMoonPosition();
		G3D::Vector3 getSunPosition();

		// FUNCTION: PLAYER 0x00704ee0
		// FUNCTION: STUDIO 0x007d0b30
		float getGeographicLatitude() const { return skyParameters->geoLatitude; }
		void setGeographicLatitude(float value);

		void setClearColor(G3D::Color4 value);
		void setShadowColor(G3D::Color4 value);
		void setFogColor(G3D::Color4 value);

		G3D::Color3 getShadowColor3() const { return shadowColor.rgb(); }
		void setShadowColor3(G3D::Color3 value);

		// FUNCTION: PLAYER 0x00704ef0
		// FUNCTION: STUDIO 0x007d0b40
		G3D::Color3 getFogColor3() const { return fogColor.rgb(); }

		// FUNCTION: PLAYER 0x007057b0
		// FUNCTION: STUDIO 0x007d13e0
		void setFogColor3(G3D::Color3 value) { setFogColor(G3D::Color4(value)); }

		float getFogStart() const { return fogStart; }
		void setFogStart(float value);
		float getFogEnd() const { return fogEnd; }
		void setFogEnd(float value);
		bool getGlobalShadows() const { return globalShadows; }
		void setGlobalShadows(bool value);

		G3D::Color3 getTopColorShift() const { return topColorShift; }
		G3D::Color3 getBottomColorShift() const { return bottomColorShift; }
		float getGlobalBrightness() const { return globalBrightness; }
		G3D::Color3 getGlobalAmbient() const { return globalAmbient; }
		G3D::Color3 getGlobalOutdoorAmbient() const { return globalOutdoorAmbient; }
		bool getOutlines() const { return outlines; }

		void replaceSky(Sky* newSky);

	protected:
		/*override*/ void onChildAdded(Instance* child);
		/*override*/ void onChildRemoving(Instance* child);
		/*override*/ void onChildChanged(Instance* instance, const PropertyChanged& event);
		/*override*/ bool askAddChild(const Instance* instance) const;

	private:
		void fireLightingChanged(bool skyboxChanged) { lightingChangedSignal(skyboxChanged); }

		// FUNCTION: PLAYER 0x00706a10
		// FUNCTION: STUDIO 0x007d27c0
		void onPropChanged(const Reflection::PropertyDescriptor&) { fireLightingChanged(false); }
	};

} // namespace RBX

// SYNTHETIC: PLAYER 0x00707e40
// SYNTHETIC: STUDIO 0x007d3c80
// RBX::Lighting::~Lighting

// SYNTHETIC: PLAYER 0x00707ef0
// SYNTHETIC: STUDIO 0x007d3d30
// RBX::Lighting::`scalar deleting destructor'

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x007057a0
// TEMPLATE: STUDIO 0x007d13d0
// ??_E?$FactoryProduct@VLighting@RBX@@VInstance@2@$E?sLighting@2@3QBDBV32@@RBX@@OBM@AEPAXI@Z

// TEMPLATE: PLAYER 0x00705b00
// TEMPLATE: STUDIO 0x007d0c60
// ??1?$DescribedCreatable@VLighting@RBX@@VInstance@2@$E?sLighting@2@3QBDB$0BL@$0A@@RBX@@UAE@XZ

// TEMPLATE: PLAYER 0x007075e0
// TEMPLATE: STUDIO 0x007d3390
// ??0?$Described@VLighting@RBX@@$E?sLighting@2@3QBDBV?$FactoryProduct@VLighting@RBX@@VInstance@2@$E?sLighting@2@3QBDBV32@@2@$0BL@$0A@@Reflection@RBX@@QAE@XZ

// boost::posix_time::to_simple_string(time_duration), out of line for getTimeStr.
// TEMPLATE: PLAYER 0x00706150
// TEMPLATE: STUDIO 0x007d1cf0
// ?to_simple_string@posix_time@boost@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@Vtime_duration@12@@Z

// boost::posix_time::duration_from_string's parser, out of line for the ctor and setTimeStr.
// TEMPLATE: PLAYER 0x00706330
// TEMPLATE: STUDIO 0x007d1f70
// ??$str_from_delimited_time_duration@Vtime_duration@posix_time@boost@@D@date_time@boost@@YA?AVtime_duration@posix_time@1@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z
