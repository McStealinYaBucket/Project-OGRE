/* App/v8datamodel/GuiBuilder.cpp — October 2013 (custom-stat surface only;
   buildGui SCOUTED, bodies queued).
   Evidence: buildGui 0x39715A (3493B) + panel builders (statsHud1 0x397F00 /
     statsHud2 0x399282 / render 0x39A0B4 / network 0x39B2A4 / network2 0x39C284 /
     physics 0x39E102 / physics2 0x39F042 / fps 0x39FAAC / summary 0x39FE08 /
     customStats 0x3A1454 / chatHud 0x3A1DB4 / chatMenu 0x3A1F62 /
     rightPalette 0x3A230A / luaGui 0x3A244C / updateGui 0x3A31FC) +
     Initialize 0x3A31EE (gb+0 = DataModel*) + getVerb 0x396E20;
     buildGui shape: 10 stats panels under DM+2968 root, mode-gated Visible
     (FPS/Physics/Summary/Render), bool=true Player path (chatHud/chatMenu/
     safeChat/palette/LuaGui ControlFrame) vs false Studio path (stats only);
     stats use RelativePanel+Layout/TextDisplay/EquationDisplay (NO TextLabel
     in stats); chat = ChatOutput/ChatButton/ChatWidget; LuaGui = Frame x4 +
     NotificationBox + 2xGuiImageButton; Adorn only to chatMenu/palette.
     Member map: Gb+0 DataModel*, +4 Workspace*, +8 TopMenuBar, +16 registry,
     +40 networkStatsPage; DM+2936 Workspace*, +2968 guiRoot, +3488 GuiBuilder.
     Registry Data = {string value; shared_ptr<TextDisplay> display} (12B).
     Full decomps filed in Temp\opencode\guibuild_*.txt.
   Evidence: GuiBuilder::addCustomStat 0x3A436C / removeCustomStat 0x3A4DAC
     (F:\2013Decompile\exports\decomp\GuiBuilder_stats.txt),
     DataModel::addCustomStat 0x2D16A8 / removeCustomStat 0x2D16D2 /
     writeStatsSettings 0x2D16F4 (F:\2013Decompile\exports\decomp\DataModel_stats.txt),
     GuiBuilder::saveCustomStats 0x3A4F38 (GuiBuilder_save.txt).
   Ported: map<string, Data> registry bookkeeping (this+16) + DataModel lookup
     of the "CustomStats" GuiItem root via findConstFirstChildByName with the
     GuiItem isA check (Described<GuiItem, sGuiItem, NonFactoryProduct<Instance>,
     27>).
   CORRECTION (scouted): addCustomStat creates EquationDisplay, NOT TextLabel;
      gate = find "CustomStats" under DM+2968 + isA GuiItem (silent no-op else);
      TODO: EquationDisplay creation + restyle constants (+26..37) + parenting;
     node teardown inside removeCustomStat; saveCustomStats body (builds a
     CustomStatsGuiJSON writer (vtable off_EDC538) + WriteFile 0x396906, then
     erases a map<string, void(*)(const char*)> registry); the other ~20
     GuiBuilder methods (stats HUDs, chat, palettes, Initialize(DataModel*));
     GuiItem class itself now in tree as flattened Item shell (sGuiItem, 0x54E400).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiBuilder.cpp */

#include "stdafx.h"

#include "V8DataModel/GuiBuilder.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/CustomStatsGuiJSON.h"
#include "V8DataModel/GuiRoot.h"
#include "V8DataModel/Item.h"
#include "V8Tree/Instance.h"
#include "gui/ChatOutput.h"
#include "gui/ChatWidget.h"
#include "gui/ChatButton.h"
#include "V8DataModel/SafeChat.h"
#include "gui/Layout.h"
#include "gui/TopMenuBar.h"
#include "gui/UnifiedWidget.h"
#include "V8DataModel/ChatOption.h"
#include "V8DataModel/CoreGuiService.h"
#include "V8DataModel/Frame.h"
#include "V8DataModel/GuiImageButton.h"
#include "V8DataModel/NotificationBox.h"
#include "V8DataModel/Workspace.h"
#include "V8Tree/Service.h"
#include "V8Tree/Verb.h"
#include "Util/Object.h"
#include <sstream>

namespace RBX {

	// FUNCTION: PLAYER 0x005c30c0
	// FUNCTION: STUDIO 0x008a8950
	void GuiBuilder::Initialize(DataModel* dataModel)
	{
		this->dataModel = dataModel;
	}

	void GuiBuilder::addCustomStat(const std::string& name, const std::string& text)
	{
		// mac: 0x3A436C: no-op when the name is already registered.
		if (customStats.find(name) != customStats.end())
			return;
		// Gate: find "CustomStats" + isA GuiItem (Item shell), silent no-op else.
		// Then EquationDisplay(label=name+" : ", value) + parent under panel
		// (styling/positioning TODO — member map pending).
		shared_ptr<EquationDisplay> eq;
		if (dataModel != NULL)
		{
			const Instance* stats = dataModel->findConstFirstChildByName("CustomStats");
			const GuiItem* panel = (stats != NULL) ? Instance::fastDynamicCast<GuiItem>(stats) : NULL;
			if (panel != NULL)
			{
				eq = Creatable<Instance>::create<EquationDisplay>(name + " : ", text);
				eq->setParent(const_cast<GuiItem*>(panel));
			}
		}
		customStats[name] = Data(text, eq);
	}

