/* V8DataModel/GuiBuilder.h — October 2013 (minimal bring-up).
   Evidence: GuiBuilder::addCustomStat 0x3A436C / removeCustomStat 0x3A4DAC
     (F:\2013Decompile\exports\decomp\GuiBuilder_stats.txt):
      - std::map<std::string, GuiBuilder::Data> at this+16 (find + end-sentinel
        at this+20); DataModel::addCustomStat 0x2D16A8 / removeCustomStat 0x2D16D2
        / writeStatsSettings 0x2D16F4 delegate to the GuiBuilder member at
        DataModel+3488.
      - Both functions locate the "CustomStats" GuiItem via
        Instance::findConstFirstChildByName on the Instance* at GuiBuilder+2968,
        isA-checked against Described<GuiItem, sGuiItem, NonFactoryProduct<
        Instance, sGuiItem>, Functionality 27>.
   NOT YET PORTED (all GUI surface — needs GuiItem/TextLabel/Adorn):
     buildGui/buildStatsHud1+2/buildRenderStats/buildNetworkStats(+2)/
     buildPhysicsStats(+2)/buildFPS/buildSummaryStats/buildCustomStats/
     buildChatHud-PORTED/buildChatMenu/buildRightPalette-PORTED/buildLuaGui/addSafeChatMenu/
     Initialize(DataModel*)/updateGui-PORTED/updateSummaryStats-decl-PORTED/getVerb/
     buildNetworkStatsOutput (0x396E20..0x3A31EE).
   LAYOUT (2013 Windows, ctor PLAYER 0x00644f30 / STUDIO 0x00761ef0, dtor PLAYER 0x0056b810):
     0x00 dataModel (stored by Initialize, PLAYER 0x005c30c0, folded with
     RakNet::BitStream::SetWriteOffset), 0x04 workspace (buildGui), 0x08 safeChatMenu
     (shared_ptr, zeroed by the ctor), 0x10 customStats map (head at 0x14, size at 0x18),
     0x1c networkStatsPage (nextNetworkStats PLAYER 0x007f2e80 reads/writes +0x1c).
     PLAYER SIZE 0x20 (DataModel's parentDataModel follows at 0xe0c).
     STUDIO (Oct 17) SIZE 0x1c: no networkStatsPage, a real source difference (the
     STUDIO DataModel stores parentDataModel at 0xe08 right after guiBuilder 0xdec,
     ctor 0x0076b720). This header follows PLAYER.
     Data entries carry name+text only (binary Data holds GUI item refs — see
     0x3A436C past the map-find + 0x3A4DAC node teardown).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiBuilder.h */

#pragma once

#include <map>
#include "gui/EquationDisplay.h"
#include "gui/RelativePanel.h"  // for Data::display (binary +20).
#include <string>

// Global-namespace render adorn (GfxBase; 0-byte stub in this tree).
// Only the pointer shape is needed here (cf. DataModel::startCoreScripts
// mac: 0x2D617A taking Adorn*).
// Adorn = RBX::Adorn (binary mangle truth: buildGui 0x39715A /
// buildChatMenu 0x3A1F62 / buildRightPalette 0x3A230A all PNS_5AdornE,
// batch791).
#include "Gui/Adorn.h"

namespace RBX {

	class DataModel;
	class Instance;
	class Workspace;
	class TopMenuBar;
	class Verb;
	class ChatOption;
	class UnifiedWidget;
	class Frame;

	class GuiBuilder
	{
	public:
		// Custom-stat registry entry (binary Data payload TODO — GUI refs).
		struct Data
		{
			std::string text;
			shared_ptr<EquationDisplay> display;  // binary +20 (substat display node).
			Data() {}
			Data(const std::string& t) : text(t) {}
			Data(const std::string& t, shared_ptr<EquationDisplay> d) : text(t), display(d) {}
		};

		// Inline, emitted in DataModel.cpp (PLAYER 0x00644f30, STUDIO 0x00761ef0): only
		// safeChatMenu and customStats are constructed; dataModel, workspace and
		// networkStatsPage stay uninitialized until Initialize/buildGui.
		// FUNCTION: PLAYER 0x00644f30
		// FUNCTION: STUDIO 0x00761ef0
		GuiBuilder() {}

		// iOS: GuiBuilder::Initialize(DataModel*). PLAYER 0x005c30c0 holds it folded with
		// RakNet::BitStream::SetWriteOffset (that row owns the address, so no marker here).
		void Initialize(DataModel* dataModel);

		// mac: 0x2D617A callee (via DataModel::startCoreScripts): build the
		// in-game GUI. TODO: body (needs GuiItem/TextLabel/Adorn).
		void buildGui(Adorn* adorn, Workspace* workspace, bool buildInGameGui);

