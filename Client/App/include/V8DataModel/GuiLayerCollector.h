/* V8DataModel/GuiLayerCollector.h — October 2013 (gui render-order shell).
   Evidence: GuiLayerCollector C2 0x6E5C74 (GuiBase2d(name) + 2 rows x 11
     empty vector<shared_ptr<GuiBase>> at +144..168 + GuiTargetLifetime log;
     Described<GuiLayerCollector, sLayerCollector, GuiBase2d, 27>).
     sLayerCollector = "LayerCollector" (2016 GuiLayerCollector.cpp).
     2016 shape carried for the layer grid (GuiLayers mGuiVectors[2], temp
     render arrays, Z 0..10 per queue); TODO verify 2013 member roles.
   TODO: full decompile (render/input/process surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiLayerCollector.h */

#pragma once

#include "V8DataModel/GuiBase2d.h"

#include <vector>

namespace RBX {

	extern const char* const sLayerCollector;

	class GuiLayerCollector
		: public DescribedNonCreatable<GuiLayerCollector, GuiBase2d, sLayerCollector, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		GuiLayerCollector(const char* name = NULL);

	protected:
		typedef std::vector<shared_ptr<GuiBase> > GuiVector;
		typedef std::vector<GuiVector> GuiLayers;

		// mac: 0x6E5C74: 2 queues x 11 Z-layers, pre-sized empty (temp render
		// arrays — never realloc, always fast-clear, per 2016).
		GuiLayers guiLayers[GUIQUEUE_COUNT];
	};

} // namespace RBX