	// STUB: PLAYER 0x007f2d40
	// STUB: STUDIO 0x008b5b20
	void GuiBuilder::removeCustomStat(const std::string& name)
	{
		// mac: 0x3A4DAC: unparent node (shared_ptr at Data+20 via setParent NULL, clear
		// byte at node+156), then tree-erase (erases even when panel missing).
		std::map<std::string, Data>::iterator it = customStats.find(name);
		if (it == customStats.end())
			return;
		if (dataModel != NULL)
		{
			const Instance* stats = dataModel->findConstFirstChildByName("CustomStats");
			(void)stats;
		}
		if (it->second.display != NULL)
			it->second.display->setParent(NULL);
		customStats.erase(it);
	}

	// Restyle helper (buildFPS/buildCustomStats assert lines 526/1172):
	// font/size/color/alpha/backdrop per constants below.
	void GuiBuilder::restyleStatRow(TextDisplay* row, int fontSize, float w, float h)
	{
		row->fontSize = fontSize;
		row->sizeX = w;
		row->sizeY = h;
		row->textColor = G3D::Color3(0.7f, 0.0f, 1.0f);
		row->textAlpha = 1.0f;
		row->backdrop = G3D::Color4::clear();
	}

	// mac: 0x3A230A (322B).  Layout ints verbatim: x=3 (RIGHT), y=1 (BOTTOM),
	// offset (0,0) (one dword zero store over both words), style=1
	// (VERTICAL), backdrop=clear.  Adorn* is never read -- NULL-safe.
	// STUB: PLAYER 0x007e6950
	// STUB: STUDIO 0x008aa4d0
	shared_ptr<RelativePanel> GuiBuilder::buildRightPalette(Adorn* adorn)
	{
		(void)adorn;
		Layout layout;
		layout.xLocation = Rect::RIGHT;
		layout.yLocation = Rect::BOTTOM;
		layout.offset = Vector2int16(0, 0);
		layout.layoutStyle = Layout::VERTICAL;
		layout.backdropColor = G3D::Color4::clear();
		shared_ptr<RelativePanel> panel =
			Creatable<Instance>::create<RelativePanel>(layout);
		panel->setName("RightPalette");
		return panel;
	}

	// mac: 0x3A1DB4 (430B).  Layout ints verbatim from disasm: x=2 (LEFT), y=0
	// (TOP), offset (15,100), style=1 (VERTICAL), backdrop=clear.  The child
	// goes under the panel via setParent (public wrapper for the binary's
	// setParentInternal(child, panel, false) -- same call, same arg).
	// STUB: PLAYER 0x007e6a90
	// STUB: STUDIO 0x008aa610
	shared_ptr<RelativePanel> GuiBuilder::buildChatHud()
	{
		Layout layout;
		layout.xLocation = Rect::LEFT;
		layout.yLocation = Rect::TOP;
		layout.offset = Vector2int16(15, 100);
		layout.layoutStyle = Layout::VERTICAL;
		layout.backdropColor = G3D::Color4::clear();
		shared_ptr<RelativePanel> panel =
			Creatable<Instance>::create<RelativePanel>(layout);
		panel->setName("ChatHud");
		shared_ptr<ChatOutput> chat =
			Creatable<Instance>::create<ChatOutput>();
		chat->setParent(panel.get());
		return panel;
	}
	// STUB: PLAYER 0x007e7260
	// STUB: STUDIO 0x008aade0
	shared_ptr<RelativePanel> GuiBuilder::buildStatsHud1()
	{
		// mac: 0x397F00: 31 children (verbatim label/key strings from IDA table).
		// Layout variant + live-value updaters TODO.
		static const char* kLabels[] = {
			NULL, "----- World ----",
			"Instances     : ", "numInstances",
			"Physics Mode  : ", "physicsMode",
			"Primitives    : ", "numPrimitives",
			"Moving Prims  : ", "numMovingPrimitives",
			"Joints        : ", "numJoints",
			"Contacts      : ", "numContacts",
			"Tree          : ", "maxTreeDepth",
			NULL, "----- Timing ----",
			"Physics       : ", "Physics",
			"Mouse Move    : ", "MouseMove",
			"Render        : ", "Render",
			"Delta bw render   : ", "Delta Between Renders",
			"Ogre   : ", "Ogre",
			"Total Render   : ", "Total Render",
			"Render Nom     : ", "Render Nominal FPS",
			NULL, "----- Graphics ----",
			"Graphic Mode  : ", "Graphics Mode",
			"Video Memory  : ", "Video Memory",
			"Anti-Aliasing : ", "Anti-Aliasing",
			"Frame Id      : ", "drawId",
			"World Id      : ", "worldId",
			NULL, "----- Throttles ----",
			"EnviroSpeed % : ", "environmentSpeed",
			"FRM	          : ", "FRM",
			"FRM Target  : ", "FRM Target",
			"FRM Visible   : ", "FRM Visible",
			"FRM Quality   : ", "FRM Quality",
			"Particles     : ", "Particles",
			NULL, "----- Network ----",
			"RequestQueue  : ", "RequestQueueSize",
		};
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("StatsHud1");
		for (int i = 0; i < 31; ++i)
		{
			if (kLabels[i * 2] == NULL)
			{
				shared_ptr<TextDisplay> header = Creatable<Instance>::create<TextDisplay>("", kLabels[i * 2 + 1]);
				header->setParent(panel.get());
				restyleStatRow(header.get(), 12, 310.0f, 22.0f);
			}
			else
			{
				shared_ptr<EquationDisplay> row = Creatable<Instance>::create<EquationDisplay>(kLabels[i * 2], kLabels[i * 2 + 1]);
				row->setParent(panel.get());
				restyleStatRow(row.get(), 12, 310.0f, 22.0f);
			}
		}
		return panel;
	}

