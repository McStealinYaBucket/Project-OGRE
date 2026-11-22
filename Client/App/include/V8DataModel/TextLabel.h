/* V8DataModel/TextLabel.h -- October 2013.
   Layout from the PLAYER ctor 0x006c1c10 (STUDIO 0x00854b20): the Described ctor ("TextLabel"), then the
   GuiTextMixin base at +0x21c ("Label", BrickColor(26).color3()), then the TextLabel vftables and
   guiQueue = GUIQUEUE_TEXT. Creatable<Instance>::create<TextLabel> 0x00421af0 allocates 0x270. */

#pragma once

#include "V8DataModel/GuiLabel.h"
#include "V8DataModel/GuiText.h"

namespace RBX {

	extern const char* const sTextLabel;

	// VTABLE: PLAYER 0x00b3b4bc RBX::Debugable
	// VTABLE: STUDIO 0x00c1d1c4 RBX::Debugable
	// VTABLE: PLAYER 0x00b3b490 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c1d198 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b3b45c RBX::IAdornable
	// VTABLE: STUDIO 0x00c1d164 RBX::IAdornable
	// SIZE 0x270
	class TextLabel
		: public DescribedCreatable<TextLabel, GuiLabel, sTextLabel>
		, public GuiTextMixin  // 0x21c
	{
		typedef DescribedCreatable<TextLabel, GuiLabel, sTextLabel> Super;
	public:
		TextLabel();

		/*override*/ void checkForResize();
		/*override*/ void setTransparencyLegacy(float value);

		void setText(std::string value);
		void setFontSize(TextService::FontSize value);
		void setFont(TextService::Font value);
		void setTextColor(BrickColor value);
		void setTextColor3(G3D::Color3 value);
		void setTextTransparency(float value);
		void setTextStrokeTransparency(float value);
		void setTextStrokeColor3(G3D::Color3 value);
		void setTextWrap(bool value);
		void setTextScale(bool value);
		void setXAlignment(TextService::XAlignment value);
		void setYAlignment(TextService::YAlignment value);
		G3D::Vector2 getTextBounds() const;
		bool getTextFits() const;

		static Reflection::PropDescriptor<TextLabel, std::string> prop_Text;
		static Reflection::EnumPropDescriptor<TextLabel, TextService::FontSize> prop_FontSize;
		static Reflection::EnumPropDescriptor<TextLabel, TextService::Font> prop_Font;
		static Reflection::PropDescriptor<TextLabel, BrickColor> prop_TextColor;
		static Reflection::PropDescriptor<TextLabel, G3D::Color3> prop_TextColor3;
		static Reflection::PropDescriptor<TextLabel, float> prop_TextTransparency;
		static Reflection::PropDescriptor<TextLabel, bool> prop_TextWrap;
		static Reflection::PropDescriptor<TextLabel, bool> prop_depTextWrap;
		static Reflection::PropDescriptor<TextLabel, bool> prop_TextScale;
		static Reflection::EnumPropDescriptor<TextLabel, TextService::XAlignment> prop_TextXAlignment;
		static Reflection::EnumPropDescriptor<TextLabel, TextService::YAlignment> prop_TextYAlignment;
		static Reflection::PropDescriptor<TextLabel, G3D::Vector2> prop_TextBounds;
		static Reflection::PropDescriptor<TextLabel, bool> prop_TextFits;
		static Reflection::PropDescriptor<TextLabel, G3D::Color3> prop_TextStrokeColor3;
		static Reflection::PropDescriptor<TextLabel, float> prop_TextStrokeTransparency;
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls).
	// clang-format off
	// TEMPLATE: PLAYER 0x006c1b60
	// TEMPLATE: STUDIO 0x00854aa0
	// ??$?0PBD@?$Described@VTextLabel@RBX@@$E?sTextLabel@2@3QBDBV?$FactoryProduct@VTextLabel@RBX@@VGuiLabel@2@$E?sTextLabel@2@3QBDBVInstance@2@@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// clang-format on


