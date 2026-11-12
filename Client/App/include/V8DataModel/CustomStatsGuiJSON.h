/* V8DataModel/CustomStatsGuiJSON.h — October 2013 (minimal bring-up).
   Evidence: CustomStatsGuiJSON::DefaultHandler 0x39608E / WriteFile 0x396906 /
     dtor 0x3A598C (F:\2013Decompile\exports\decomp\GuiJSON_2013.txt),
     GuiBuilder::saveCustomStats 0x3A4F38 (builds a CustomStatsGuiJSON writer
     object on the stack + WriteFile).
      - DefaultHandler(name, text): creates an EquationDisplay (Creatable,
        held as shared_ptr<TextDisplay>) + inserts pair<string,
        GuiBuilder::Data>(name, text) into the GuiBuilder customStats map
        (registry at builder+16); returns true. EquationDisplay/TextDisplay
        GUI creation TODO (classes not in tree).
      - WriteFile: opens GetCustomStatsFilename() 0x395FB6 for output; writes
        "{\n" + per-entry `"key": "value",\n` (NO escaping) + "}". Only when
        the file opens.
      - Layout: GuiBuilder* at +28 (WriteFile reads *(this+7) as the builder).
   TODO(decompile): EquationDisplay/TextDisplay classes; handler registry
     semantics (the map<string, void(*)(const char*)> in saveCustomStats);
     exact +28 slot (MSVC layout differs).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CustomStatsGuiJSON.h */

#pragma once

#include <string>

namespace RBX {

	class GuiBuilder;

	class CustomStatsGuiJSON
	{
	public:
		explicit CustomStatsGuiJSON(GuiBuilder* builder)
			: builder(builder)
		{}

		virtual ~CustomStatsGuiJSON();

		// mac: 0x396906.
		void WriteFile();
		// mac: 0x39608E — default per-stat handler; returns true.
		virtual bool DefaultHandler(const std::string& name, const std::string& text);

	private:
		GuiBuilder* builder;  // +28 (TODO: exact slot)
	};

	// mac: 0x395FB6 (TODO: body — in IDA queue).
	void GetCustomStatsFilename(std::string& result);

} // namespace RBX
