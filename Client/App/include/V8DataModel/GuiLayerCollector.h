/* V8DataModel/GuiLayerCollector.h -- October 2013.
   Layout from the PLAYER ctor 0x008b4430 (STUDIO 0x00988c30): GuiBase2d (0x94), then the two layer
   grids at +0x94 (eh vector ctor, 0x10 each), each filled with 11 empty layers; ScreenGui's base
   members start at 0xb4. The element type is iOS-named: GuiLayerCollector::LoadZ(shared_ptr<Instance>,
   vector<vector<shared_ptr<GuiBase> > >&) 0x008b39f0. */

#pragma once

#include "V8DataModel/GuiBase2d.h"
#include "Gui/GuiEvent.h"

#include <vector>

namespace RBX {

	extern const char* const sLayerCollector;

	// VTABLE: PLAYER 0x00b86fec RBX::Debugable
	// VTABLE: STUDIO 0x00c4c02c RBX::Debugable
	// VTABLE: PLAYER 0x00b86fc0 RBX::GuiTarget
	// VTABLE: STUDIO 0x00c4c000 RBX::GuiTarget
	// VTABLE: PLAYER 0x00b86f8c RBX::IAdornable
	// VTABLE: STUDIO 0x00c4bfcc RBX::IAdornable
	// SIZE 0xb4
	class GuiLayerCollector
		: public DescribedNonCreatable<GuiLayerCollector, GuiBase2d, sLayerCollector, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		GuiLayerCollector(const char* name = NULL);
		~GuiLayerCollector();

	protected:
		typedef std::vector<shared_ptr<GuiBase> > GuiVector;
		typedef std::vector<GuiVector> GuiLayers;

		// mac: 0x6E5C74: 2 queues x 11 Z-layers, pre-sized empty (temp render
		// arrays — never realloc, always fast-clear, per 2016).
		GuiLayers guiLayers[GUIQUEUE_COUNT];  // 0x94  the ctor fills each with maxZIndex() + 1 empty layers
	};

	// Out-of-line copies the original emitted in this class's .obj (the Described ctor its ctor calls, the layer vector code).
	// clang-format off
	// TEMPLATE: PLAYER 0x008b3970
	// TEMPLATE: STUDIO 0x00988170
	// ??$?0PBD@?$Described@VGuiLayerCollector@RBX@@$E?sLayerCollector@2@3QBDBV?$NonFactoryProduct@VGuiBase2d@RBX@@$E?sLayerCollector@2@3QBDB@2@$0BL@$0A@@Reflection@RBX@@QAE@PBD@Z
	// TEMPLATE: PLAYER 0x008b3580
	// TEMPLATE: STUDIO 0x00987e50
	// ??$_Uninit_fill_n@PAV?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@IV12@V?$allocator@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@@2@@std@@YAXPAV?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@0@IABV10@AAV?$allocator@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@@0@U_Nonscalar_ptr_iterator_tag@0@U_Range_checked_iterator_tag@0@@Z
	// TEMPLATE: PLAYER 0x008b4040
	// TEMPLATE: STUDIO 0x00988840
	// ?_Insert_n@?$vector@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@V?$allocator@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@@2@@std@@IAEXV?$_Vector_const_iterator@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@V?$allocator@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@@2@@2@IABV?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@2@@Z
	// TEMPLATE: PLAYER 0x008b4310
	// TEMPLATE: STUDIO 0x00988b10
	// ??1?$vector@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@V?$allocator@V?$vector@V?$shared_ptr@VGuiBase@RBX@@@boost@@V?$allocator@V?$shared_ptr@VGuiBase@RBX@@@boost@@@std@@@std@@@2@@std@@QAE@XZ
	// clang-format on

} // namespace RBX
