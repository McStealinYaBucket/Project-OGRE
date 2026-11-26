/* ClientBase/RenderSettingsItem.cpp -- October 2013 client shell (PLAYER and STUDIO compile it).
   No 2013 assert names this file; the path is the 2016 tree's (ClientBase/RenderSettingsItem.cpp).
   The shell is /GL, so the linker emits these bodies through LTCG: specialised calling conventions,
   one-call-site descriptor ctors inlined into their dynamic initialisers (PLAYER 0x00a5bae0-
   0x00a5c3e7), and no .obj order. The ctor's misses are its unresolved GfxBase/Base callees.

   2013 property list, in dynamic-initialiser order (name, category, getter/setter at PLAYER):
     graphicsMode "General" 0x00456a90; FrameRateManager "General" 0x00456ac0; QualityLevel
     "Performance" 0x00456b20; AlwaysDrawConnectors "General" 0x004558f0/0x00456ba0;
     IsAggregationShown "Debug" 0x00455870/0x00456a00; IsSynchronizedWithPhysics and UsesPaintMessage
     "Performance" (BoundProp<bool>, +0xdc/+0xdd); AASamples "Quality" 0x00456a30; profileName
     "Quality" (STREAMING); Shadow "Debug" 0x00456b70; Antialiasing "Quality" 0x00456af0;
     ShowBoundingBoxes "Debug" 0x00455900/0x00456c50; AutoFRMLevel "Debug" (setter is the virtual
     setAutoQualityLevel through a vcall thunk); EnableFRM "Debug" 0x00455910/0x00456c80;
     DebugDisableInterpolation "Debug" 0x00456cd0/0x00456ce0; Resolution "Screen" 0x00456a60;
     GetMaxQualityLevel (BoundFuncDesc); TextureCacheSize, MeshCacheSize "Cache";
     EagerBulkExecution "Performance" 0x00455920/0x00456cf0. */

#include "RenderSettingsItem.h"

#include "V8DataModel/PartInstance.h"

namespace RBX {
	namespace SystemUtil {
		// Base's rbx/SystemUtil.h declares uint64_t; the 2013 caller compares eax only (PLAYER
		// 0x004568c2), so the 2013 return type is 32 bits. PLAYER 0x009b59b0.
		size_t getVideoMemory();
	}
}

// PartInstance::disableInterpolation (PLAYER 0x00df3b55) is public in 2013 (the shell reads it) but
// private in App's PartInstance.h, so the shell binds to it by its current decorated name until App
// makes it public. The generated code is the same `mov al, [0x00df3b55]`.
extern bool g_partInstanceDisableInterpolation;
#pragma comment(linker, "/alternatename:?g_partInstanceDisableInterpolation@@3_NA=?disableInterpolation@PartInstance@RBX@@0_NA")

const char* const sRenderSettings = "RenderSettings";

using namespace RBX;

RBX_REGISTER_CLASS(CRenderSettingsItem);

RBX_REGISTER_ENUM(CRenderSettings::AASamples);
RBX_REGISTER_ENUM(CRenderSettings::GraphicsMode);
RBX_REGISTER_ENUM(CRenderSettings::FrameRateManagerMode);
RBX_REGISTER_ENUM(CRenderSettings::AntialiasingMode);
RBX_REGISTER_ENUM(CRenderSettings::ShadowMode);
RBX_REGISTER_ENUM(CRenderSettings::QualityLevel);
RBX_REGISTER_ENUM(CRenderSettings::ResolutionPreset);

namespace RBX
{
	namespace Reflection
	{
		// FUNCTION: PLAYER 0x00455950
		// FUNCTION: STUDIO 0x00436840
		template<> EnumDesc<CRenderSettings::AASamples>::EnumDesc()
			: EnumDescriptor("AASamples", typeid(CRenderSettings::AASamples))
		{
			addPair(CRenderSettings::NONE, "None");
			addPair(CRenderSettings::AA4, "4");
			addPair(CRenderSettings::AA8, "8");
		}