	// Out-of-line template and implicit-member copies the original emitted for this class, paired by
	// identical masked bytes (tools/unpaired_twins.py on a 2026-11-13 build).
	// clang-format off
	// TEMPLATE: PLAYER 0x006bf0e0
	// TEMPLATE: STUDIO 0x008532c0
	// ?getValue@?$GetSetImpl@P8GuiTextMixin@RBX@@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZP8TextLabel@2@AEXV34@@Z@?$PropDescriptor@VTextLabel@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@UBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x006bf1b0
	// TEMPLATE: STUDIO 0x00853390
	// ?getValue@?$GetSetImpl@P8GuiTextMixin@RBX@@BE?AVBrickColor@2@XZP8TextLabel@2@AEXV32@@Z@?$PropDescriptor@VTextLabel@RBX@@VBrickColor@2@@Reflection@RBX@@UBE?AVBrickColor@4@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x006bf200
	// TEMPLATE: STUDIO 0x008533e0
	// ?getValue@?$GetSetImpl@P8GuiTextMixin@RBX@@BE?AVColor3@G3D@@XZP8TextLabel@2@AEXV34@@Z@?$PropDescriptor@VTextLabel@RBX@@VColor3@G3D@@@Reflection@RBX@@UBE?AVColor3@G3D@@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x006bf290
	// TEMPLATE: STUDIO 0x00853460
	// ?getValue@?$GetSetImpl@P8GuiTextMixin@RBX@@BE?AW4YAlignment@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4YAlignment@TextService@2@@Reflection@RBX@@UBE?AW4YAlignment@TextService@4@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x006bf2c0
	// TEMPLATE: STUDIO 0x008386d0
	// ?getValue@?$GetImpl@P8TextLabel@RBX@@BE?AVVector2@G3D@@XZ@?$PropDescriptor@VTextLabel@RBX@@VVector2@G3D@@@Reflection@RBX@@UBE?AVVector2@G3D@@PBVDescribedBase@34@@Z
	// TEMPLATE: PLAYER 0x006bf3f0
	// ??$getset@P8GuiTextMixin@RBX@@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZP8TextLabel@2@AEXV34@@Z@?$PropDescriptor@VTextLabel@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@4@XZP8TextLabel@2@AEXV64@@Z@Z
	// TEMPLATE: PLAYER 0x006bf440
	// ??$getset@P8GuiTextMixin@RBX@@BE?AW4FontSize@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4FontSize@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4FontSize@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4FontSize@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
	// TEMPLATE: PLAYER 0x006bf490
	// ??$getset@P8GuiTextMixin@RBX@@BE?AW4Font@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4Font@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4Font@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4Font@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
	// TEMPLATE: PLAYER 0x006bf4e0
	// ??$getset@P8GuiTextMixin@RBX@@BE?AVBrickColor@2@XZP8TextLabel@2@AEXV32@@Z@?$PropDescriptor@VTextLabel@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AVBrickColor@2@XZP8TextLabel@2@AEXV62@@Z@Z
	// TEMPLATE: PLAYER 0x006bf530
	// ??$getset@P8GuiTextMixin@RBX@@BE?AVColor3@G3D@@XZP8TextLabel@2@AEXV34@@Z@?$PropDescriptor@VTextLabel@RBX@@VColor3@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VColor3@G3D@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AVColor3@G3D@@XZP8TextLabel@2@AEXV67@@Z@Z
	// TEMPLATE: PLAYER 0x006bf580
	// ??$getset@P8GuiTextMixin@RBX@@BEMXZP8TextLabel@2@AEXM@Z@?$PropDescriptor@VTextLabel@RBX@@M@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@M@Reflection@RBX@@@std@@P8GuiTextMixin@2@BEMXZP8TextLabel@2@AEXM@Z@Z
	// TEMPLATE: PLAYER 0x006bf5d0
	// ??$getset@P8GuiTextMixin@RBX@@BE_NXZP8TextLabel@2@AEX_N@Z@?$PropDescriptor@VTextLabel@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE_NXZP8TextLabel@2@AEX_N@Z@Z
	// TEMPLATE: PLAYER 0x006bf620
	// ??$getset@P8GuiTextMixin@RBX@@BE?AW4XAlignment@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4XAlignment@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4XAlignment@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4XAlignment@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
	// TEMPLATE: PLAYER 0x006bf670
	// ??$getset@P8GuiTextMixin@RBX@@BE?AW4YAlignment@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4YAlignment@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4YAlignment@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4YAlignment@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
	// TEMPLATE: PLAYER 0x006bf6c0
	// ??$getset@P8TextLabel@RBX@@BE?AVVector2@G3D@@XZH@?$PropDescriptor@VTextLabel@RBX@@VVector2@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2@G3D@@@Reflection@RBX@@@std@@P8TextLabel@2@BE?AVVector2@G3D@@XZH@Z
	// TEMPLATE: PLAYER 0x006bf710
	// ??$getset@P8TextLabel@RBX@@BE_NXZH@?$PropDescriptor@VTextLabel@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8TextLabel@2@BE_NXZH@Z
	// TEMPLATE: PLAYER 0x006c0660
	// ?initSingleton@?$Singleton@$$CBV?$EnumDesc@W4FontSize@TextService@RBX@@@Reflection@RBX@@@Reflection@RBX@@CAXXZ
	// TEMPLATE: PLAYER 0x006c06d0
	// ?initSingleton@?$Singleton@$$CBV?$EnumDesc@W4Font@TextService@RBX@@@Reflection@RBX@@@Reflection@RBX@@CAXXZ
	// TEMPLATE: PLAYER 0x006c0740
	// ?initSingleton@?$Singleton@$$CBV?$EnumDesc@W4XAlignment@TextService@RBX@@@Reflection@RBX@@@Reflection@RBX@@CAXXZ
	// TEMPLATE: PLAYER 0x006c0750
	// ?doGetSingleton@?$Singleton@$$CBV?$EnumDesc@W4YAlignment@TextService@RBX@@@Reflection@RBX@@@Reflection@RBX@@CAABV?$EnumDesc@W4YAlignment@TextService@RBX@@@23@XZ
	// TEMPLATE: PLAYER 0x006c1b10
	// ??_G?$FactoryProduct@VTextLabel@RBX@@VGuiLabel@2@$E?sTextLabel@2@3QBDBVInstance@2@@RBX@@MAEPAXI@Z
	// TEMPLATE: PLAYER 0x006c1be0
	// ??1?$DescribedCreatable@VTextLabel@RBX@@VGuiLabel@2@$E?sTextLabel@2@3QBDB$0BL@$0A@@RBX@@UAE@XZ
	// SYNTHETIC: PLAYER 0x006c1d20
	// SYNTHETIC: STUDIO 0x00854c30
	// RBX::TextLabel::~TextLabel
	// clang-format on

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: STUDIO 0x00853490
// ??$getset@P8GuiTextMixin@RBX@@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZP8TextLabel@2@AEXV34@@Z@?$PropDescriptor@VTextLabel@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@4@XZP8TextLabel@2@AEXV64@@Z@Z
// TEMPLATE: STUDIO 0x008534e0
// ??$getset@P8GuiTextMixin@RBX@@BE?AW4FontSize@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4FontSize@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4FontSize@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4FontSize@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
// TEMPLATE: STUDIO 0x00853530
// ??$getset@P8GuiTextMixin@RBX@@BE?AW4Font@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4Font@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4Font@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4Font@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
// TEMPLATE: STUDIO 0x00853580
// ??$getset@P8GuiTextMixin@RBX@@BE?AVBrickColor@2@XZP8TextLabel@2@AEXV32@@Z@?$PropDescriptor@VTextLabel@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AVBrickColor@2@XZP8TextLabel@2@AEXV62@@Z@Z
// TEMPLATE: STUDIO 0x008535d0
// ??$getset@P8GuiTextMixin@RBX@@BE?AVColor3@G3D@@XZP8TextLabel@2@AEXV34@@Z@?$PropDescriptor@VTextLabel@RBX@@VColor3@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VColor3@G3D@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AVColor3@G3D@@XZP8TextLabel@2@AEXV67@@Z@Z
// TEMPLATE: STUDIO 0x00853620
// ??$getset@P8GuiTextMixin@RBX@@BEMXZP8TextLabel@2@AEXM@Z@?$PropDescriptor@VTextLabel@RBX@@M@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@M@Reflection@RBX@@@std@@P8GuiTextMixin@2@BEMXZP8TextLabel@2@AEXM@Z@Z
// TEMPLATE: STUDIO 0x00853670
// ??$getset@P8GuiTextMixin@RBX@@BE_NXZP8TextLabel@2@AEX_N@Z@?$PropDescriptor@VTextLabel@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE_NXZP8TextLabel@2@AEX_N@Z@Z
// TEMPLATE: STUDIO 0x008536c0
// ??$getset@P8GuiTextMixin@RBX@@BE?AW4XAlignment@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4XAlignment@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4XAlignment@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4XAlignment@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
// TEMPLATE: STUDIO 0x00853710
// ??$getset@P8GuiTextMixin@RBX@@BE?AW4YAlignment@TextService@2@XZP8TextLabel@2@AEXW4342@@Z@?$PropDescriptor@VTextLabel@RBX@@W4YAlignment@TextService@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4YAlignment@TextService@RBX@@@Reflection@RBX@@@std@@P8GuiTextMixin@2@BE?AW4YAlignment@TextService@2@XZP8TextLabel@2@AEXW4672@@Z@Z
// TEMPLATE: STUDIO 0x00853760
// ??$getset@P8TextLabel@RBX@@BE?AVVector2@G3D@@XZH@?$PropDescriptor@VTextLabel@RBX@@VVector2@G3D@@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VVector2@G3D@@@Reflection@RBX@@@std@@P8TextLabel@2@BE?AVVector2@G3D@@XZH@Z
// clang-format on
// SYNTHETIC: PLAYER 0x006c1da0
// SYNTHETIC: STUDIO 0x00854cb0
// RBX::TextLabel::`scalar deleting destructor'
