/* V8DataModel/GuiButton.h -- October 2013.
   Layout from the PLAYER ctor 0x007cb2e0 (GuiButton(name); STUDIO 0x008e7d90), which passes active=true
   to GuiObject: clicked +0x21c, six remote signals +0x220..+0x250 (two void(), four void(int,int)),
   their six count/EventReplicator pairs +0x250..+0x310 (connectionSignal = signal + 4, instances set
   last in declaration order), autoButtonColor +0x310 (true), selected +0x311, modal +0x312,
   style +0x314, verb +0x318, verbToSet +0x31c (""), images +0x338 (scoped_array<GuiDrawImage>; setStyle
   0x007c3ab0 allocates four). Member names and order are 2016's (GuiObject.h, class GuiButton), which
   match the 2013 offsets.
   The 2013 asserts put GuiButton's code in GuiObject.cpp (GuiButton::getChildRect2D's assert is
   GuiObject.cpp line 1428); this tree keeps its reflection block and setters in GuiButton.cpp. */

#pragma once

#include "V8DataModel/GuiObject.h"
#include "Gui/GuiDrawImage.h"

#include <boost/scoped_array.hpp>
#include <string>

namespace RBX {

	extern const char* const sGuiButton;

	class Verb;

	// VTABLE: PLAYER 0x00b677ec RBX::Debugable
	// VTABLE: STUDIO 0x00c35264 RBX::Debugable
	// SIZE 0x33c
	class GuiButton
		: public DescribedNonCreatable<GuiButton, GuiObject, sGuiButton>
	{
		typedef DescribedNonCreatable<GuiButton, GuiObject, sGuiButton> Super;
		bool clicked;  // 0x21c

	public:
		enum Style
		{
			CUSTOM_STYLE = 0,
			ROBLOX_RED_STYLE = 1,
			ROBLOX_GREY_STYLE = 2,
		};

		GuiButton(const char* name);

		rbx::remote_signal<void()> mouseButton1ClickSignal;          // 0x220
		rbx::remote_signal<void()> mouseButton2ClickSignal;          // 0x228
		rbx::remote_signal<void(int, int)> mouseButton1DownSignal;   // 0x230
		rbx::remote_signal<void(int, int)> mouseButton1UpSignal;     // 0x238
		rbx::remote_signal<void(int, int)> mouseButton2DownSignal;   // 0x240
		rbx::remote_signal<void(int, int)> mouseButton2UpSignal;     // 0x248

		DECLARE_EVENT_REPLICATOR_SIG(GuiButton, MouseButton1Click, void())        // 0x250, 0x254
		DECLARE_EVENT_REPLICATOR_SIG(GuiButton, MouseButton2Click, void())        // 0x270, 0x274
		DECLARE_EVENT_REPLICATOR_SIG(GuiButton, MouseButton1Down, void(int, int)) // 0x290, 0x294
		DECLARE_EVENT_REPLICATOR_SIG(GuiButton, MouseButton1Up, void(int, int))   // 0x2b0, 0x2b4
		DECLARE_EVENT_REPLICATOR_SIG(GuiButton, MouseButton2Down, void(int, int)) // 0x2d0, 0x2d4
		DECLARE_EVENT_REPLICATOR_SIG(GuiButton, MouseButton2Up, void(int, int))   // 0x2f0, 0x2f4

		// The out-of-line getters the prop descriptors take (GuiObject.obj, PLAYER 0x007bd060..).
		// FUNCTION: PLAYER 0x007bd060
		// FUNCTION: STUDIO 0x008d9a20
		bool getAutoButtonColor() const { return autoButtonColor; }
		void setAutoButtonColor(bool value);
		// FUNCTION: PLAYER 0x007bd070
		// FUNCTION: STUDIO 0x008d9a30
		bool getSelected() const { return selected; }
		void setSelected(bool value);
		// FUNCTION: PLAYER 0x007bd080
		// FUNCTION: STUDIO 0x008d9a40
		Style getStyle() const { return style; }
		void setStyle(Style value);
		// FUNCTION: PLAYER 0x007bd090
		// FUNCTION: STUDIO 0x008d9a50
		bool getModal() const { return modal; }
		void setModal(bool value);

		static Reflection::PropDescriptor<GuiButton, bool> prop_AutoButtonColor;
		static Reflection::PropDescriptor<GuiButton, bool> prop_Selected;
		static Reflection::PropDescriptor<GuiButton, bool> prop_Modal;
		static Reflection::EnumPropDescriptor<GuiButton, Style> prop_Style;

		/*override*/ void onPropertyChanged(const Reflection::PropertyDescriptor& descriptor);

	protected:
		/*override*/ void setServerGuiObject();

		bool autoButtonColor;                     // 0x310
		bool selected;                            // 0x311
		bool modal;                               // 0x312
		Style style;                              // 0x314
		Verb* verb;                               // 0x318
		std::string verbToSet;                    // 0x31c
		boost::scoped_array<GuiDrawImage> images; // 0x338
	};

} // namespace RBX

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x006be250
// TEMPLATE: STUDIO 0x00852690
// ??1?$EventReplicatorImpl@$0A@VGuiButton@RBX@@$$A6AXXZ@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x006be260
// TEMPLATE: STUDIO 0x008526a0
// ??1?$EventReplicatorImpl@$01VGuiButton@RBX@@$$A6AXHH@Z@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x007bf710
// TEMPLATE: STUDIO 0x008dc100
// ??$getset@P8GuiButton@RBX@@BE_NXZP812@AEX_N@Z@?$PropDescriptor@VGuiButton@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8GuiButton@2@BE_NXZP852@AEX_N@Z@Z
// TEMPLATE: PLAYER 0x007bf770
// TEMPLATE: STUDIO 0x008dc160
// ??$getset@P8GuiButton@RBX@@BE?AW4Style@12@XZP812@AEXW4312@@Z@?$PropDescriptor@VGuiButton@RBX@@W4Style@12@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@W4Style@GuiButton@RBX@@@Reflection@RBX@@@std@@P8GuiButton@2@BE?AW4Style@52@XZP852@AEXW4652@@Z@Z
// clang-format on

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x0070bb40
// ?connectGeneric@?$EventDescImpl@$01VGuiButton@RBX@@$$A6AXHH@ZV?$remote_signal@$$A6AXHH@Z@rbx@@PQ12@V34@@Reflection@RBX@@UBE?AVconnection@signals@rbx@@PAVDescribedBase@23@V?$shared_ptr@VGenericSlotWrapper@Reflection@RBX@@@boost@@@Z
// clang-format on