		// mac: 0x3A436C / 0x3A4DAC (map bookkeeping below; GuiItem creation TODO).
		void addCustomStat(const std::string& name, const std::string& text);
		void removeCustomStat(const std::string& name);
		// mac: 0x3A5008: cycle networkStatsPage 1->2->3->1.
		void nextNetworkStats();
		// mac: 0x39FAAC: FPS panel (1 EquationDisplay row + 400x22 restyle).
		shared_ptr<RelativePanel> buildFPS();
		// mac: 0x397F00: StatsHud1 panel (31 header/rows + 310x22 restyle; live values
		// via updaters TODO — rows carry their stat keys at build).
		shared_ptr<RelativePanel> buildStatsHud1();
		// mac: 0x399282: rows + restyle (live values via updaters TODO).
		shared_ptr<RelativePanel> buildStatsHud2();
		// mac: 0x39E102: rows + restyle (live values via updaters TODO).
		shared_ptr<RelativePanel> buildPhysicsStats();
		// mac: 0x39F042: rows + restyle (live values via updaters TODO).
		shared_ptr<RelativePanel> buildPhysicsStats2();
		// mac: 0x39FE08: rows + restyle (live values via updaters TODO).
		shared_ptr<RelativePanel> buildSummaryStats();
		// mac: 0x39A0B4: rows + restyle (live values via updaters TODO).
		shared_ptr<RelativePanel> buildRenderStats();
		// mac: 0x39B2A4: rows + restyle (live values via updaters TODO).
		shared_ptr<RelativePanel> buildNetworkStats();
		// mac: 0x39C284 (bool-gated visible; struct TODO): rows + restyle; bool gates visible (struct TODO).
		shared_ptr<RelativePanel> buildNetworkStats2(bool resetPage = false);
		// mac: 0x3A1454: CustomStats panel (header + per-registry rows + 440x22 restyle;
		// disk pre-seed via SimpleJSON TODO).
		shared_ptr<RelativePanel> buildCustomStats();
		// mac: 0x3A1DB4 (430B): RelativePanel "ChatHud" (Layout LEFT/TOP/(15,100)/
		// VERTICAL/clear) + ChatOutput child via setParentInternal(false).
		shared_ptr<RelativePanel> buildChatHud();
		// mac: 0x3A230A (322B): RelativePanel "RightPalette" (Layout RIGHT/BOTTOM/
		// (0,0)/VERTICAL/clear).  Takes Adorn* like the binary but never
		// touches it (verified: no arg_4 use in disasm) -- NULL-safe.
		shared_ptr<RelativePanel> buildRightPalette(Adorn* adorn);
		// mac: 0x3A1F62 (882B): 1-arg (PORTED below) -- was blocked on
		// ChatButton : UnifiedImageWidget (landed s219) + the factory
		// string ("Chat", rodata 0xB99CF8, batch342).
		shared_ptr<RelativePanel> buildChatMenu(Adorn* adorn);
		// mac: 0x3A51A8 (1087B): 4-arg recursive core (PORTED below) -- per child
		// ChatWidget ("prefix index", child label) parented under the passed
		// widget, then recurse with the child's subtree.
		void buildChatMenu(ChatOption* option, const std::string& prefix,
			shared_ptr<UnifiedWidget> parent);
		// mac: 0x3A244C (3490B): ControlFrame + 3 corner Frames + NotificationBox
		// + Exit/MouseLock GuiImageButtons, assembled under ControlFrame and
		// hooked via CoreGuiService::addChild.  See the cpp for the value
		// table + DELTAs.
		shared_ptr<Frame> buildLuaGui();
		// mac: 0x3A31FC (269B): find "SummaryStats" under the gui root, isA-check
		// against GuiItem (= Item), and if visible call updateSummaryStats.
		// No-ops on missing root/panel/failed check/hidden bar -- like binary.
		void updateGui();
		// mac: 0x3A330A (4193B): per-frame SummaryStats refresh (36 children,
		// :941 assert).  Full port -- see the cpp for the threshold table.
		void updateSummaryStats(TopMenuBar* hudArray);
		// mac: 0x3A55E8 (250B): shared FPS-color helper (value against hi/mid/lo
		// thresholds, big-flag font/size, backdrop clear, final 440x22).
		// Static like the binary (no this-use; all args on stack).
		static void updatePerformanceBasedStat(shared_ptr<TextDisplay> row,
			float value, float hi, float mid, float lo, bool big);
		// mac: 0x396E20 (135B): VerbContainer::getVerb on the Workspace+324
		// container (Workspace IS-A VerbContainer in this tree, so plain
		// workspace->getVerb(name)).  :116 asserts workspace non-null.
		Verb* getVerb(const std::string& name);
		// mac: 0x3A22D4 (53B): if the gb+8 safe-chat holder and gb+0 DataModel are
		// both set, parent the holder under DM+2968 (getGuiRoot()).  Exact:
		// with a NULL guiRoot this unparents (setParent(NULL) semantics).
		void addSafeChatMenu();
		// mac: 0x3A4FDA (45B): unparent the gb+8 holder if set.
		void removeSafeChatMenu();
		int getNetworkStatsPage() const { return networkStatsPage; }
		// mac: 0x3A4F38 — persists via CustomStatsGuiJSON::WriteFile (TODO: body).
		void saveCustomStats();

		// Minimal registry access for CustomStatsGuiJSON (binary reaches the
		// map at +16 directly; layout TODO).
		std::map<std::string, Data>& customStatsMap() { return customStats; }
		const std::map<std::string, Data>& customStatsMap() const { return customStats; }

	private:
		DataModel* dataModel;						// 0x00 Initialize
		Workspace* workspace;						// 0x04 buildGui
		shared_ptr<TopMenuBar> safeChatMenu;		// 0x08 (2016 type; nothing in 2013 stores it)
		std::map<std::string, Data> customStats;	// 0x10
		int networkStatsPage;						// 0x1c PLAYER only (nextNetworkStats 0x007f2e80)
		// Bring-up factoring (binary inlines these writes in each restyle loop).
		static void restyleStatRow(TextDisplay* row, int fontSize, float w, float h);
	};

} // namespace RBX
