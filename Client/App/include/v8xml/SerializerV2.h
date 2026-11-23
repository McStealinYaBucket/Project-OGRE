/* v8xml/SerializerV2.h -- the October 2013 XML archive reader/writer front end.
   Names from the iOS 10-29 / Mac 10-23 symbols (global namespace, no RBX::). The two
   IReferenceBinder implementations (ArchiveBinder, MemoryBinder) are classes local to
   Client/App/v8xml/SerializerV2.cpp in 2013: their inline bodies carry SerializerV2.cpp asserts
   (MemoryBinder::announceIDREF :50, ArchiveBinder::processID :116, resolveIDREF :137-142).
   load(XmlElement*, DataModel*) is static: PLAYER 0x006fcb70 is cdecl and never reads this. */

#pragma once

#include <istream>
#include <string>

#include "V8Xml/XmlElement.h"
#include "V8Tree/Instance.h"
#include "V8Xml/Reference.h"
#include "Util/Object.h"

namespace RBX {
	class DataModel;
}

class SerializerV2 {
protected:
	int schemaVersionLoading;	// 0x00
public:
	static const int CURRENT_SCHEMA_VERSION = 4;

	static XmlElement* newRootElement();
	static XmlElement* newRootElement(const std::string& type);

	void loadInstancesFromText(const XmlElement* root, RBX::Instances& result);
	void loadInstancesFromMemory(const XmlElement* root, RBX::Instances& result, RBX::CreatorRole creatorRole);

	static void load(XmlElement* root, RBX::DataModel* dataModel);
	void load(std::istream& stream, RBX::DataModel* dataModel);

	void loadInstances(const XmlElement* root, RBX::Instances& result, RBX::IReferenceBinder& binder, RBX::CreatorRole creatorRole);
	shared_ptr<RBX::Instance> loadInstance(const XmlElement* itemElement, RBX::IReferenceBinder& binder, RBX::CreatorRole creatorRole);

private:
	void loadXML(std::istream& stream, RBX::DataModel* dataModel);
};

// Bodies SerializerV2.obj emits for the file-local binders: their compiler-generated ctors and
// dtors, and the STL code for MemoryBinder's map<InstanceHandle, InstanceHandle> and vector of
// IDREFItem and ArchiveBinder's list of IDREFBinding. Named by our PUBLICS symbol (no oracle
// names); each is byte-identical to PLAYER under tools/probe_compare.py. Where VC9's /OPT:ICF
// folds identical bodies, the MemoryBinder instantiation stands for its MergeBinder twin.
// clang-format off

// list<IDREFBinding>::_Tidy. /OPT:ICF folds it with list<AsyncRetryTask>::_Tidy and boost iostreams'
// list<linked_streambuf*>::_Tidy; our PDB keeps this name for the folded body. Pairs once
// reccmp/player-libdata.csv is regenerated (its 0x006fa690 row still names linked_streambuf).
// TEMPLATE: PLAYER 0x006fa690
// ?_Tidy@?$list@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@std@@IAEXXZ

// TEMPLATE: PLAYER 0x006fa580
// ?_Rrotate@?$_Tree@V?$_Tmap_traits@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VInstanceHandle@RBX@@U?$less@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@V?$allocator@U?$pair@$$CBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VInstanceHandle@RBX@@@std@@@2@$0A@@std@@@std@@IAEXPAU_Node@?$_Tree_nod@V?$_Tmap_traits@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VInstanceHandle@RBX@@U?$less@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@V?$allocator@U?$pair@$$CBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@VInstanceHandle@RBX@@@std@@@2@$0A@@std@@@2@@Z

// TEMPLATE: PLAYER 0x006fa5e0
// ?_Max@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@KAPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@2@PAU342@@Z

// map<string,InstanceHandle>::_Max (PLAYER 0x006fa600): our link folds it into the
// map<string,shared_ptr<Script>> copy, so it pairs through the CSV row, not a marker.

// TEMPLATE: PLAYER 0x006fa620
// TEMPLATE: STUDIO 0x00748d80
// ?_Buynode@?$list@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@std@@IAEPAU_Node@?$_List_nod@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@2@XZ

// TEMPLATE: PLAYER 0x006fa640
// TEMPLATE: STUDIO 0x00748da0
// ??$_Count_if@V?$_Iterator@$0A@@?$list@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@std@@V?$binder1st@V?$mem_fun1_t@_NVArchiveBinder@@UIDREFBinding@1@@std@@@3@@std@@YAHV?$_Iterator@$0A@@?$list@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@0@0V?$binder1st@V?$mem_fun1_t@_NVArchiveBinder@@UIDREFBinding@1@@std@@@0@@Z


