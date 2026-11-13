# gui-5h (workflow phase5-wave2, wf_cba1073c-3f1)

PHASE 5h (GUI), resumed attempt. I kept and measured the first attempt's GUI edits, then added layout proofs and new functions. Measured with fresh builds in build-gui/ and build-gui-studio/: final JSONs are in scratchpad/gui2/P4.json and S4.json, compared against build/sweep7/P-final.json and build-studio/sweep7/S-final.json. Those builds include the other agents' uncommitted edits.

**Row 5h coverage (PLAYER functions at effective 100%)**
- Same 1,304-function set as the baseline: 51 → 72.
- tools/row_coverage.py --row 5h, using its own ranges: 51/1304 (3.9%) before → 74/1274 (5.8%) after.
- The per-function list with scores is in scratchpad/gui2/5h-P4.csv; the baseline list is 5h-base.csv.
- GuiService: 246 functions in its range, 6 at 100%. Its ctor 0x0069ed90 is not done.

**Whole tree, all agents' edits together**
- PLAYER effective 100%: 5972 → 6134 (non-library 3790 → 3957).
- STUDIO effective 100%: 5848 → 6010 (non-library 3744 → 3904).
- My files now carry 27 PLAYER and 26 STUDIO `FUNCTION` markers (one FOLDED each), every one measured at 100%.
- check_folded: 0 problems on both targets. Address padding: 0 bad.
- decomplint: I fixed the GuiDrawImage.cpp order error and the duplicate 0x00927cc0 marker in GuiBase2d.h.

**Layouts proven**
- At 100% in reccmp:
  - GuiBase, SIZE 0x7c: ctor 0x007bc4b0.
  - GuiBase2d, SIZE 0x94: ctor 0x007bcc40.
  - Frame, SIZE 0x2a4: ctor 0x006c2b70.
  - GuiLabel: ctor 0x007cb640.
  - GuiDrawImage, SIZE 0x84: 0x006bc570.
  - New GuiImageMixin, SIZE 0xb8, in GuiMixin.h: ctor 0x006bc910. It has a vftable of 3 pure setters (0x00b3a604); ImageLabel's override table 0x00b3a96c is now at 100%.
  - New GuiTextMixin, SIZE 0x54, in GuiText.h: ctor 0x006be930.
  - ScreenGui, SIZE 0xec, with a HeartbeatInstance base at +0xb4: ScreenGui(name) 0x008b2ae0 and onHeartbeat 0x008b1e20 at 100% on PLAYER. ScreenGui(name) is 96.4% on STUDIO.
- Byte-identical in a direct obj-vs-binary compare (tools/probe_compare.py), but reccmp only gives 93.8–99.1% because some calls and vftables do not pair yet:
  - TextLabel, SIZE 0x270: ctor 0x006c1c10.
  - ImageLabel, SIZE 0x2d4: ctor 0x006bdb40.
  - GuiLayerCollector, SIZE 0xb4: ctor 0x008b4430.
  - ScreenGui(): 0x008b2980.
- Not proven: GuiObject. Its ctor is at 48.4% and no dtor or scalar deleting dtor is written. GuiService is not started.

**New functions written from the 2013 bytes, at 100% in both targets**
- Frame::getChildRect2D 0x006c1dd0, with its assert on line 43 as the 2013 string records.
- GuiObject::setClipping 0x007bd3a0, setBorderSizePixel 0x007bd400, setVisible 0x007bd540.
- FASTFLAGVARIABLE BorderSizePixelUpTo100 and four GuiObject prop descriptors, with GLOBAL markers.

**Raised to 100%**
- setBackgroundTransparency 0x007bd610 (was 47%), getRect2D, handleResize, setAbsolutePosition/Size.
- RecursiveRenderChildren, ResizeChildren, askAddChild, GuiObject::handleResize.

**Also written, still below 100%**
- GuiObject::Scale9Rect2D 0x007bf190 (98.2%) and ~GuiLayerCollector 0x008b4350 (80%).
- ImageLabel's three setters (62–78%): they do not yet raise their properties.

**Marker fixes that made reccmp pair**
- A GLOBAL on a constructor-style definition needs a nameref comment line under the marker; reccmp's parser then adds the enclosing namespace itself.
- Added TEMPLATE markers that spell the mangled symbol, for the out-of-line Described ctors, getset, visitChildren, Rect2D::xywh and the classDescriptor at 0x00541290.
- Added the missing GuiTarget/IAdornable VTABLE markers.

**Lost 100% rows in my area**
Each one is CSV twin-naming churn rather than a code difference. I checked each with --verbose:
- FactoryProduct<Frame> vftables 0x00b3b87c/0x00b3b88c: the libdata rows at 0x006c2870/0x006c2880 name NotificationObject's thunks; they should be FactoryProduct<Frame>'s.
- GuiMain vftables 0x00b8670c/71c/944/954: the row at 0x008b2ea0 names DescribedCreatable<ScreenGui>'s thunk; it should be GuiMain's.
- 0x007bc8f0 and Instance::getPersistentDataCost 0x0061a9a0: the libdata symbol should be the visitChildren<...PAVAdorn...> twin.
- 0x006bc510 isReadOnly twin, which also takes down about 15 TypedPropertyDescriptor vftables. This one was already lost in the first-attempt build before this session's changes.

## Files changed

