/* Util/DFFlag.h — October 2013 (dynamic fast-flag STUB, small set).
   Evidence: setCoordinateFrameRoot 0x441A08 reads DFFlag::
     PromoteAssemblyModifications (F:\\2013Decompile\\exports\\decomp\\
     PartCFrame.txt). Zero DFFlag symbols in the Studio function list, and no
     DFFlag delivery system in the tree (2016-era dynamic flags absent —
     consistent with the handoff). Ported as a plain static bool (default
     false = direct-primitive path, matching all bring-up tests); dynamic
     delivery + default TODO-verify.
   GetAccurateScriptInfoResultsFromScriptContext: read by Script's EmbeddedSource
     ::embeddedCodeIsReady 0x1821EC (batch770:380) as a plain bool test feeding
     ScriptInformationProvider::getScriptInfo's accurateFlag (C1 C-lands as a
     static bool, same stub rationale).
   Original file: (2013 path TODO). */

#pragma once

namespace DFFlag {

	extern bool PromoteAssemblyModifications;
	extern bool GetAccurateScriptInfoResultsFromScriptContext;

} // namespace DFFlag