		// FUNCTION: PLAYER 0x00455ab0
		// FUNCTION: STUDIO 0x004369a0
		template<> EnumDesc<CRenderSettings::GraphicsMode>::EnumDesc()
			: EnumDescriptor("GraphicsMode", typeid(CRenderSettings::GraphicsMode))
		{
			addPair(CRenderSettings::AutoGraphicsMode, "Automatic");
			addPair(CRenderSettings::Direct3D9, "Direct3D");
			addPair(CRenderSettings::OpenGL, "OpenGL");
			addPair(CRenderSettings::NoGraphics, "NoGraphics");
			addLegacy(2, "OpenGL legacy", CRenderSettings::AutoGraphicsMode);
		}

		// FUNCTION: PLAYER 0x00455c20
		// FUNCTION: STUDIO 0x00436b10
		template<> EnumDesc<CRenderSettings::FrameRateManagerMode>::EnumDesc()
			: EnumDescriptor("FramerateManagerMode", typeid(CRenderSettings::FrameRateManagerMode))
		{
			addPair(CRenderSettings::FrameRateManagerAuto, "Automatic");
			addPair(CRenderSettings::FrameRateManagerOn, "On");
			addPair(CRenderSettings::FrameRateManagerOff, "Off");
		}

		// FUNCTION: PLAYER 0x00455d80
		// FUNCTION: STUDIO 0x00436c70
		template<> EnumDesc<CRenderSettings::AntialiasingMode>::EnumDesc()
			: EnumDescriptor("Antialiasing", typeid(CRenderSettings::AntialiasingMode))
		{
			addPair(CRenderSettings::AntialiasingAuto, "Automatic");
			addPair(CRenderSettings::AntialiasingOff, "Off");
			addPair(CRenderSettings::AntialiasingOn, "On");
		}

		// FUNCTION: PLAYER 0x00455ee0
		// FUNCTION: STUDIO 0x00436dd0
		template<> EnumDesc<CRenderSettings::ShadowMode>::EnumDesc()
			: EnumDescriptor("Shadow", typeid(CRenderSettings::ShadowMode))
		{
			addPair(CRenderSettings::c_shadowMode0, "Automatic");
			addPair(CRenderSettings::c_shadowMode1, "All");
			addPair(CRenderSettings::c_shadowMode3, "CharacterOnly");
			addPair(CRenderSettings::c_shadowMode2, "Off");
		}

		// STUB: PLAYER 0x00456050
		// STUB: STUDIO 0x00436f40
		template<> EnumDesc<CRenderSettings::QualityLevel>::EnumDesc()
			: EnumDescriptor("QualityLevel", typeid(CRenderSettings::QualityLevel))
		{
			addPair(CRenderSettings::QualityAuto, "Automatic");

			for (int i = 1; i < CRenderSettings::QualityLevelMax; i++)
			{
				std::string name = RBX::format("Level%.2d", i);
				addPair((CRenderSettings::QualityLevel)i, name.c_str());
			}
			{
				char name[] = "Level 00";
				char* numberstart = &name[sizeof(name) - 3];
				for (int i = 1; i < CRenderSettings::QualityLevelMax; i++)
				{
					_snprintf(numberstart, 3, "%2u", i);
					addLegacyName(name, (CRenderSettings::QualityLevel)i);
				}
			}
		}

