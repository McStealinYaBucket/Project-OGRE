/* V8DataModel/GuiService.h -- October 2013.
   Layout from PLAYER GuiService() 0x0069ed90 and ~GuiService 0x0069ef30 (operator new(0xc0) in
   Creatable<Instance>::create<GuiService> 0x00635ef0). Member names from iOS 10-29
   (getShowLegacyPlayerList reads +0x70, getUseLuaChat +0x71, the ctor sets +0x71 in its body) and
   the 2016 header, whose members match the 2013 offsets one for one; 2016's errorMessage,
   uiMessage, selectionMap, gamepad flags and notificationCallback did not exist yet.
   No virtual overrides: the Debugable vftable 0x00b3822c repeats NonFactoryProduct's.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include <list>
#include <map>
#include <set>

namespace RBX {

	extern const char* const sGuiService;

	class GuiObject;

	// VTABLE: PLAYER 0x00b3822c RBX::Debugable
	// VTABLE: STUDIO 0x00c1e34c RBX::Debugable
	// SIZE 0xc0
	class GuiService
		: public DescribedNonCreatable<GuiService, Instance, sGuiService, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		enum CenterDialogType
		{
			CENTER_DIALOG_UNSOLICITED_DIALOG = 1,
			CENTER_DIALOG_PLAYER_INITIATED_DIALOG = 2,
			CENTER_DIALOG_MODAL_DIALOG = 3,
			CENTER_DIALOG_QUIT_DIALOG = 4,
		};

		enum SpecialKey
		{
			KEY_INSERT = 0,
			KEY_HOME = 1,
			KEY_END = 2,
			KEY_PAGEUP = 3,
			KEY_PAGEDOWN = 4,
			KEY_CHATHOTKEY = 5,
		};

	private:
		typedef DescribedNonCreatable<GuiService, Instance, sGuiService, Reflection::ClassDescriptor::INTERNAL_LOCAL> Super;

		G3D::Rect2D guiInset;			// 0x64  2016: Vector4. Only Rect2D (two Vector2(0,0)) gives the ctor's x,y,x,y store order
		bool showLegacyPlayerList;		// 0x74
		bool useLuaChat;				// 0x75

	public:
		GuiService();
		bool getModalDialogStatus() const;
		void addKey(std::string key);
		void removeKey(std::string key);
		bool dispatchKey(SpecialKey special);
		void openBrowserWindow(std::string url);

		rbx::signal<void(std::string)> openUrlWindow;							// 0x78
		rbx::signal<void()> urlWindowClosed;									// 0x7c
		rbx::signal<void(std::string, std::string)> keyPressed;					// 0x80
		rbx::signal<void()> escapeKeyPressed;									// 0x84
		rbx::signal<void(GuiService::SpecialKey, std::string)> specialKeyPressed;	// 0x88

	protected:
		struct DialogWrapper
		{
			weak_ptr<GuiObject> dialog;
			CenterDialogType dialogType;
			boost::function<void()> showFunction;
			boost::function<void()> hideFunction;
		};

		DialogWrapper* currentDialog;											// 0x8c
		std::map<CenterDialogType, std::list<DialogWrapper*> > dialogQueue;		// 0x90
		std::map<weak_ptr<GuiObject>, DialogWrapper*> dialogWrapperMap;			// 0x9c
		std::set<char> subscribedChars;											// 0xa8
		std::set<SpecialKey> subscribedSpecials;								// 0xb4

		bool hasSpecial(SpecialKey key) { return subscribedSpecials.find(key) != subscribedSpecials.end(); }
		bool shouldPreemptCurrentDialog(DialogWrapper* newDialog) const;
		bool showWaitingDialog(CenterDialogType type);

		// SYNTHETIC: PLAYER 0x0069ef30
		// SYNTHETIC: STUDIO 0x00860c80
		// RBX::GuiService::~GuiService

		// SYNTHETIC: PLAYER 0x0069f090
		// SYNTHETIC: STUDIO 0x00860de0
		// RBX::GuiService::`scalar deleting destructor'
	};

} // namespace RBX

// Template bodies GuiService.obj emits, named by our PUBLICS symbol (the 2013 copies have no oracle name;
// paired through the calls in addKey 0x00696980, removeKey 0x00695ca0, dispatchKey 0x0069a300 and ~GuiService).

// TEMPLATE: PLAYER 0x00488c20
// TEMPLATE: STUDIO 0x0051abc0
// ??1?$signal@$$A6AXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x00694a00
// TEMPLATE: STUDIO 0x00857cd0
// ?_Buynode@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@IAEPAU_Node@?$_Tree_nod@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@2@XZ

// TEMPLATE: PLAYER 0x00694e50
// TEMPLATE: STUDIO 0x00858310
// ??0?$signal@$$A6AXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x00694eb0
// TEMPLATE: STUDIO 0x00858370
// ??0?$signal@$$A6AXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x00694f10
// TEMPLATE: STUDIO 0x008583d0
// ?erase@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@@Z

// TEMPLATE: PLAYER 0x006951e0
// TEMPLATE: STUDIO 0x008586a0
// ?find@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@QAE?AViterator@12@ABD@Z

// TEMPLATE: PLAYER 0x00695800
// TEMPLATE: STUDIO 0x00858be0
// ?erase@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@0@Z

// TEMPLATE: PLAYER 0x00696080
// TEMPLATE: STUDIO 0x008591a0
// ?insert@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@QAE?AU?$pair@Viterator@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@_N@2@ABD@Z

// TEMPLATE: PLAYER 0x00696c90
// TEMPLATE: STUDIO 0x009d9780
// ?erase@?$_Tree@V?$_Tmap_traits@V?$weak_ptr@VGuiObject@RBX@@@boost@@PAUDialogWrapper@GuiService@RBX@@U?$less@V?$weak_ptr@VGuiObject@RBX@@@boost@@@std@@V?$allocator@U?$pair@$$CBV?$weak_ptr@VGuiObject@RBX@@@boost@@PAUDialogWrapper@GuiService@RBX@@@std@@@7@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@0@Z

// TEMPLATE: PLAYER 0x00698ef0
// TEMPLATE: STUDIO 0x0085b710
// ??1?$signal@$$A6AXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x00698f80
// TEMPLATE: STUDIO 0x0085b7a0
// ??R?$signal_with_args@$01$$A6AXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@signals@rbx@@QAEXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z

// TEMPLATE: PLAYER 0x00699060
// TEMPLATE: STUDIO 0x0085b880
// ??1?$signal@$$A6AXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x0069aaf0
// TEMPLATE: STUDIO 0x0085c890
// ??A?$map@W4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@U?$less@W4CenterDialogType@GuiService@RBX@@@5@V?$allocator@U?$pair@$$CBW4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@@std@@@5@@std@@QAEAAV?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@1@ABW4CenterDialogType@GuiService@RBX@@@Z

// TEMPLATE: PLAYER 0x0069ac40
// TEMPLATE: STUDIO 0x0085c960
// ?erase@?$_Tree@V?$_Tmap_traits@W4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@U?$less@W4CenterDialogType@GuiService@RBX@@@5@V?$allocator@U?$pair@$$CBW4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@@std@@@5@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@0@Z

// TEMPLATE: PLAYER 0x0069b120
// TEMPLATE: STUDIO 0x0085ce40
// ??$?0PBD@?$Described@VGuiService@RBX@@$E?sGuiService@2@3QBDBV?$NonFactoryProduct@VInstance@RBX@@$E?sGuiService@2@3QBDB@2@$00$0A@@Reflection@RBX@@QAE@PBD@Z

// TEMPLATE: PLAYER 0x007f3ce0
// TEMPLATE: STUDIO 0x008fdcd0
// ?find@?$_Tree@V?$_Tset_traits@W4SpecialKey@GuiService@RBX@@U?$less@W4SpecialKey@GuiService@RBX@@@std@@V?$allocator@W4SpecialKey@GuiService@RBX@@@5@$0A@@std@@@std@@QAE?AViterator@12@ABW4SpecialKey@GuiService@RBX@@@Z

// Template copies the original emitted in this class's .obj (identical bodies, tools/unpaired_twins.py).
// clang-format off
// TEMPLATE: PLAYER 0x00693f30
// ?fireItem@?$signal_with_args@$01$$A6AXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@signals@rbx@@CAXPAVslot@?$signal@$$A6AXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@23@W4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z
// TEMPLATE: PLAYER 0x00696b10
// TEMPLATE: STUDIO 0x00859a60
// ??1?$pair@$$CBW4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@@std@@QAE@XZ
// TEMPLATE: PLAYER 0x00697400
// TEMPLATE: STUDIO 0x0085a030
// ?_Tidy@?$_Tree@V?$_Tset_traits@DU?$less@D@std@@V?$allocator@D@2@$0A@@std@@@std@@IAEXXZ
// TEMPLATE: PLAYER 0x006979a0
// ?_Tidy@?$_Tree@V?$_Tmap_traits@V?$weak_ptr@VGuiObject@RBX@@@boost@@PAUDialogWrapper@GuiService@RBX@@U?$less@V?$weak_ptr@VGuiObject@RBX@@@boost@@@std@@V?$allocator@U?$pair@$$CBV?$weak_ptr@VGuiObject@RBX@@@boost@@PAUDialogWrapper@GuiService@RBX@@@std@@@7@$0A@@std@@@std@@IAEXXZ
// TEMPLATE: PLAYER 0x00699950
// ??1Creator@?$FactoryProduct@VScreenGui@RBX@@VGuiLayerCollector@2@$E?sScreenGui@2@3QBDBVInstance@2@@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x00699ba0
// ??$?0PAVGuiMain@RBX@@VDeleter@?$Creatable@VInstance@RBX@@@1@@shared_count@detail@boost@@QAE@PAVGuiMain@RBX@@VDeleter@?$Creatable@VInstance@RBX@@@4@@Z
// TEMPLATE: PLAYER 0x00699e20
// ??1Creator@?$FactoryProduct@VGuiMain@RBX@@VScreenGui@2@$E?sGuiMain@2@3QBDBVInstance@2@@RBX@@QAE@XZ
// TEMPLATE: PLAYER 0x0069a880
// ?initSingleton@?$Singleton@$$CBV?$EnumDesc@W4SpecialKey@GuiService@RBX@@@Reflection@RBX@@@Reflection@RBX@@CAXXZ
// TEMPLATE: PLAYER 0x0069a8f0
// ?initSingleton@?$Singleton@$$CBV?$EnumDesc@W4CenterDialogType@GuiService@RBX@@@Reflection@RBX@@@Reflection@RBX@@CAXXZ
// TEMPLATE: PLAYER 0x0069aad0
// TEMPLATE: STUDIO 0x0085c870
// ??1?$signal_with_args@$01$$A6AXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@0@Z@signals@rbx@@QAE@XZ
// TEMPLATE: PLAYER 0x0069aae0
// TEMPLATE: STUDIO 0x0085c880
// ??1?$signal@$$A6AXW4SpecialKey@GuiService@RBX@@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z@rbx@@QAE@XZ
// TEMPLATE: PLAYER 0x0069e610
// TEMPLATE: STUDIO 0x00860360
// ??1?$_Tree@V?$_Tmap_traits@W4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@U?$less@W4CenterDialogType@GuiService@RBX@@@5@V?$allocator@U?$pair@$$CBW4CenterDialogType@GuiService@RBX@@V?$list@PAUDialogWrapper@GuiService@RBX@@V?$allocator@PAUDialogWrapper@GuiService@RBX@@@std@@@std@@@std@@@5@$0A@@std@@@std@@QAE@XZ
// clang-format on