- Client/App/include/V8DataModel/GuiMixin.h
- Client/App/include/V8DataModel/GuiText.h
- Client/App/include/V8DataModel/GuiBase.h
- Client/App/include/V8DataModel/GuiBase2d.h
- Client/App/include/V8DataModel/GuiObject.h
- Client/App/include/V8DataModel/Frame.h
- Client/App/include/V8DataModel/GuiLabel.h
- Client/App/include/V8DataModel/TextLabel.h
- Client/App/include/V8DataModel/ImageLabel.h
- Client/App/include/V8DataModel/ScreenGui.h
- Client/App/include/V8DataModel/GuiLayerCollector.h
- Client/App/include/V8DataModel/TextService.h
- Client/App/include/Gui/GuiEvent.h
- Client/App/include/Gui/GuiDrawImage.h
- Client/App/Gui/GuiDrawImage.cpp
- Client/App/v8datamodel/GuiBase.cpp
- Client/App/v8datamodel/GuiBase2d.cpp
- Client/App/v8datamodel/GuiObject.cpp
- Client/App/v8datamodel/Frame.cpp
- Client/App/v8datamodel/GuiLabel.cpp
- Client/App/v8datamodel/TextLabel.cpp
- Client/App/v8datamodel/ImageLabel.cpp
- Client/App/v8datamodel/ScreenGui.cpp
- Client/App/v8datamodel/GuiLayerCollector.cpp

## Unresolved

- CSV rows for the Integrate agent, from reccmp/player-libdata.csv. These rows went stale once the GUI classes got real layouts. Regenerate them, or name the addresses as follows: 0x006c2870/0x006c2880 as ??_E?$FactoryProduct@VFrame@...OBI@/OBM@ (not NotificationObject's); 0x008b2ea0 as GuiMain's DescribedCreatable thunk; 0x007bc8f0 as ??$visitChildren@V?$bind_t@XP6AXV?$shared_ptr@VInstance@RBX@@@boost@@PAVAdorn@RBX@@@Z... (the GuiBase2d copy; this also restores Instance::getPersistentDataCost 0x0061a9a0); 0x006bc510 as whichever folded isReadOnly alias our PUBLICS keeps. A TEMPLATE marker cannot override a symbol row on the same address: both stop pairing, which I measured at 0x007bc8f0.
- More pairing rows that would take TextLabel ctor 0x006c1c10, ImageLabel ctor 0x006bdb40, ScreenGui() 0x008b2980 and the GuiLayerCollector ctor from 93-99% to 100% (all byte-identical in a direct obj compare): the DescribedCreatable<TextLabel|ImageLabel> GuiTarget/IAdornable vftables 0x00b3b388/0x00b3b354/0x00b3a790/0x00b3a75c; BrickColor::color3 0x00792510; the ScreenGui float constants 800.0 at 0x00b25074 and 600.0 at 0x00b69450; G3D::Rect2D::xywh(float,float,float,float) 0x00491500 (Scale9Rect2D is 98.2% because of it).
- GfxBase IAdornable.h is not in my ownership. In 2013 IAdornable is 0x18 bytes, with a float at +0x14 that the GuiBase ctor zeroes before the -1 indices, and it has an out-of-line dtor at PLAYER 0x009ca5c0. GuiBase keeps a placeholder m_unk0x78 until IAdornable gets the float. ~GuiLayerCollector 0x008b4350 (80%) is blocked on that dtor.
- FastLogSettings.cpp, not mine, needs LOGVARIABLE(GuiTargetLifetime, 0) at PLAYER 0x00de3f47. GuiEvent.h now declares LOGGROUP(GuiTargetLifetime) for the GuiLayerCollector ctor/dtor FASTLOG. The data symbol stays unresolved under /FORCE:UNRESOLVED until it is defined.
- ImageLabel's setters 0x006bbe20/0x006bbe70/0x006bc280 need their property raises: prop_Image at 0x00dec104, and prop_ImageRectOffset/Size through a pointer at 0x00dec0e4/0x00dec0e8 with a null check. That needs the GuiImageMixin reflection block.
- GuiObject ctor 0x007c77c0 is at 48.4%: its event replicators and remote_signal members are placeholders. GuiObject's dtor (0x006bd630, inlined into the FactoryProduct sdds) and its reflection block (setSize/setPosition/setSizeConstraint use prop globals at 0x00e0f27c and others) are not written. GuiService (ctor 0x0069ed90, 381 B, with five signals and four maps) and BillboardGui (ctor 0x0090ffa0) are not started.
- GuiBase2d::recursiveRender2d 0x007bcd60 is at 95%: its visitChildren<RecursiveRenderChildren> callee is ICF-folded with the computeChildCost visitor at 0x007bc8f0, which is the CSV row in the first item.
- tools/probe_compare.py splits NAME@0xADDR on the first '@', so MSVC-mangled names cannot be passed. I used a scratch copy that splits on the last '@' (scratchpad/gui2/pc.py). Fixing it in tools/ is a one-line change for whoever owns that file.
- tools/row_coverage.py's 5h ranges are guesses. GuiButton's range overlaps GuiObject.cpp: 2013 GuiObject.cpp holds GuiButton code, per GuiObject.cpp asserts 556-1606. GUI.cpp's cluster also seeds on the wrong markers. Fixed ranges (FIXED) would make the 5h denominator stable.