		// STUB: PLAYER 0x00456260
		// STUB: STUDIO 0x00437150
		template<> EnumDesc<CRenderSettings::ResolutionPreset>::EnumDesc()
			: EnumDescriptor("Resolution", typeid(CRenderSettings::ResolutionPreset))
		{
			addPair(CRenderSettings::ResolutionAuto, "Automatic");
			addPair(CRenderSettings::Resolution720x526, "720x526");
			addPair(CRenderSettings::Resolution800x600, "800x600");
			addPair(CRenderSettings::Resolution1024x600, "1024x600");
			addLegacyName("1024x600 (wide)", CRenderSettings::Resolution1024x600);
			addPair(CRenderSettings::Resolution1024x768, "1024x768");
			addPair(CRenderSettings::Resolution1280x720, "1280x720");
			addLegacyName("1280x720 (wide)", CRenderSettings::Resolution1280x720);
			addPair(CRenderSettings::Resolution1280x768, "1280x768");
			addLegacyName("1280x768 (wide)", CRenderSettings::Resolution1280x768);
			addPair(CRenderSettings::Resolution1152x864, "1152x864");
			addPair(CRenderSettings::Resolution1280x800, "1280x800");
			addLegacyName("1280x800 (wide)", CRenderSettings::Resolution1280x800);
			addPair(CRenderSettings::Resolution1360x768, "1360x768");
			addLegacyName("1360x768 (wide)", CRenderSettings::Resolution1360x768);
			addPair(CRenderSettings::Resolution1280x960, "1280x960");
			addPair(CRenderSettings::Resolution1280x1024, "1280x1024");
			addPair(CRenderSettings::Resolution1440x900, "1440x900");
			addLegacyName("1440x900 (wide)", CRenderSettings::Resolution1440x900);
			addPair(CRenderSettings::Resolution1600x900, "1600x900");
			addLegacyName("1600x900 (wide)", CRenderSettings::Resolution1600x900);
			addPair(CRenderSettings::Resolution1600x1024, "1600x1024");
			addLegacyName("1600x1024 (wide)", CRenderSettings::Resolution1600x1024);
			addPair(CRenderSettings::Resolution1600x1200, "1600x1200");
			addPair(CRenderSettings::Resolution1680x1050, "1680x1050");
			addLegacyName("1680x1050 (wide)", CRenderSettings::Resolution1680x1050);
			addPair(CRenderSettings::Resolution1920x1080, "1920x1080");
			addLegacyName("1920x1080 (wide)", CRenderSettings::Resolution1920x1080);
			addPair(CRenderSettings::Resolution1920x1200, "1920x1200");
			addLegacyName("1920x1200 (wide)", CRenderSettings::Resolution1920x1200);
		}
	}
}

