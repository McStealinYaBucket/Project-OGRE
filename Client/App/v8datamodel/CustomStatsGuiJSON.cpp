/* App/v8datamodel/CustomStatsGuiJSON.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\GuiJSON_2013.txt, GuiJSON_file.txt)
      - DefaultHandler 0x39608E(name, text): creates an EquationDisplay via
        Creatable (held as shared_ptr<TextDisplay>) + inserts
        pair<string, GuiBuilder::Data>(name, text) into the GuiBuilder
        customStats registry (map at builder+16); returns true.
        EquationDisplay/TextDisplay GUI creation TODO (classes not in tree);
        the registry insert (the observable state) is exact.
      - WriteFile 0x396906: opens GetCustomStatsFilename() for output; writes
        "{\n", then per registry entry `"key": "value",\n` (NO escaping, and
        the trailing comma is written even for the last entry), then "}".
        Skips everything if the file won't open.
      - dtor 0x3A598C: vtable reset + registry-map erase (standard).
      - GetCustomStatsFilename 0x395FB6(out): FileSystem::getUserDirectory()
        + "StatDisplaySettings.json" (24 chars incl. nul per append count).
   TODO(decompile): EquationDisplay/TextDisplay; handler-registry semantics
     (named handlers + the map<string, void(*)(const char*)> in
     saveCustomStats); exact +28 builder slot.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CustomStatsGuiJSON.cpp */

#include "stdafx.h"

#include "V8DataModel/CustomStatsGuiJSON.h"
#include "V8DataModel/GuiBuilder.h"
#include "Util/FileSystem.h"

#include <fstream>

namespace RBX {

	CustomStatsGuiJSON::~CustomStatsGuiJSON()
	{
	}

	// STUB: PLAYER 0x007e4cc0
	// STUB: STUDIO 0x008a8870
	void GetCustomStatsFilename(std::string& result)
	{
		// mac: 0x395FB6.
		FileSystem::getUserDirectory(result);
		result += "StatDisplaySettings.json";
	}

	// STUB: PLAYER 0x007e66f0
	// STUB: STUDIO 0x008aa270
	bool CustomStatsGuiJSON::DefaultHandler(const std::string& name, const std::string& text)
	{
		// mac: 0x39608E: EquationDisplay/TextDisplay creation TODO (classes absent).
		if (builder != NULL)
			builder->customStatsMap()[name] = GuiBuilder::Data(text);
		return true;
	}

	// STUB: PLAYER 0x007e5710
	// STUB: STUDIO 0x008a9290
	void CustomStatsGuiJSON::WriteFile()
	{
		// mac: 0x396906.
		if (builder == NULL)
			return;
		std::string filename;
		GetCustomStatsFilename(filename);
		std::ofstream file(filename.c_str());
		if (!file.is_open())
			return;
		file.write("{\n", 2);
		const std::map<std::string, GuiBuilder::Data>& stats = builder->customStatsMap();
		for (std::map<std::string, GuiBuilder::Data>::const_iterator it = stats.begin();
			it != stats.end(); ++it)
		{
			file << "\"" << it->first << "\": \"" << it->second.text << "\",\n";
		}
		file.write("}", 1);
	}

} // namespace RBX
