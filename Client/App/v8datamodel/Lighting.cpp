/* App/v8datamodel/Lighting.cpp — October 2013. Layout and evidence in V8DataModel/Lighting.h.
   Descriptors in the order of their 2013 dynamic initializers (PLAYER 0x00a6ffd0 .. 0x00a705b0;
   the objects sit at 0x00df2ea8..0x00df3298, paired by the generated libdata rows: reccmp's
   parser does not take a GLOBAL marker above a constructor-style definition);
   functions in PLAYER address order (the .obj starts at getMoonPhase 0x00704ed0).
   onChildRemoving 0x007068f0 and onChildAdded 0x00707510 were kept out on 2026-11-13 (their
   shared_ptr<Sky>::reset / shared_from<Sky> instantiations join folded template bodies); they are
   back in, measured with regenerated CSVs. */

#include "stdafx.h"

#include "V8DataModel/Lighting.h"
#include "V8DataModel/Sky.h"
#include "boost/date_time/posix_time/time_formatters.hpp"
#include "boost/date_time/posix_time/time_parsers.hpp"

DECOMP_SIZE_ASSERT(RBX::Lighting, 0x1f0);

namespace RBX {
	const char* const sLighting = "Lighting";

	REFLECTION_BEGIN();
	static Reflection::EventDesc<Lighting, void(bool)> event_LightingChanged(&Lighting::lightingChangedSignal, "LightingChanged", "skyboxChanged");
	static Reflection::PropDescriptor<Lighting, std::string> prop_TimeOfDay("TimeOfDay", category_Data, &Lighting::getTimeStr, &Lighting::setTimeStr);
	static Reflection::PropDescriptor<Lighting, float> prop_GeographicLatitude("GeographicLatitude", category_Data, &Lighting::getGeographicLatitude, &Lighting::setGeographicLatitude);
	static Reflection::BoundFuncDesc<Lighting, float()> func_GetMoonPhase(&Lighting::getMoonPhase, "GetMoonPhase", Security::None);
	static Reflection::BoundFuncDesc<Lighting, G3D::Vector3()> func_GetMoonDirection(&Lighting::getMoonPosition, "GetMoonDirection", Security::None);
	static Reflection::BoundFuncDesc<Lighting, G3D::Vector3()> func_GetSunDirection(&Lighting::getSunPosition, "GetSunDirection", Security::None);
	static Reflection::BoundFuncDesc<Lighting, double()> func_GetMinutes(&Lighting::getMinutesAfterMidnight, "GetMinutesAfterMidnight", Security::None);
	static Reflection::BoundFuncDesc<Lighting, void(double)> func_SetMinutes(&Lighting::setMinutesAfterMidnight, "SetMinutesAfterMidnight", "minutes", Security::None);
	static Reflection::BoundFuncDesc<Lighting, double()> dep_getMinutes(&Lighting::getMinutesAfterMidnight, "getMinutesAfterMidnight", Security::None, Reflection::Descriptor::Attributes::deprecated(func_GetMinutes));
	static Reflection::BoundFuncDesc<Lighting, void(double)> dep_setMinutes(&Lighting::setMinutesAfterMidnight, "setMinutesAfterMidnight", "minutes", Security::None, Reflection::Descriptor::Attributes::deprecated(func_SetMinutes));
	static Reflection::PropDescriptor<Lighting, G3D::Color3> prop_ShadowColor("ShadowColor", category_Appearance, &Lighting::getShadowColor3, &Lighting::setShadowColor3);
	static Reflection::PropDescriptor<Lighting, G3D::Color3> prop_FogColor("FogColor", "Fog", &Lighting::getFogColor3, &Lighting::setFogColor3);
	static Reflection::PropDescriptor<Lighting, float> prop_FogStart("FogStart", "Fog", &Lighting::getFogStart, &Lighting::setFogStart);
	static Reflection::PropDescriptor<Lighting, float> prop_FogEnd("FogEnd", "Fog", &Lighting::getFogEnd, &Lighting::setFogEnd);
	static Reflection::PropDescriptor<Lighting, bool> prop_GlobalShadows("GlobalShadows", category_Appearance, &Lighting::getGlobalShadows, &Lighting::setGlobalShadows);
	Reflection::BoundProp<float> Lighting::desc_GlobalBrightness("Brightness", category_Appearance, &Lighting::globalBrightness, &Lighting::onPropChanged);
	Reflection::BoundProp<G3D::Color3> Lighting::desc_TopColorShift("ColorShift_Top", category_Appearance, &Lighting::topColorShift, &Lighting::onPropChanged);
	Reflection::BoundProp<G3D::Color3> Lighting::desc_BottomColorShift("ColorShift_Bottom", category_Appearance, &Lighting::bottomColorShift, &Lighting::onPropChanged);
	Reflection::BoundProp<G3D::Color3> Lighting::desc_GlobalAmbient("Ambient", category_Appearance, &Lighting::globalAmbient, &Lighting::onPropChanged);
	Reflection::BoundProp<G3D::Color3> Lighting::desc_GlobalOutdoorAmbient("OutdoorAmbient", category_Appearance, &Lighting::globalOutdoorAmbient, &Lighting::onPropChanged);
	Reflection::BoundProp<bool> Lighting::desc_Outlines("Outlines", category_Appearance, &Lighting::outlines, &Lighting::onPropChanged);
	REFLECTION_END();