REFLECTION_BEGIN();
static Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::GraphicsMode> prop_graphicsMode("graphicsMode", "General", &CRenderSettingsItem::getGraphicsMode, &CRenderSettingsItem::setGraphicsMode);
static Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::FrameRateManagerMode> prop_frameRateManagerMode("FrameRateManager", "General", &CRenderSettingsItem::getFrameRateManagerMode, &CRenderSettingsItem::setFrameRateManagerMode);
static Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::QualityLevel> prop_frmQuality("QualityLevel", "Performance", &CRenderSettingsItem::getQualityLevel, &CRenderSettingsItem::setQualityLevel);
static Reflection::PropDescriptor<CRenderSettingsItem, bool> prop_drawConnectors("AlwaysDrawConnectors", "General", &CRenderSettingsItem::getDrawConnectors, &CRenderSettingsItem::setAlwaysDrawConnectors);
static Reflection::PropDescriptor<CRenderSettingsItem, bool> prop_ShowAggregation("IsAggregationShown", "Debug", &CRenderSettingsItem::getShowAggregation, &CRenderSettingsItem::setShowAggregation);
static Reflection::BoundProp<bool> prop_IsSynchronizedWithPhysics("IsSynchronizedWithPhysics", "Performance", &CRenderSettingsItem::isSynchronizedWithPhysics);
static Reflection::BoundProp<bool> prop_usesPaintMessage("UsesPaintMessage", "Performance", &CRenderSettingsItem::usesPaintMessage);
static Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::AASamples> prop_AASamples("AASamples", "Quality", &CRenderSettingsItem::getAASamples, &CRenderSettingsItem::setAASamples);
Reflection::BoundProp<std::string> CRenderSettingsItem::prop_profileName("profileName", "Quality", &CRenderSettingsItem::profileName, Reflection::PropertyDescriptor::STREAMING);
static Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::ShadowMode> prop_shadowMode("Shadow", "Debug", &CRenderSettingsItem::getShadowMode, &CRenderSettingsItem::setShadowMode);
static Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::AntialiasingMode> prop_antialiasingMode("Antialiasing", "Quality", &CRenderSettingsItem::getAntialiasingMode, &CRenderSettingsItem::setAntialiasingMode);
static Reflection::PropDescriptor<CRenderSettingsItem, bool> prop_debugShowBoundingBoxes("ShowBoundingBoxes", "Debug", &CRenderSettingsItem::getDebugShowBoundingBoxes, &CRenderSettingsItem::setDebugShowBoundingBoxes);
static Reflection::PropDescriptor<CRenderSettingsItem, int> prop_autoQualityLevel("AutoFRMLevel", "Debug", &CRenderSettingsItem::getAutoQualityLevel, &CRenderSettingsItem::setAutoQualityLevel);
static Reflection::PropDescriptor<CRenderSettingsItem, bool> prop_enableFRM("EnableFRM", "Debug", &CRenderSettingsItem::getEnableFRM, &CRenderSettingsItem::setEnableFRM);
static Reflection::PropDescriptor<CRenderSettingsItem, bool> prop_debugDisableInterpolation("DebugDisableInterpolation", "Debug", &CRenderSettingsItem::getDebugDisableInterpolation, &CRenderSettingsItem::setDebugDisableInterpolation);
Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::ResolutionPreset> CRenderSettingsItem::prop_resolution("Resolution", "Screen", &CRenderSettingsItem::getResolutionPreference, &CRenderSettingsItem::setResolutionPreference);
static Reflection::BoundFuncDesc<CRenderSettingsItem, int()> func_getMaxQualityLevel(&CRenderSettingsItem::getMaxQualityLevel, "GetMaxQualityLevel", Security::None);
static Reflection::PropDescriptor<CRenderSettingsItem, int> prop_textureCacheSize("TextureCacheSize", "Cache", &CRenderSettingsItem::getTextureCacheSize, &CRenderSettingsItem::setTextureCacheSize);
static Reflection::PropDescriptor<CRenderSettingsItem, int> prop_meshCacheSize("MeshCacheSize", "Cache", &CRenderSettingsItem::getMeshCacheSize, &CRenderSettingsItem::setMeshCacheSize);
static Reflection::PropDescriptor<CRenderSettingsItem, bool> prop_eagerBulkExecution("EagerBulkExecution", "Performance", &CRenderSettingsItem::getEagerBulkExecution, &CRenderSettingsItem::setEagerBulkExecution);
REFLECTION_END();

// FUNCTION: PLAYER 0x004567d0
// STUB: STUDIO 0x004376e0
CRenderSettingsItem::CRenderSettingsItem()
	: currentDisplaySize(800, 600)
	, isSynchronizedWithPhysics(false)
	, usesPaintMessage(true)
{
	setName("Rendering");
	fullScreenSizes.push_back(currentDisplaySize);
	if (SystemUtil::getVideoMemory() >= 16000000)
		fullscreenSize = G3D::Vector2int16(1024, 768);
	else
		fullscreenSize = G3D::Vector2int16(800, 600);
}

// FUNCTION: PLAYER 0x00456a00
// FUNCTION: STUDIO 0x00437910
void CRenderSettingsItem::setShowAggregation(bool value)
{
	if (value != showAggregation)
	{
		showAggregation = value;
		settingsChangedSignal(&prop_ShowAggregation);
	}
}

// FUNCTION: PLAYER 0x00456a30
// FUNCTION: STUDIO 0x004379d0
void CRenderSettingsItem::setAASamples(AASamples value)
{
	if (value != aaSamples)
	{
		aaSamples = value;
		settingsChangedSignal(&prop_AASamples);
	}
}

// FUNCTION: PLAYER 0x00456a60
// FUNCTION: STUDIO 0x00437a00
void CRenderSettingsItem::setResolutionPreference(ResolutionPreset value)
{
	if (value != resolutionPreference)
	{
		resolutionPreference = value;
		settingsChangedSignal(&prop_resolution);
	}
}

// FUNCTION: PLAYER 0x00456a90
// FUNCTION: STUDIO 0x00437a30
void CRenderSettingsItem::setGraphicsMode(GraphicsMode value)
{
	if (value != graphicsMode)
	{
		graphicsMode = value;
		settingsChangedSignal(&prop_graphicsMode);
	}
}