// TEMPLATE: PLAYER 0x006fa980
// TEMPLATE: STUDIO 0x00749090
// ??$_Destroy_range@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@YAXPAUIDREFItem@MemoryBinder@@0AAV?$allocator@UIDREFItem@MemoryBinder@@@0@U_Nonscalar_ptr_iterator_tag@0@@Z

// TEMPLATE: PLAYER 0x006fa9d0
// TEMPLATE: STUDIO 0x007490e0
// ?erase@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@@Z

// TEMPLATE: PLAYER 0x006faca0
// TEMPLATE: STUDIO 0x007493b0
// ?_Erase@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@IAEXPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@2@@Z

// TEMPLATE: PLAYER 0x006face0
// TEMPLATE: STUDIO 0x007493f0
// ?_Destroy@?$vector@UIDREFItem@MemoryBinder@@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@IAEXPAUIDREFItem@MemoryBinder@@0@Z

// TEMPLATE: PLAYER 0x006fad00
// TEMPLATE: STUDIO 0x00749410
// ?erase@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@0@Z

// TEMPLATE: PLAYER 0x006fae10
// TEMPLATE: STUDIO 0x00749520
// ?_Tidy@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@IAEXXZ

// SYNTHETIC: PLAYER 0x006faeb0
// SYNTHETIC: STUDIO 0x00749620
// ??0ArchiveBinder@@QAE@XZ

// TEMPLATE: PLAYER 0x006fafa0
// TEMPLATE: STUDIO 0x007496b0
// ?_Buynode@?$list@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@std@@IAEPAU_Node@?$_List_nod@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@2@PAU342@0ABUIDREFBinding@ArchiveBinder@@@Z

// TEMPLATE: PLAYER 0x006faff0
// TEMPLATE: STUDIO 0x00749700
// ?_Incsize@?$list@UIDREFBinding@ArchiveBinder@@V?$allocator@UIDREFBinding@ArchiveBinder@@@std@@@std@@IAEXI@Z

// SYNTHETIC: PLAYER 0x006fb460
// SYNTHETIC: STUDIO 0x007499f0
// ??1ArchiveBinder@@UAE@XZ

// SYNTHETIC: PLAYER 0x006fb500
// SYNTHETIC: STUDIO 0x00749a90
// ??_GArchiveBinder@@UAEPAXI@Z

// SYNTHETIC: PLAYER 0x006fb6e0
// SYNTHETIC: STUDIO 0x00749c70
// ??0MemoryBinder@@QAE@XZ

// TEMPLATE: PLAYER 0x006fb7b0
// TEMPLATE: STUDIO 0x00749d40
// ?_Lbound@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@IBEPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@2@ABVInstanceHandle@RBX@@@Z

// TEMPLATE: PLAYER 0x006fb7f0
// TEMPLATE: STUDIO 0x00749d80
// ??0IDREFItem@MemoryBinder@@QAE@ABU01@@Z

// TEMPLATE: PLAYER 0x006fb820
// TEMPLATE: STUDIO 0x00749db0
// ??0_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@QAE@PAU012@00ABU?$pair@$$CBVInstanceHandle@RBX@@V12@@2@D@Z

// TEMPLATE: PLAYER 0x006fb890
// TEMPLATE: STUDIO 0x00749e20
// ??$_Copy_backward_opt@PAUIDREFItem@MemoryBinder@@PAU12@Urandom_access_iterator_tag@std@@@std@@YAPAUIDREFItem@MemoryBinder@@PAU12@00Urandom_access_iterator_tag@0@U_Nonscalar_ptr_iterator_tag@0@U_Range_checked_iterator_tag@0@@Z

// TEMPLATE: PLAYER 0x006fb940
// TEMPLATE: STUDIO 0x00749ed0
// ??$_Uninit_copy@PAUIDREFItem@MemoryBinder@@PAU12@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@YAPAUIDREFItem@MemoryBinder@@PAU12@00AAV?$allocator@UIDREFItem@MemoryBinder@@@0@U_Nonscalar_ptr_iterator_tag@0@U_Range_checked_iterator_tag@0@@Z