	// STUB: PLAYER 0x007e8460
	// STUB: STUDIO 0x008abfe0
	shared_ptr<RelativePanel> GuiBuilder::buildRenderStats()
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("RenderStats");
		panel->visible = false;
		{
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Graphics ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Physics       : ", "Physics"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Heartbeat     : ", "Heartbeat"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Network send  : ", "Network Send"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Network receive : ", "Network Receive"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Video Memory  : ", "Video Memory"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Resolution: ", "RenderStatsResolution"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Performance -----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Config: ", "RenderStatsFRMConfig"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Blocks: ", "RenderStatsFRMBlocks"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Adjust: ", "RenderStatsFRMAdjust"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Target Time: ", "RenderStatsFRMTargetTime"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Distance: ", "RenderStatsFRMDistance"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Framerate Variance: ", "FRM Variance"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Timing -----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Scheduler Render        : ", "Render"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("CPU: ", "RenderStatsTimeTotal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("GPU: ", "RenderStatsGPU"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Delta: ", "RenderStatsDelta"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (total): ", "RenderStatsPassTotal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (scene): ", "RenderStatsPassScene"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (shadow): ", "RenderStatsPassShadow"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (UI): ", "RenderStatsPassUI"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (3D Adorns): ", "RenderStatsPass3dAdorns"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Light grid: ", "RenderStatsLightGrid"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Geometry Gen: ", "RenderStatsGeometryGen"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Clusters: ", "RenderStatsClusters"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("TC: ", "RenderStatsTC"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
		}
		return panel;
	}

	// STUB: PLAYER 0x007e9500
	// STUB: STUDIO 0x008ad080
	shared_ptr<RelativePanel> GuiBuilder::buildNetworkStats()
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("NetworkStats");
		panel->visible = false;
		{
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Replicator ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("General  (MTU Size, Ping) :", "General  (MTU Size, Ping) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("              ", "GeneralStats"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("----- Incoming ----", "----- Incoming ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Overall (KB/s, Pkt/s, Queue) :", "Overall (KB/s, Pkt/s, Queue) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("IncomingStats", "IncomingStats"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("In Data (KB/s, Pkt/s, Avg Size) :", "In Data (KB/s, Pkt/s, Avg Size) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("InData", "InData"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("In Physics (KB/s, Pkt/s, Avg Size, Avg Lag) :", "In Physics (KB/s, Pkt/s, Avg Size, Avg Lag) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("InPhysics", "InPhysics"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("In Touches (KB/s, Pkt/s, Avg Size) :", "In Touches (KB/s, Pkt/s, Avg Size) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("InTouches", "InTouches"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("In Clusters (KB/s, Pkt/s, Avg Size) :", "In Clusters (KB/s, Pkt/s, Avg Size) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("InClusters", "InClusters"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("----- Outgoing ----", "----- Outgoing ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Overall (KB/s) :", "OutgoingStats"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Out Data (KB/s, Pkt/s, Avg Size, Throttle) :", "Out Data (KB/s, Pkt/s, Avg Size, Throttle) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("OutData", "OutData"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Out Physics (KB/s, Pkt/s, Avg Size, Throttle) :", "Out Physics (KB/s, Pkt/s, Avg Size, Throttle) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("OutPhysics", "OutPhysics"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Out Touches (KB/s, Pkt/s, Avg Size, #Waiting) :", "Out Touches (KB/s, Pkt/s, Avg Size, #Waiting) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("OutTouches", "OutTouches"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Out Clusters (KB/s, Pkt/s, Avg Size) :", "Out Clusters (KB/s, Pkt/s, Avg Size) :"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("OutClusters", "OutClusters"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 420.0f, 22.0f); }
		}
		return panel;
	}

	// STUB: PLAYER 0x007ea3b0
	// STUB: STUDIO 0x008ae4e0
	shared_ptr<RelativePanel> GuiBuilder::buildFPS()
	{
		// mac: 0x39FAAC: panel "FPS" variant 0 + EquationDisplay row, restyle 400x22.
		// Panel +132 store (global byte) TODO — member unmapped.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("FPS");
		shared_ptr<EquationDisplay> row = Creatable<Instance>::create<EquationDisplay>("FPS              : ", "Render");
		row->setParent(panel.get());
		// End-state equivalent (binary writes 240x48 transiently, then 400x22).
		restyleStatRow(row.get(), 24, 400.0f, 22.0f);
		return panel;
	}

	// STUB: PLAYER 0x007ea7a0
	shared_ptr<RelativePanel> GuiBuilder::buildNetworkStats2(bool resetPage)
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("NetworkStats2");
		panel->visible = false;
		{
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- In Physics ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Class) Type (KB/s, Avg Size, Count/s)", "Class) Type (KB/s, Avg Size, Count/s)"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("", "InPhysicsDetails"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Out Physics ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Class) Type (KB/s, Avg Size, Count/s)", "Class) Type (KB/s, Avg Size, Count/s)"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("", "OutPhysicsDetails"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Raknet ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("msgTotalBytesPushed/s : ", "messageDataBytesSentPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("msgTotalBytesSent/s : ", "messageTotalBytesSentPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("msgDataBytesResent/s : ", "messageDataBytesResentPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("msgBytesRecv/s : ", "messagesBytesReceivedPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("msgBytesRecvAndIgnored/s : ", "messagesBytesReceivedAndIgnoredPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("bytesSent/s : ", "bytesSentPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("bytesReceived/s : ", "bytesReceivedPerSec"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("outBandwidthLimitBytes/s : ", "outgoingBandwidthLimitBytesPerSecond"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("limitedByOutBandwidthLimit : ", "isLimitedByOutgoingBandwidthLimit"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("congestionCtrlLimitBytes/s : ", "congestionControlLimitBytesPerSecond"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("limitedByCongestionControl : ", "isLimitedByCongestionControl"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("messagesInResendQueue : ", "messagesInResendQueue"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("bytesInResendQueue : ", "bytesInResendQueue"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("packetlossLastSecond : ", "packetlossLastSecond"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- In Data ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Details (KB/s, Avg Size, Count/s)", "Details (KB/s, Avg Size, Count/s)"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("", "InDataDetails"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Out Data ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("Details (KB/s, Avg Size, Count/s)", "Details (KB/s, Avg Size, Count/s)"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("", "OutDataDetails"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", ""); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 280.0f, 22.0f); }
		}
		// Conditional store (bool-gated @0x39D910); struct reset TODO.
		if (!resetPage)
			panel->visible = true;
		return panel;
	}

	// STUB: PLAYER 0x007ec640
	// STUB: STUDIO 0x008af690
	shared_ptr<RelativePanel> GuiBuilder::buildStatsHud2()
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("StatsHud2");
		panel->visible = false;
		{
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Contacts ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("CtctStageCtcts: ", "numContactsInCollisionStage"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("SteppingCtcts : ", "numSteppingContacts"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("TouchingCtcts : ", "numTouchingContacts"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("% Pair Hit    : ", "contactPairHitRatio"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("% Feature Hit : ", "contactFeatureHitRatio"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("# link(p)     : ", "numLinkCalls"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("sH Nodes Out  : ", "numHashNodes"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("sH Max Bucket : ", "maxBucketSize"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Kernel ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Solver Iteratn: ", "solverIterations"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Matrix Size   : ", "matrixSize"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Bodies        : ", "numBodies"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("MaxBodies     : ", "maxBodies"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Leaves        : ", "numLeafBodies"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Constraints   : ", "numConstraints"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Points        : ", "numPoints"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("% Active CC's : ", "percentConnectorsActive"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Energy B      : ", "energyBody"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Energy C      : ", "energyConnector"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Energy T      : ", "energyTotal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
		}
		return panel;
	}

	// STUB: PLAYER 0x007ed390
	// STUB: STUDIO 0x008b03e0
	shared_ptr<RelativePanel> GuiBuilder::buildPhysicsStats()
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("PhysicsStats");
		panel->visible = false;
		{
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- FPS ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Physics       : ", "Physics"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("PhysicsReal   : ", "PhysicsReal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Render        : ", "Render"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Heartbeat     : ", "Heartbeat"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Network send  : ", "Network Send"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Network recv  : ", "Network Receive"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Rx data pkts  : ", "Received Data Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Rx phys pkts  : ", "Received Physics Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- World ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Player Radius : ", "Player Radius"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Parts		  : ", "numPrimitives"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Moving parts  : ", "numMovingPrimitives"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Joints        : ", "numJoints"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Contacts      : ", "numContactsAll"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("% Pair Hit    : ", "contactPairHitRatio"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("% Feature Hit : ", "contactFeatureHitRatio"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("sH Nodes      : ", "numHashNodes"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Kernel ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Bodies        : ", "numBodiesAll"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Connectors    : ", "numConnectorsAll"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Energy T      : ", "energyTotal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Iterations    : ", "numIterations"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
		}
		return panel;
	}
	// STUB: PLAYER 0x007eed30
	// STUB: STUDIO 0x008b1d80
	shared_ptr<RelativePanel> GuiBuilder::buildSummaryStats()
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("SummaryStats");
		panel->visible = false;
		{
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FPS         : ", "Effective FPS"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Physics         : ", "Physics"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Render          : ", "Render"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Network receive : ", "Network Receive"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Rendering -----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Video Memory  : ", "Video Memory"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Auto Quality: ", "Auto Quality"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Quality     : ", "FRM Quality"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("FRM Visible     : ", "FRM Visible"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Particles     : ", "Particles"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Scheduler Render        : ", "Render"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Present time            : ", "Present Time"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (total): ", "RenderStatsPassTotal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (scene): ", "RenderStatsPassScene"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (shadow): ", "RenderStatsPassShadow"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (UI): ", "RenderStatsPassUI"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Draw (3D Adorns): ", "RenderStatsPass3dAdorns"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("----- Physics -----", "----- Physics -----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Parts		  : ", "numPrimitives"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Moving parts  : ", "numMovingPrimitives"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Joints        : ", "numJoints"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Contacts      : ", "numContacts"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("EnviroSpeed % : ", "environmentSpeed"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Energy T      : ", "energyTotal"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("TouchingCtcts : ", "numTouchingContacts"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("----- Network -----", "----- Network -----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Data Ping       : ", "Data Ping"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Send KBytesPerSec       : ", "Send kBps"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Receive KBytesPerSec       : ", "Receive kBps"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Incoming Packet Queue       : ", "Packet Queue"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Received Data Packets      : ", "Received Data Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Received Physics Packets       : ", "Received Physics Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Received Cluster Packets      : ", "Received Cluster Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("messagesInResendQueue      : ", "messagesInResendQueue"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Sent Data Packets       : ", "Sent Data Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("Sent Physics Packets       : ", "Sent Physics Packets"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 440.0f, 22.0f); }
		}
		return panel;
	}
	// STUB: PLAYER 0x007f01a0
	// STUB: STUDIO 0x008b31f0
	shared_ptr<RelativePanel> GuiBuilder::buildPhysicsStats2()
	{
		// Rows verbatim from IDA table; updaters TODO.
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("PhysicsStats2");
		panel->visible = false;
		{
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "----- Physics Profiling ----"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<TextDisplay> w = Creatable<Instance>::create<TextDisplay>("", "World Step -"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Break Time:			", "Break Time"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Assembler Time:	", "Assembler Time"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Filter Time:            ", "Filter Time"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  UI Step:                  ", "UI Step"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Broadphase:           ", "Broadphase"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Collision:                 ", "Collision"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Joint Sleep:           ", "Joint Sleep"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Wake:                    ", "Wake"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Sleep:                     ", "Sleep"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Joint Update:          ", "Joint Update"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Bodies:                   ", "Kernel Bodies"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
			{ shared_ptr<EquationDisplay> w = Creatable<Instance>::create<EquationDisplay>("  Connectors:             ", "Kernel Connectors"); w->setParent(panel.get()); restyleStatRow(w.get(), 12, 260.0f, 22.0f); }
		}
		return panel;
	}

	// STUB: PLAYER 0x007f14e0
	// STUB: STUDIO 0x008b44f0
	void GuiBuilder::saveCustomStats()
	{
		// mac: 0x3A4F38: persist the registry via CustomStatsGuiJSON::WriteFile
		// (binary builds the writer object on the stack; heap here — same
		// observable behavior. Registry callback map + JSON reader TODO).
		CustomStatsGuiJSON writer(this);
		writer.WriteFile();
	}

	// STUB: PLAYER 0x007f1590
	// STUB: STUDIO 0x008b45a0
	shared_ptr<RelativePanel> GuiBuilder::buildCustomStats()
	{
		// mac: 0x3A1454: panel "CustomStats" + header + one row per registry node,
		// restyle 440x22/font12 (disk pre-seed via SimpleJSON TODO).
		shared_ptr<RelativePanel> panel = Creatable<Instance>::create<RelativePanel>();
		panel->setName("CustomStats");
		shared_ptr<TextDisplay> header = Creatable<Instance>::create<TextDisplay>("", "----- Custom Stats -----");
		header->setParent(panel.get());
		for (std::map<std::string, Data>::iterator it = customStats.begin(); it != customStats.end(); ++it)
		{
			shared_ptr<EquationDisplay> row = Creatable<Instance>::create<EquationDisplay>(it->first + " : ", it->second.text);
			row->setParent(panel.get());
			restyleStatRow(row.get(), 12, 440.0f, 22.0f);
			it->second.display = row;
		}
		restyleStatRow(header.get(), 12, 440.0f, 22.0f);
		return panel;
	}

	// mac: 0x3A31FC (269B).  DELTA (layout): the binary reads the gui root from
	// GuiBuilder+2968; this tree's minimal GuiBuilder does not preserve
	// binary offsets (see header), so the root comes from
	// dataModel->getGuiRoot() -- same object the binary's slot points at
	// (DM+2968).  isA GuiItem == fastDynamicCast<Item> (Item IS the C++
	// GuiItem: sGuiItem="Item", NonFactoryProduct<GuiItem, Instance>).
	// vtbl+148 is the virtual isVisible() -- dispatched the same way.
	// STUB: PLAYER 0x007f28f0
	// STUB: STUDIO 0x008b56d0
	void GuiBuilder::updateGui()
	{
		if (dataModel == NULL)
			return;
		Instance* root = dataModel->getGuiRoot();
		if (root == NULL)
			return;
		const Instance* found = root->findConstFirstChildByName("SummaryStats");
		const GuiItem* item = (found != NULL)
			? Instance::fastDynamicCast<GuiItem>(found) : NULL;
		if (item == NULL)
			return;
		const TopMenuBar* bar = Instance::fastDynamicCast<TopMenuBar>(found);
		if (bar != NULL && bar->isVisible())
			updateSummaryStats(const_cast<TopMenuBar*>(bar));
	}

	// STUB: PLAYER 0x007f2e80
	void GuiBuilder::nextNetworkStats()
	{
		networkStatsPage = (networkStatsPage >= 3) ? 1 : networkStatsPage + 1;
	}

	// mac: 0x3A55E8 (250B).  Colors inlined as literals (Color3.cpp is NOT in
	// the link -- see the TextDisplay link-debt note); values byte-proven
	// from the static bodies (batch180): green (0,1,0), yellow (1,1,0),
	// orange (1,0.5,0), red (1,0,0).
	// NOTE: the font/size writes are inlined, NOT via restyleStatRow --
	// restyleStatRow also stamps textColor purple and would clobber the
	// threshold color set above (caught live: rows stayed purple).
	void GuiBuilder::updatePerformanceBasedStat(shared_ptr<TextDisplay> row,
		float value, float hi, float mid, float lo, bool big)
	{
		if (value >= hi)
			row->textColor = G3D::Color3(0.0f, 1.0f, 0.0f);
		else if (value >= mid)
			row->textColor = G3D::Color3(1.0f, 1.0f, 0.0f);
		else if (value >= lo)
			row->textColor = G3D::Color3(1.0f, 0.5f, 0.0f);
		else
			row->textColor = G3D::Color3(1.0f, 0.0f, 0.0f);
		row->textAlpha = 1.0f;
		if (big)
		{
			row->fontSize = 18;
			row->sizeX = 180.0f;
			row->sizeY = 36.0f;
		}
		else
		{
			row->fontSize = 12;
			row->sizeX = 120.0f;
			row->sizeY = 24.0f;
		}
		row->backdrop = G3D::Color4::clear();
		row->sizeX = 440.0f;
		row->sizeY = 22.0f;
	}

	// mac: 0x3A330A (4193B).
	// Stat reads go through DataModel vtable slot +160
	// (DataModel::getMetricValue, IMetric); live counters are unported so the default
	// is 0.0 -- the whole threshold tree below is still exact and is tested
	// with those zeros.
	// Row mapping is INDEX-faithful to the binary (child0<-Effective,
	// child1<-Render, child2<-Physics, child3<-network): note our panel's
	// rows 1/2 are labeled Physics/Render (string-table order), so with
	// nonzero stats the colors would sit under swapped labels -- flagged,
	// not fixed (creation order is its own question; the update mapping is
	// what's ported here).
	// Hysteresis bytes FB2AC0/1/2 are function-statics (BSS-false start).
	// STUB: PLAYER 0x007ee1d0
	// STUB: STUDIO 0x008b1220
	void GuiBuilder::updateSummaryStats(TopMenuBar* hudArray)
	{
		RBXASSERT(hudArray != NULL
			&& hudArray->numChildren() == 36);  // "NetworkStat_End == hudArray->numChildren()"
		static bool showRenderRows = false;  // FB2AC0.
		static bool showPhysicsRows = false;  // FB2AC1.
		static bool showNetworkRows = false;  // FB2AC2.

		double effFPS = dataModel->getMetricValue("Effective FPS");
		double renderFPS = dataModel->getMetricValue("Render FPS");
		double physicsFPS = dataModel->getMetricValue("Physics FPS");
		double netCPU = dataModel->getMetricValue("Network Receive CPU");
		double physPackets = dataModel->getMetricValue("Received Physics Packets");

		// Render-time dominance (only when Effective < 30).
		bool renderDominant = false;
		bool renderFlag = false;
		bool unstable = false;
		if ((float)effFPS < 30.0f)
		{
			double frameT = dataModel->getMetricValue("Frame Time");
			double renderT = dataModel->getMetricValue("Render Time");
			double physT = dataModel->getMetricValue("Physics Time");
			double netT = dataModel->getMetricValue("Network Receive Time");
			float fT = (float)frameT;
			float rT = (float)renderT;
			float pT = (float)physT;
			float nT = (float)netT;
			// renderFlag ends true ONLY when physics strictly dominates AND
			// exceeds 0.4x frame time (the LABEL_26 v28=0 zeroes every other
			// fall-through path -- verified against the jumps, not the
			// decompiler's label placement).
			if (rT <= pT || rT <= nT || rT <= 0.4f * fT)
			{
				if (pT > rT && pT > nT && pT > 0.4f * fT)
					renderFlag = true;
				else
					unstable = (nT > rT && nT > pT)
						|| ((float)netCPU > 0.5f);
			}
			else
			{
				renderDominant = true;
			}
		}

		// Hysteresis bands (show while bad, hide at 15, re-show under 20).
		bool showRender = true;
		if ((float)renderFPS >= 15.0f)
		{
			showRender = false;
			if (showRenderRows)
				showRender = (float)renderFPS < 20.0f;
		}
		showRenderRows = showRender;
		bool showPhysics = true;
		if ((float)physicsFPS >= 15.0f)
		{
			showPhysics = false;
			if (showPhysicsRows)
				showPhysics = (float)physicsFPS < 20.0f;
		}
		showPhysicsRows = showPhysics;

		// Network section flag (LABEL_39/LABEL_53 goto-free spelling --
		// verified arm by arm: default true, early-true on bad net,
		// hysteresis under previous-show, else false).
		bool showNet;
		float netForColor = (float)netCPU;
		if ((float)netCPU <= 0.0f && (float)physPackets <= 0.0f)
			showNet = false;
		else if ((float)netCPU > 25.0f || (float)physPackets < 15.0f)
			showNet = true;
		else if (showNetworkRows)
			showNet = ((float)netCPU > 20.0f)
				? true
				: ((float)physPackets < 17.0f);
		else
			showNet = false;
		showNetworkRows = showNet;

		// Children 0/1/2: FPS performance rows (index mapping above).
		// shared_from_dynamic_cast is the binary's own helper (null on a
		// failed cast, like the binary's unchecked use for these three).
		updatePerformanceBasedStat(
			shared_from_dynamic_cast<TextDisplay>(hudArray->getChild(0)),
			(float)effFPS, 28.0f, 20.0f, 10.0f, false);
		updatePerformanceBasedStat(
			shared_from_dynamic_cast<TextDisplay>(hudArray->getChild(1)),
			(float)renderFPS, 28.0f, 20.0f, 10.0f, renderFlag);
		updatePerformanceBasedStat(
			shared_from_dynamic_cast<TextDisplay>(hudArray->getChild(2)),
			(float)physicsFPS, 28.0f, 20.0f, 10.0f, renderDominant);

		// Child 3: network status color + restyle.  fastDynamicCast plays
		// the dynamic-cast role; the binary does not null-check this one
		// (only the loop rows get :1030+ asserts), so neither do we.
		TextDisplay* netRow = Instance::fastDynamicCast<TextDisplay>(
			hudArray->getChild(3));
		if (showNet
			|| (netForColor < 15.0f && (float)physPackets >= 19.0f))
			netRow->textColor = G3D::Color3(0.0f, 1.0f, 0.0f);
		else if (netForColor < 20.0f && (float)physPackets >= 16.0f)
			netRow->textColor = G3D::Color3(1.0f, 1.0f, 0.0f);
		else if (netForColor < 25.0f && (float)physPackets >= 12.0f)
			netRow->textColor = G3D::Color3(1.0f, 0.5f, 0.0f);
		else
			netRow->textColor = G3D::Color3(1.0f, 0.0f, 0.0f);
		netRow->textAlpha = 1.0f;
		if (unstable)
		{
			netRow->fontSize = 18;
			netRow->sizeX = 180.0f;
			netRow->sizeY = 36.0f;
		}
		else
		{
			netRow->fontSize = 12;
			netRow->sizeX = 120.0f;
			netRow->sizeY = 24.0f;
		}
		netRow->backdrop = G3D::Color4::clear();
		netRow->sizeX = 440.0f;
		netRow->sizeY = 22.0f;

		// Children 4..35: section visibility (COW snapshot + dynamic_cast
		// + :1030-family asserts in the binary; single-threaded direct walk
		// here, same order, same asserts).
		for (size_t i = 4; i < hudArray->numChildren(); ++i)
		{
			TextDisplay* item = Instance::fastDynamicCast<TextDisplay>(
				hudArray->getChild(i));
			RBXASSERT(item != NULL);  // :1030 "item"
			item->visible = false;
		}
		if (showRenderRows)
		{
			for (size_t j = 4; j < 17 && j < hudArray->numChildren(); ++j)
			{
				TextDisplay* item = Instance::fastDynamicCast<TextDisplay>(
					hudArray->getChild(j));
				RBXASSERT(item != NULL);  // :1038 "item"
				item->visible = true;
			}
		}
		if (showPhysicsRows)
		{
			for (size_t k = 17; k < 25 && k < hudArray->numChildren(); ++k)
			{
				TextDisplay* item = Instance::fastDynamicCast<TextDisplay>(
					hudArray->getChild(k));
				RBXASSERT(item != NULL);  // :1047 "item"
				item->visible = true;
			}
		}
		if (showNetworkRows)
		{
			for (size_t m = 25; m < 36 && m < hudArray->numChildren(); ++m)
			{
				TextDisplay* item = Instance::fastDynamicCast<TextDisplay>(
					hudArray->getChild(m));
				RBXASSERT(item != NULL);  // :1056 "item"
				item->visible = true;
			}
		}
	}

	// mac: 0x396E20 (135B).  One-liner past the assert: the +324 container is the
	// VerbContainer subobject of Workspace (multiple inheritance carries it;
	// no offset math needed in-tree).
	// STUB: PLAYER 0x007e4d40
	// STUB: STUDIO 0x008a88f0
	Verb* GuiBuilder::getVerb(const std::string& name)
	{
		RBXASSERT(workspace != NULL);  // GuiBuilder.cpp:116 "workspace"
		return workspace->getVerb(name);
	}

	// mac: 0x3A51A8 (1087B).  The binary's first param (Adorn*, threaded from the
	// 1-arg overload) is never read -- only passed through the recursion.
	// Name shape verbatim: prefix + ' ' + (unsigned long)index, built with
	// ostringstream exactly like the binary (__ostream_insert x2 +
	// _M_insert<ulong>).  Per child: ChatWidget(name, child-label) under the
	// passed widget (setParent == setParentInternal(x, y, false)), recurse
	// with (child, name, widget).  shared_ptr<ChatWidget> converts to the
	// shared_ptr<UnifiedWidget> param implicitly (base-class conversion).
	// No NULL guard -- the binary derefs option immediately; callers never
	// pass NULL.
	void GuiBuilder::buildChatMenu(ChatOption* option,
		const std::string& prefix, shared_ptr<UnifiedWidget> parent)
	{
		for (size_t i = 0; i < option->children.size(); ++i)
		{
			ChatOption* child = option->children[i];
			std::ostringstream os;
			os << prefix << ' ' << (unsigned long)i;
			shared_ptr<ChatWidget> widget =
				Creatable<Instance>::create<ChatWidget>(os.str(), child->label);
			widget->setParent(parent.get());
			buildChatMenu(child, os.str(), widget);
		}
	}

	// mac: 0x3A1F62 (882B, batch285: ChatMenuPanel + ChatButton factory
	// ("Chat", 15, rodata 0xB99CF8) + SafeChat-root subtree + panel
	// store/return).
	// STUB: PLAYER 0x007e6eb0
	// STUB: STUDIO 0x008aaa30
	shared_ptr<RelativePanel> GuiBuilder::buildChatMenu(Adorn* adorn)
	{
		Layout layout;
		layout.xLocation = Rect::LEFT;
		layout.yLocation = Rect::TOP;
		layout.offset = Vector2int16(15, 130);
		layout.layoutStyle = Layout::VERTICAL;
		layout.backdropColor = G3D::Color4::clear();
		shared_ptr<RelativePanel> panel =
			Creatable<Instance>::create<RelativePanel>(layout);
		panel->setName("ChatMenuPanel");
		shared_ptr<ChatButton> button =
			Creatable<Instance>::create<ChatButton>(adorn, "Chat", 15);
		button->setParent(panel.get());
		buildChatMenu(SafeChat::singleton().getChatRoot(), "", button);
		return panel;
	}

	// mac: 0x3A22D4 (53B) / 0x3A4FDA (45B).  setParent is the public wrapper for
	// the binary's setParentInternal(x, y, false) (private in this tree).
	void GuiBuilder::addSafeChatMenu()
	{
		if (safeChatMenu != NULL && dataModel != NULL)
			safeChatMenu->setParent(dataModel->getGuiRoot());
	}
	void GuiBuilder::removeSafeChatMenu()
	{
		if (safeChatMenu != NULL)
			safeChatMenu->setParent(NULL);
	}

	// mac: 0x3A244C (3490B).  Value table (all UDim2s verbatim from the disasm
	// stores; floats decoded: 1065353216 = 1.0f, 1028443341 = 0.05f):
	//   ControlFrame: size (1,0,1,0), trans 1.0, locked.
	//   BottomLeftControl: size (0,130,0,46), pos (0,0,1,-46).
	//   BottomRightControl: size (0,180,0,41), pos (1,-180,1,-41).
	//   TopLeftControl: size (0.05,0,0.05,0).
	//   Exit button "Exit": image rbxasset://textures/Exit.png, state 7,
	//     size (0,56,0,41), pos (0,2,0,-2), under TopLeftControl.
	//   MouseLock button "MouseLockLabel": image
	//     rbxasset://textures/ui/mouseLock_off.png, size (0,149,0,30),
	//     pos (0,2,0,-33), under BottomLeftControl.
	//   Assembly: BL/BR/TL/NotificationBox under ControlFrame; the service
	//   tail parents ControlFrame under CoreGuiService's +116 slot.
	// DELTAS: the TopLeftControl+36 registry call (unknown singleton +12
	//   virtual -- no EA, skipped); the MouseLock visibility gate reads
	//   GlobalBasicSettingsItem<GameBasicSettings> singleton +104 (class
	//   absent) -- defaults hidden (fresh zero-state memory reads 0, and the
	//   binary hides on anything but 1); the CoreGuiService tail is skipped
	//   when dataModel is NULL (binary never runs model-less); texture
	//   binding (render pipeline -- the ContentIds are stored, never bound).
	// STUB: PLAYER 0x007f0ba0
	// STUB: STUDIO 0x008b3bf0
	shared_ptr<Frame> GuiBuilder::buildLuaGui()
	{
		shared_ptr<Frame> controlFrame =
			Creatable<Instance>::create<Frame>();
		controlFrame->setName("ControlFrame");
		controlFrame->setBackgroundTransparency(1.0f);
		controlFrame->setSize(UDim2(1.0f, 0, 1.0f, 0));
		controlFrame->setRobloxLocked(true);

		shared_ptr<Frame> bottomLeft =
			Creatable<Instance>::create<Frame>();
		bottomLeft->setSize(UDim2(0.0f, 130, 0.0f, 46));
		bottomLeft->setPosition(UDim2(0.0f, 0, 1.0f, -46));
		bottomLeft->setBackgroundTransparency(1.0f);
		bottomLeft->setName("BottomLeftControl");
		bottomLeft->setRobloxLocked(true);

		shared_ptr<NotificationBox> notificationBox =
			Creatable<Instance>::create<NotificationBox>();
		shared_ptr<Frame> bottomRight =
			Creatable<Instance>::create<Frame>();
		bottomRight->setSize(UDim2(0.0f, 180, 0.0f, 41));
		bottomRight->setPosition(UDim2(1.0f, -180, 1.0f, -41));
		bottomRight->setBackgroundTransparency(1.0f);
		bottomRight->setName("BottomRightControl");
		bottomRight->setRobloxLocked(true);

		shared_ptr<Frame> topLeft =
			Creatable<Instance>::create<Frame>();
		topLeft->setSize(UDim2(0.05f, 0, 0.05f, 0));
		topLeft->setBackgroundTransparency(1.0f);
		topLeft->setName("TopLeftControl");
		// DELTA: TopLeftControl+36 registry call (see header note).

		shared_ptr<GuiImageButton> exitButton =
			Creatable<Instance>::create<GuiImageButton>(getVerb("Exit"));
		exitButton->setImage(
			ContentId("rbxasset://textures/Exit.png"));
		exitButton->setImageState(7);
		exitButton->setName("Exit");
		exitButton->setBackgroundTransparency(1.0f);
		exitButton->setSize(UDim2(0.0f, 56, 0.0f, 41));
		exitButton->setPosition(UDim2(0.0f, 2, 0.0f, -2));
		exitButton->setRobloxLocked(true);
		exitButton->setParent(topLeft.get());

		shared_ptr<GuiImageButton> mouseLockButton =
			Creatable<Instance>::create<GuiImageButton>(
				getVerb("ToggleViewMode"));
		mouseLockButton->setName("MouseLockLabel");
		mouseLockButton->setImage(ContentId(
			"rbxasset://textures/ui/mouseLock_off.png"));
		mouseLockButton->setSize(UDim2(0.0f, 149, 0.0f, 30));
		mouseLockButton->setBackgroundTransparency(1.0f);
		mouseLockButton->setPosition(UDim2(0.0f, 2, 0.0f, -33));
		mouseLockButton->setParent(bottomLeft.get());
		// DELTA: visibility gate (see header note) -- hidden.
		mouseLockButton->setVisible(false);

		bottomLeft->setParent(controlFrame.get());
		bottomRight->setParent(controlFrame.get());
		topLeft->setParent(controlFrame.get());
		notificationBox->setParent(controlFrame.get());

		// Service tail (skipped model-less; binary never runs that way).
		if (dataModel != NULL)
		{
			if (CoreGuiService* coreGui = ServiceProvider::find<CoreGuiService>(
					dataModel))
			{
				shared_ptr<CoreGuiService> keep = shared_from(coreGui);
				coreGui->addChild(controlFrame.get());
			}
		}
		return controlFrame;
	}

	// mac: 0x39715A (via DataModel::startCoreScripts 0x2D617A).
	// Stats panels parented under DM+2968 guiRoot (setParentInternal shape);
	// mode-gated Visible + in-game path (chat/palette/LuaGui) TODO.
	// STUB: PLAYER 0x007f1e80
	// STUB: STUDIO 0x008b4c70
	void GuiBuilder::buildGui(Adorn* adorn, Workspace* ws, bool buildInGameGui)
	{
		(void)adorn;
		workspace = ws;  // gb+4.
		if (dataModel == NULL)
			return;
		Instance* root = dataModel->getGuiRoot();
		if (root == NULL)
			return;
		buildFPS()->setParent(root);
		buildCustomStats()->setParent(root);
		buildStatsHud1()->setParent(root);
		buildStatsHud2()->setParent(root);
		buildRenderStats()->setParent(root);
		buildNetworkStats()->setParent(root);
		buildNetworkStats2(true)->setParent(root);
		buildPhysicsStats()->setParent(root);
		buildPhysicsStats2()->setParent(root);
		buildSummaryStats()->setParent(root);
		if (buildInGameGui)
		{
			// Player path (binary order TODO -- this order follows the EA
			// mac: numbering: chatHud 0x3A1DB4, rightPalette 0x3A230A, luaGui
			// mac: 0x3A244C).
			// mac: TODO: chatMenu 0x3A1F62 (+4-arg 0x3A51A8), safeChat menu.
			buildChatHud()->setParent(root);
			buildRightPalette(adorn)->setParent(root);
			buildLuaGui();
		}
	}

} // namespace RBX