// FUNCTION: PLAYER 0x00456ac0
// FUNCTION: STUDIO 0x00437a60
void CRenderSettingsItem::setFrameRateManagerMode(FrameRateManagerMode value)
{
	if (value != frameRateManagerMode)
	{
		frameRateManagerMode = value;
		settingsChangedSignal(&prop_frameRateManagerMode);
	}
}

// FUNCTION: PLAYER 0x00456af0
// FUNCTION: STUDIO 0x00437a90
void CRenderSettingsItem::setAntialiasingMode(AntialiasingMode value)
{
	if (value != antialiasingMode)
	{
		antialiasingMode = value;
		settingsChangedSignal(&prop_antialiasingMode);
	}
}

// FUNCTION: PLAYER 0x00456b20
// FUNCTION: STUDIO 0x00437ac0
void CRenderSettingsItem::setQualityLevel(QualityLevel value)
{
	if (value != qualityLevel)
	{
		qualityLevel = value;
		settingsChangedSignal(&prop_frmQuality);
	}
}

// FUNCTION: PLAYER 0x00456b50
// FUNCTION: STUDIO 0x00437af0
void CRenderSettingsItem::setAutoQualityLevel(int level)
{
	if (level != autoQualityLevel)
	{
		autoQualityLevel = level;
		settingsChangedSignal(&prop_frmQuality);
	}
}

// FUNCTION: PLAYER 0x00456b70
// FUNCTION: STUDIO 0x00437b10
void CRenderSettingsItem::setShadowMode(ShadowMode value)
{
	if (value != shadowMode)
	{
		shadowMode = value;
		settingsChangedSignal(&prop_shadowMode);
	}
}

// FUNCTION: PLAYER 0x00456ba0
// FUNCTION: STUDIO 0x00437b40
void CRenderSettingsItem::setAlwaysDrawConnectors(bool value)
{
	bool oldValue = getAlwaysDrawConnectors();
	drawConnectors = value;
	if (oldValue != getAlwaysDrawConnectors())
		settingsChangedSignal(&prop_drawConnectors);
}

// FUNCTION: PLAYER 0x00456c50
// FUNCTION: STUDIO 0x00437ba0
void CRenderSettingsItem::setDebugShowBoundingBoxes(bool value)
{
	if (value != debugShowBoundingBoxes)
	{
		debugShowBoundingBoxes = value;
		settingsChangedSignal(&prop_debugShowBoundingBoxes);
	}
}

// FUNCTION: PLAYER 0x00456c80
// FUNCTION: STUDIO 0x00437bd0
void CRenderSettingsItem::setEnableFRM(bool value)
{
	if (value != enableFRM)
	{
		enableFRM = value;
		settingsChangedSignal(&prop_enableFRM);
	}
}

// FUNCTION: PLAYER 0x00456cb0
// FUNCTION: STUDIO 0x00437c00
void CRenderSettingsItem::setTextureCacheSize(unsigned int size)
{
	textureCacheSize = size;
}

// FUNCTION: PLAYER 0x00456cc0
// FUNCTION: STUDIO 0x00437c10
void CRenderSettingsItem::setMeshCacheSize(unsigned int size)
{
	meshCacheSize = size;
}

// FUNCTION: PLAYER 0x00456cd0
// FUNCTION: STUDIO 0x00437c20
bool CRenderSettingsItem::getDebugDisableInterpolation() const
{
	return g_partInstanceDisableInterpolation;
}

// FUNCTION: PLAYER 0x00456ce0
// FUNCTION: STUDIO 0x00437c30
void CRenderSettingsItem::setDebugDisableInterpolation(bool value)
{
	g_partInstanceDisableInterpolation = value;
}

// FUNCTION: PLAYER 0x00456cf0
// FUNCTION: STUDIO 0x00437c40
void CRenderSettingsItem::setEagerBulkExecution(bool value)
{
	if (value != eagerBulkExecution)
	{
		eagerBulkExecution = value;
		settingsChangedSignal(&prop_eagerBulkExecution);
	}
}