// TEMPLATE: PLAYER 0x006fb990
// TEMPLATE: STUDIO 0x00749f20
// ?_Buynode@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@IAEPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@2@PAU342@00ABU?$pair@$$CBVInstanceHandle@RBX@@V12@@2@D@Z

// TEMPLATE: PLAYER 0x006fb9d0
// TEMPLATE: STUDIO 0x00749f60
// ??$_Copy_opt@PAUIDREFItem@MemoryBinder@@PAU12@Urandom_access_iterator_tag@std@@@std@@YAPAUIDREFItem@MemoryBinder@@PAU12@00Urandom_access_iterator_tag@0@U_Nonscalar_ptr_iterator_tag@0@U_Range_checked_iterator_tag@0@@Z

// TEMPLATE: PLAYER 0x006fba80
// TEMPLATE: STUDIO 0x0074a010
// ??$_Uninit_fill_n@PAUIDREFItem@MemoryBinder@@IU12@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@YAXPAUIDREFItem@MemoryBinder@@IABU12@AAV?$allocator@UIDREFItem@MemoryBinder@@@0@U_Nonscalar_ptr_iterator_tag@0@U_Range_checked_iterator_tag@0@@Z

// TEMPLATE: PLAYER 0x006fbad0
// TEMPLATE: STUDIO 0x0074a060
// ??$_Fill@PAUIDREFItem@MemoryBinder@@U12@@std@@YAXPAUIDREFItem@MemoryBinder@@0ABU12@@Z

// TEMPLATE: PLAYER 0x006fbb60
// TEMPLATE: STUDIO 0x0074a0f0
// ?_Insert@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@IAE?AViterator@12@_NPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@2@ABU?$pair@$$CBVInstanceHandle@RBX@@V12@@2@@Z

// TEMPLATE: PLAYER 0x006fbd60
// TEMPLATE: STUDIO 0x0074a2f0
// ??$_Unchecked_move_backward@PAUIDREFItem@MemoryBinder@@PAU12@@stdext@@YAPAUIDREFItem@MemoryBinder@@PAU12@00@Z

// TEMPLATE: PLAYER 0x006fbd90
// TEMPLATE: STUDIO 0x0074a320
// ?_Ufill@?$vector@UIDREFItem@MemoryBinder@@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@IAEPAUIDREFItem@MemoryBinder@@PAU34@IABU34@@Z

// TEMPLATE: PLAYER 0x006fbdd0
// TEMPLATE: STUDIO 0x0074a360
// ?insert@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@QAE?AU?$pair@Viterator@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@_N@2@ABU?$pair@$$CBVInstanceHandle@RBX@@V12@@2@@Z

// TEMPLATE: PLAYER 0x006fbe90
// TEMPLATE: STUDIO 0x0074a420
// ?insert@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@ABU?$pair@$$CBVInstanceHandle@RBX@@V12@@2@@Z

// TEMPLATE: PLAYER 0x006fc050
// TEMPLATE: STUDIO 0x0074a5e0
// ??$_Umove@PAUIDREFItem@MemoryBinder@@@?$vector@UIDREFItem@MemoryBinder@@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@IAEPAUIDREFItem@MemoryBinder@@PAU23@00@Z

// TEMPLATE: PLAYER 0x006fc080
// TEMPLATE: STUDIO 0x0074a610
// ??A?$map@VInstanceHandle@RBX@@V12@U?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@V12@@std@@@4@@std@@QAEAAVInstanceHandle@RBX@@ABV23@@Z

// TEMPLATE: PLAYER 0x006fc150
// TEMPLATE: STUDIO 0x0074a6e0
// ?_Insert_n@?$vector@UIDREFItem@MemoryBinder@@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@IAEXV?$_Vector_const_iterator@UIDREFItem@MemoryBinder@@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@2@IABUIDREFItem@MemoryBinder@@@Z

// TEMPLATE: PLAYER 0x006fc6b0
// TEMPLATE: STUDIO 0x0074ac40
// ?push_back@?$vector@UIDREFItem@MemoryBinder@@V?$allocator@UIDREFItem@MemoryBinder@@@std@@@std@@QAEXABUIDREFItem@MemoryBinder@@@Z

// SYNTHETIC: PLAYER 0x006fc8d0
// SYNTHETIC: STUDIO 0x0074ae60
// ??1MemoryBinder@@UAE@XZ

// SYNTHETIC: PLAYER 0x006fc970
// SYNTHETIC: STUDIO 0x0074af00
// ??_EMemoryBinder@@UAEPAXI@Z
// clang-format on