	// FUNCTION: PLAYER 0x00704f20
	// FUNCTION: STUDIO 0x007d0b70
	G3D::Vector3 Lighting::getMoonPosition()
	{
		return skyParameters->physicallyCorrect ? skyParameters->trueMoonPosition : skyParameters->moonPosition;
	}

	// FUNCTION: PLAYER 0x00704f50
	// FUNCTION: STUDIO 0x007d0ba0
	G3D::Vector3 Lighting::getSunPosition()
	{
		return skyParameters->physicallyCorrect ? skyParameters->trueSunPosition : skyParameters->sunPosition;
	}

	// FUNCTION: PLAYER 0x00704f80
	// FUNCTION: STUDIO 0x007d0bd0
	void Lighting::setFogStart(float value)
	{
		if (value != fogStart)
		{
			fogStart = value;
			raisePropertyChanged(prop_FogStart);
		}
	}

	// FUNCTION: PLAYER 0x00704fb0
	// FUNCTION: STUDIO 0x007d0c00
	void Lighting::setFogEnd(float value)
	{
		if (value != fogEnd)
		{
			fogEnd = value;
			raisePropertyChanged(prop_FogEnd);
		}
	}

	// FUNCTION: PLAYER 0x00704fe0
	// FUNCTION: STUDIO 0x007d0c30
	void Lighting::setGlobalShadows(bool value)
	{
		if (value != globalShadows)
		{
			globalShadows = value;
			raisePropertyChanged(prop_GlobalShadows);
		}
	}

	// FUNCTION: PLAYER 0x00706190
	// FUNCTION: STUDIO 0x007d1d30
	std::string Lighting::getTimeStr() const
	{
		return boost::posix_time::to_simple_string(timeOfDay);
	}

	// FUNCTION: PLAYER 0x007067e0
	// FUNCTION: STUDIO 0x007d2590
	void Lighting::setClearColor(G3D::Color4 value)
	{
		if (value != clearColor)
		{
			clearColor = value;
			fireLightingChanged(false);
		}
	}

	// FUNCTION: PLAYER 0x00706860
	// FUNCTION: STUDIO 0x007d2690
	void Lighting::setShadowColor(G3D::Color4 value)
	{
		if (value != shadowColor)
		{
			shadowColor = value;
			raisePropertyChanged(prop_ShadowColor);
			fireLightingChanged(false);
		}
	}

	// Inlined into setFogColor3 0x007057b0; no body of its own survives the link.
	void Lighting::setFogColor(G3D::Color4 value)
	{
		if (value != fogColor)
		{
			fogColor = value;
			raisePropertyChanged(prop_FogColor);
		}
	}

	// FUNCTION: PLAYER 0x007068f0
	// FUNCTION: STUDIO 0x007d2720
	void Lighting::onChildRemoving(Instance* child)
	{
		if (sky.get() == child)
		{
			sky.reset();
			fireLightingChanged(true);
		}
		Super::onChildRemoving(child);
	}

	// FUNCTION: PLAYER 0x00706920
	// FUNCTION: STUDIO 0x007d2750
	void Lighting::onChildChanged(Instance* instance, const PropertyChanged& event)
	{
		Super::onChildChanged(instance, event);
		if (sky.get() == instance)
			fireLightingChanged(true);
	}

