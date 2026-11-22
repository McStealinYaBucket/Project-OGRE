/* ClientBase/RenderSettingsItem.h -- October 2013 client shell (PLAYER and STUDIO compile it).
   CRenderSettingsItem is the "RenderSettings" GlobalAdvancedSettings item, the shell's own
   Instance subclass. Layout from the 2013 bytes (PLAYER, /GL code):
     ctor CRenderSettingsItem() 0x004567d0, ~CRenderSettingsItem 0x00456940 (deleting 0x00456910),
     GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>() 0x00456ee0,
     FactoryProduct<...>() 0x0045a660, Creator::create 0x004073d0 (operator new 0xe4 at 0x0040d51c).
   Its vftables (0x00be4034, 5 groups) equal Instance's except slot 0x3c, which holds the shell copy
   of GlobalAdvancedSettings::Item::askAddChild 0x00455840.

   Stand-ins that belong to other projects (no 2013 header exists in this repository yet):
   - RBX::CRenderSettings is GfxBase's class (Client/Rendering/GfxBase/include/GfxBase/RenderSettings.h
     is empty). Defined below from GfxBase's ctor 0x009c5ce0 and the shell's inline getters, which
     only the shell's copies show (0x00455870-0x00455940). Move it to GfxBase when that header lands.
   - RBX::GlobalAdvancedSettings::Item and GlobalAdvancedSettingsItem<> are App's
     (V8DataModel/GlobalAdvancedSettings.h is a stub without them). Declared below in the 2013 shape:
     2013 has no RTTI for NonFactoryProduct<Instance, sSettingsItem>, so Item derives Instance
     directly; GlobalAdvancedSettingsItem's ctor stores Service::isPublic (+0x60). Only Item is
     declared inside GlobalAdvancedSettings, and nothing here emits code for GlobalAdvancedSettings
     itself, so App's own definition (never included here) stays the one that is linked. */

#pragma once

#include <vector>

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Reflection/Reflection.h"
#include "rbx/signal.h"
#include "G3D/Vector2int16.h"

namespace RBX {

	// App stand-in, see the header comment.
	class GlobalAdvancedSettings
	{
	public:
		class Item : public Instance
		{
		protected:
			// vtable+0x3c. Shell copy: PLAYER 0x00455840.
			/*override*/ bool askAddChild(const Instance* instance) const
			{
				return dynamic_cast<const Item*>(instance) != NULL;
			}
		};
	};

	// App stand-in, see the header comment. Only the parts the shell's CRenderSettingsItem uses.
	template<class Class, const char* const& sClassName>
	class GlobalAdvancedSettingsItem
		: public DescribedCreatable<Class, GlobalAdvancedSettings::Item, sClassName>
		, public Service
	{
		typedef DescribedCreatable<Class, GlobalAdvancedSettings::Item, sClassName> Super;
		static GlobalAdvancedSettingsItem* sing;
	protected:
		GlobalAdvancedSettingsItem()
		{
			Super::setName(sClassName);
			if (sing)
				throw RBX::runtime_error("singleton %s already exists", sClassName);
			sing = this;
		}
		~GlobalAdvancedSettingsItem()
		{
			sing = NULL;
		}
	};

	template<class Class, const char* const& sClassName>
	GlobalAdvancedSettingsItem<Class, sClassName>* GlobalAdvancedSettingsItem<Class, sClassName>::sing = NULL;

	// GfxBase stand-in, see the header comment.
	// SIZE 0x48
	class CRenderSettings
	{
	public:
		enum AASamples
		{
			NONE = 1,
			AA4 = 4,
			AA8 = 8,
		};

		// The 2013 registration (EnumDesc ctor 0x00455ab0): 1 Automatic, 3 Direct3D, 4 OpenGL,
		// 5 NoGraphics, legacy int 2 "OpenGL legacy" -> Automatic.
		typedef enum
		{
			UnknownGraphicsMode = 0,
			AutoGraphicsMode = 1,
			Direct3D9 = 3,
			OpenGL,
			NoGraphics
		} GraphicsMode;

		typedef enum
		{
			AntialiasingAuto = 0,
			AntialiasingOn = 1,
			AntialiasingOff = 2
		} AntialiasingMode;

		// 2013 only (no 2016 counterpart): EnumDesc ctor 0x00455ee0 registers "Automatic", "All" 1,
		// "CharacterOnly" 3, "Off" 2. Enumerator names are placeholders.
		typedef enum
		{
			c_shadowMode0 = 0,
			c_shadowMode1 = 1,
			c_shadowMode2 = 2,
			c_shadowMode3 = 3
		} ShadowMode;

		typedef enum
		{
			FrameRateManagerAuto = 0,
			FrameRateManagerOn = 1,
			FrameRateManagerOff = 2
		} FrameRateManagerMode;

		typedef enum
		{
			QualityAuto = 0,
			QualityLevel1,
			QualityLevel2,
			QualityLevel3,
			QualityLevel4,
			QualityLevel5,
			QualityLevel6,
			QualityLevel7,
			QualityLevel8,
			QualityLevel9,
			QualityLevel10,
			QualityLevel11,
			QualityLevel12,
			QualityLevel13,
			QualityLevel14,
			QualityLevel15,
			QualityLevel16,
			QualityLevel17,
			QualityLevel18,
			QualityLevel19,
			QualityLevel20,
			QualityLevel21,
			QualityLevelMax
		} QualityLevel;

		typedef enum
		{
			ResolutionAuto,
			Resolution720x526,
			Resolution800x600,
			Resolution1024x600,
			Resolution1024x768,
			Resolution1280x720,
			Resolution1280x768,
			Resolution1152x864,
			Resolution1280x800,
			Resolution1360x768,
			Resolution1280x960,
			Resolution1280x1024,
			Resolution1440x900,
			Resolution1600x900,
			Resolution1600x1024,
			Resolution1600x1200,
			Resolution1680x1050,
			Resolution1920x1080,
			Resolution1920x1200,
			ResolutionMaxIndex
		} ResolutionPreset;

	protected:
		GraphicsMode graphicsMode;                  // 0x04 (1)
		AntialiasingMode antialiasingMode;          // 0x08 (2)
		ShadowMode shadowMode;                      // 0x0c
		FrameRateManagerMode frameRateManagerMode;  // 0x10
		QualityLevel qualityLevel;                  // 0x14
		ResolutionPreset resolutionPreference;      // 0x18
		int autoQualityLevel;                       // 0x1c (1)
		int maxQualityLevel;                        // 0x20 (22)
		int minCullDistance;                        // 0x24 (50)
		bool debugShowBoundingBoxes;                // 0x28
		bool enableFRM;                             // 0x29
		G3D::Vector2int16 m_unk0x2a;                // 0x2a (0,0)
		G3D::Vector2int16 m_unk0x2e;                // 0x2e (0,0)
		G3D::Vector2int16 windowSize;               // 0x32 (800,600); the item ctor stores it
		G3D::Vector2int16 m_unk0x36;                // 0x36 (800,600)
		bool showAggregation;                       // 0x3a
		bool drawConnectors;                        // 0x3b
		bool m_unk0x3c;                             // 0x3c (true); ORed with drawConnectors
		bool eagerBulkExecution;                    // 0x3d
		unsigned int textureCacheSize;              // 0x40 (0x2000000)
		unsigned int meshCacheSize;                 // 0x44 (0x2000000)

		static AASamples aaSamples;                 // PLAYER 0x00db4d34

	public:
		CRenderSettings();  // PLAYER 0x009c5ce0 (GfxBase)

		// FRM reports its latest setting. vtable+0x00, PLAYER 0x009c5ed0 (GfxBase).
		virtual void setAutoQualityLevel(int level);

		// Inline getters: the shell's copies are PLAYER 0x00455870-0x00455940.
		bool getShowAggregation() const { return showAggregation; }
		AASamples getAASamples() const { return aaSamples; }
		GraphicsMode getGraphicsMode() const { return graphicsMode; }
		FrameRateManagerMode getFrameRateManagerMode() const { return frameRateManagerMode; }
		AntialiasingMode getAntialiasingMode() const { return antialiasingMode; }
		ShadowMode getShadowMode() const { return shadowMode; }
		QualityLevel getQualityLevel() const { return qualityLevel; }
		ResolutionPreset getResolutionPreference() const { return resolutionPreference; }
		int getMaxQualityLevel() { return maxQualityLevel; }
		int getAutoQualityLevel() const { return autoQualityLevel; }
		bool getDrawConnectors() const { return drawConnectors; }  // 2016 name; the shell copy 0x004558f0
		// ios: name (out of line there); ORing m_unk0x3c is inferred from the shell's setter 0x00456ba0.
		bool getAlwaysDrawConnectors() const { return drawConnectors || m_unk0x3c; }
		bool getDebugShowBoundingBoxes() const { return debugShowBoundingBoxes; }
		bool getEnableFRM() const { return enableFRM; }
		bool getEagerBulkExecution() const { return eagerBulkExecution; }
		unsigned int getTextureCacheSize() const { return textureCacheSize; }
		unsigned int getMeshCacheSize() const { return meshCacheSize; }
	};

} // namespace RBX

extern const char* const sRenderSettings;

// VTABLE: PLAYER 0x00be4034
// SIZE 0xe4
class CRenderSettingsItem
	: public RBX::GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>  // 0x00 (0x64)
	, public RBX::CRenderSettings                                                    // 0x64
{
	std::string profileName;                      // 0xac
	G3D::Vector2int16 currentDisplaySize;         // 0xc8 (800,600)
	std::vector<G3D::Vector2int16> fullScreenSizes;  // 0xcc
public:
	static RBX::Reflection::BoundProp<std::string> prop_profileName;
	static RBX::Reflection::EnumPropDescriptor<CRenderSettingsItem, CRenderSettingsItem::ResolutionPreset> prop_resolution;

	CRenderSettingsItem();

	void setShowAggregation(bool value);
	void setAASamples(AASamples value);
	void setGraphicsMode(GraphicsMode value);
	void setFrameRateManagerMode(FrameRateManagerMode value);
	void setAntialiasingMode(AntialiasingMode value);
	void setShadowMode(ShadowMode value);
	void setQualityLevel(QualityLevel value);
	void setResolutionPreference(ResolutionPreset value);
	void setAlwaysDrawConnectors(bool value);
	void setDebugShowBoundingBoxes(bool value);
	void setEnableFRM(bool value);
	void setEagerBulkExecution(bool value);
	void setTextureCacheSize(unsigned int size);
	void setMeshCacheSize(unsigned int size);
	bool getDebugDisableInterpolation() const;
	void setDebugDisableInterpolation(bool value);

	// CRenderSettings vtable+0x00 (the item's {for CRenderSettings} vftable 0x00be40cc).
	/*override*/ void setAutoQualityLevel(int level);

	bool isSynchronizedWithPhysics;               // 0xdc (false)
	bool usesPaintMessage;                        // 0xdd (true)

	// special signal used only in native code
	rbx::signal<void(const RBX::Reflection::PropertyDescriptor*)> settingsChangedSignal;  // 0xe0
};