	// FUNCTION: PLAYER 0x007069d0
	// FUNCTION: STUDIO 0x007d2780
	void Lighting::setShadowColor3(G3D::Color3 value)
	{
		setShadowColor(G3D::Color4(value));
	}

	// FUNCTION: PLAYER 0x00706a30
	// FUNCTION: STUDIO 0x007d27e0
	void Lighting::setTime(const boost::posix_time::time_duration& value)
	{
		const int secondsPerDay = 24 * 60 * 60;
		int seconds = value.total_seconds();
		seconds -= seconds / secondsPerDay * secondsPerDay;		// wraps to one day
		if (timeOfDay.total_seconds() != seconds)
		{
			timeOfDay = boost::posix_time::seconds(seconds);
			skyParameters->setTime(getGameTime());
			raisePropertyChanged(prop_TimeOfDay);
			fireLightingChanged(false);
		}
	}

	// FUNCTION: PLAYER 0x00706b00
	// FUNCTION: STUDIO 0x007d28b0
	void Lighting::setGeographicLatitude(float value)
	{
		if (value != skyParameters->geoLatitude)
		{
			skyParameters->geoLatitude = value;
			raisePropertyChanged(prop_GeographicLatitude);
			fireLightingChanged(false);
		}
	}

	double Lighting::getMinutesAfterMidnight()
	{
		return timeOfDay.total_milliseconds() / 60000.0;
	}

	// FUNCTION: PLAYER 0x00707440
	// FUNCTION: STUDIO 0x007d31f0
	void Lighting::setMinutesAfterMidnight(double value)
	{
		setTime(boost::posix_time::seconds((int)(value * 60)));
	}

	// FUNCTION: PLAYER 0x00707490
	// FUNCTION: STUDIO 0x007d3240
	void Lighting::setTimeStr(const std::string& value)
	{
		setTime(boost::posix_time::duration_from_string(value));
	}

	// FUNCTION: PLAYER 0x007074c0
	// FUNCTION: STUDIO 0x007d3270
	bool Lighting::askAddChild(const Instance* instance) const
	{
		return Instance::fastDynamicCast<Sky>(instance) != NULL;
	}

	// Byte-identical in a probe compile; PLAYER scores 98.5% because shared_from<Sky> folds into
	// our shared_from<GuiItem> and that call pairs with another original twin.
	// STUB: PLAYER 0x00707510
	// STUB: STUDIO 0x007d32c0
	void Lighting::onChildAdded(Instance* child)
	{
		Super::onChildAdded(child);
		if (Sky* s = Instance::fastDynamicCast<Sky>(child))
		{
			sky = shared_from(s);
			fireLightingChanged(true);
		}
	}

	// FUNCTION: PLAYER 0x00707710
	// FUNCTION: STUDIO 0x007d3500
	void Lighting::replaceSky(Sky* newSky)
	{
		while (Sky* old = findFirstChildOfType<Sky>())
			old->setParent(NULL);
		newSky->setParent(this);
	}

	// FUNCTION: PLAYER 0x00707ba0
	// FUNCTION: STUDIO 0x007d39e0
	Lighting::Lighting()
		: hasSky(true)
		, clearColor(G3D::Color3::white(), 1.0f)
		, shadowColor(0.7f, 0.7f, 0.72f, 1.0f)
		, fogColor(0.75f, 0.75f, 0.75f, 1.0f)
		, fogStart(0.0f)
		, fogEnd(100000.0f)
		, timeOfDay(boost::posix_time::duration_from_string("14:00:00"))
		, topColorShift(0.0f, 0.0f, 0.0f)
		, bottomColorShift(0.0f, 0.0f, 0.0f)
		, globalBrightness(1.0f)
		, globalAmbient(0.5f, 0.5f, 0.5f)
		, globalOutdoorAmbient(0.5f, 0.5f, 0.5f)
		, globalShadows(false)
		, outlines(true)
	{
		skyParameters->lightColor = G3D::Color3(152 / 255.0f, 137 / 255.0f, 102 / 255.0f);
		skyParameters->setTime(getGameTime());
		setName(sLighting);
	}
}

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Lighting);
}
