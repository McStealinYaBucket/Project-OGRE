# Phase 5c, first pass: serialization (v8xml)

Agent label `5c-serialization`, 2026-11-23 06:48–08:45. Builds: `build-xml/` (PLAYER), `build-xml-studio/`
(STUDIO). Final scored runs: `build-xml/runs/P11.json`, `build-xml-studio/runs/S7.json`; the exe/pdb/map they
scored are copied to `build-*/runs/scored/`. Shared `reccmp/` CSVs were used as they are (not regenerated).

## Row 5c

`tools/row_coverage.py --row 5c` (new row). `.obj` ranges from the 2013 `__FILE__` strings, the binary
deserializer's strings and the static-initializer/template evidence below:

| File | PLAYER range | sweep10 | now |
|---|---|---|---|
| `v8xml/XmlElement.cpp` | 0x00613c70–0x00614a30 | 14/25 | 17/25 |
| `v8xml/XmlSerializer.cpp` | 0x00627b80–0x00629630 | 3/21 | 17/21 |
| `v8xml/SerializerV2.cpp` | 0x006fa580–0x006fd510 | 14/60 | 31/60 |
| `v8xml/SerializerBinary.cpp` | 0x008cec50–0x008d2510 | 0/43 | 7/43 |
| `v8xml/WebParser.cpp` | 0x008b0f20–0x008b1500 | 1/2 | 1/2 |
| `v8xml/WebSerializer.cpp` | 0x0081f410 | 0/1 | 0/1 |
| **Total** | | **32/152 = 21.1%** | **73/152 = 48.0%** |

CSVs: `5c-coverage-before.csv` (sweep10 `build/sweep10/P-final.json`), `5c-coverage-after.csv` (P11).
Whole exe in my builds: PLAYER 9,068 effective 100% (sweep10 9,029), STUDIO 8,852 (8,826). Those totals
include other wave-4 agents' uncommitted work in the shared tree. Every loss between my runs was read with
`--verbose`. None came from v8xml: they were `shared_ptr<X>::reset` twin churn from the RunService/GuiItem
markers, the Lighting edits, and no-`.reloc` immediates (`IsOlderOrderedPacket`, `httpGetPostWinHttp`
against a moved COL address). The one row entry lost against sweep10 is 0x006fa690 (`list<…>::_Tidy`):
my new `list<IDREFBinding>::_Tidy` folds with it, our PDB now keeps the IDREFBinding name for the folded
body, and the stale `player-libdata.csv` row still asks for `list<linked_streambuf*>::_Tidy`. It is
twin-pairing churn and needs the CSV regeneration. The `TEMPLATE` marker in `SerializerV2.h` names the
current PUBLICS symbol.

Every `// FUNCTION:` marker in my files is effective 100% in the final runs, and no `// STUB:` is (checked
for both targets). `check_2016_copies.py` reports nothing. `reccmp_addr_padding.py` is clean. decomplint
reports only three descents, all pinned by asserts (see the end of this file).

## Layouts proven (by 100% functions)

- **XmlNameValuePair** 0x10, 8-byte aligned (the union holds a double): tag ref +0, valueType +4, value +8.
  **XmlAttribute** 0x18: Sibling `next` +0, the pair at +8 (alignment, not a hidden member). **XmlElement**
  0x28: Sibling +0, Parent first/last +4/+8, pair +0x10, attribute Parent +0x20/+0x24. Proven by the
  getValue family, findAttribute/findFirstChildByTag, isValueEqual, writeOpenTag and serialize.
- **XmlWriter** (novtable, noncopyable): `map<InstanceHandle,int> handles` +4, `ostream& stream` +0x10,
  nothing else. 2016's `idValidationMap`/`recordId` did not exist in 2013 (ctor 0x00628a20).
- **XmlParser**: vptr, `streambuf* buffer` +4, `std::stack<XmlElement*>` +8. The vftable has one slot, the
  pure `auto_ptr<XmlElement> parse()`, and there is no virtual dtor (ctor 0x00628980). **TextXmlParser** adds
  `map<string,string> legacyHashes` at +0x28 (inline ctor, shell copy 0x00402c00). The prior tree's
  `TextXmlParser.h` (raw `XmlElement*` parse, deque, virtual dtor) was replaced. `SafeChat.cpp` and
  `ContentProvider.cpp` compile unchanged because they already wrap `parse()` in an `auto_ptr`.
- **MemoryBinder** 0x20: `map<InstanceHandle,InstanceHandle>` +4, `vector<IDREFItem>` +0x10. Ctor
  0x006fb6e0 is 100%, dtor 0x006fc8d0 is byte-identical. It defers every IDREF and remaps it in resolveRefs.
- **ArchiveBinder : MergeBinder** 0x2c: MergeBinder's vector +4, `map<string,InstanceHandle>` +0x14,
  `list<IDREFBinding{valueIDREF, propertyOwner, idref}>` +0x20. Both binders are classes local to
  `SerializerV2.cpp`, as in 2013 (their inline bodies carry the asserts at :50, :116 and :137–142). The
  vftables have six slots: announceID, announceIDREF, resolveRefs, dtor, processID, processIDREF.
- **SerializerV2** `{int schemaVersionLoading}`. `load(XmlElement*, DataModel*)` is static: 0x006fcb70 is
  cdecl.
- **MemoryInputStream** `{data, offset, size}`. `read` 0x008cec50 throws
  `RBX::runtime_error("Read offset is out of bounds while reading %d bytes", n)`.
- **XmlElement.obj tag globals**: 56 `Name::declare`/`lookup` initializers from PLAYER 0x00a63d30. Their
  order matches our definition order exactly, without 2016's six CustomPhysics tags (removed). All 56 have
  `GLOBAL` markers for both targets, so `tag_class`, `tag_Item` and the rest now pair everywhere.
- `whitespace[256]` (0x00d30dd0 / STUDIO 0x00ebd608) is a statically initialised `.data` table: tab, LF, FF,
  CR and space. No dynamic initializer writes it.

## Functions raised to 100% (PLAYER; STUDIO verified the same unless noted)

- **XmlElement.cpp**: isValueEqual(const Name*) 0x00613c70 (new to the row), getValue(const Name*&)
  0x00613e70, and the 14 functions that were already at 100% through CSV pairing now carry markers. The file
  is in address order.
- **XmlSerializer.cpp** (parser and writer rewritten from the bytes): skipWhitespace, readFirstTag, readTag,
  findNextToken (static, custom registers), decodeString, readText, removeTag (a member that never reads
  `this`), encodedWrite, writeCloseTag, writeOpenTag, XmlParser ctor, XmlWriter ctor, serialize(node, depth),
  parseAttributes. That is 14 new; serialize(node) was already at 100%.
- **SerializerV2.cpp**: ArchiveBinder::resolveIDREF, resolveRefs, processIDREF; MemoryBinder ctor,
  announceIDREF, processIDREF; loadInstancesFromText, loadInstancesFromMemory. Template bodies, with
  `TEMPLATE`/`SYNTHETIC` markers in `SerializerV2.h`: `_Count_if`, list `_Buynode`/`_Incsize`/`_Buynode()`,
  map `erase`, `_Node` ctor, `_Uninit_fill_n`, `_Fill`, `_Ufill`, vector push_back.
- **SerializerBinary.cpp**: MemoryInputStream::read; readString (iOS name, static, in edi);
  readFormatExpected; the PROP-chunk helper 0x008d1310 (`FUN_008d1310`: readString, then
  `findPropertyDescriptor`, then readPropertyValues); deserialize(istream&, Instance*) 0x008d2510; and two
  `vector<vector<Instance*>>` template bodies.
- The map<InstanceHandle,int> bodies and `XmlWriter::getHandleIndex` behind `XmlNameValuePair::toString`
  are byte-identical (TEMPLATE markers in `TextXmlWriter.h`) but score 96–99%. See the blockers below.

Codegen facts learned (candidates for CLAUDE.md):
- VC9 keeps a separately named local in registers differently. `parseAttributes` matched only with a
  separate `const int openQuote = equals + 1` (an in-place `++equals` permutes ebx/esi/edi/ebp).
- An if/else-if chain gets one destructor copy per branch (decodeString). `continue` inverts the block
  order.
- `if (!version || !version->getValue(...)) throw …; else if (…) throw …; else …` keeps the redundant null
  test and changes block placement and EH state numbering (loadXML).
- `readFirstTag`'s BOM message is a named `std::string` local, so its stack slot differs from the other two
  throws.

## What blocks the rest

1. **ICF twins between the LTCG shell and App.** The originals of MergeBinder (0x00447xxx), its
   vector<IDREFItem> helpers, `shared_ptr` operator=/dtor (0x00469520, 0x00405b50), the TextXmlParser
   ctor/dtor (0x00402c00/0x00404e10) and `~XmlElement`/its pool free (0x00402350/0x00409ee0) are client-shell
   copies. Ours fold into one native body. This keeps these at 94–99% even though every byte matches:
   MemoryBinder::resolveRefs/processID, loadInstance, loadXML, load(XmlElement*), ArchiveBinder ctor/dtor,
   getValue(InstanceHandle&), toString, WebParser::loadValue (99.3%), and the vector<IDREFItem> template
   bodies (several show "-": paired to the shell address). In the 2013 binary MergeBinder is defined in
   **`v8xml/SerializerV2.h`** (assert `processedIDREF` at SerializerV2.h:76, reached through a header in the
   same directory, hence the lowercased path). It is not in `Reference.h`.
2. **InstanceHandle::operator<** (0x006fa4c0) folds with `std::_String_const_iterator::operator<` in our link
   (probably from GfxBase's `/Od` objects). This affects the map<InstanceHandle,*> template bodies and
   getHandleIndex.
3. **serializeNode** 0x00628a60 (72.6%) needs a public accessor for ContentId's Name pointer
   (`Util/ContentId.h`, outside my files). Its body is otherwise worked out (notes in the file).
4. **TextXmlParser::parse** 0x00628f70 (68.6%) has the right structure and length (1,722 B), but VC9 keeps
   `element` in edi and gives the deque temporary its own slot. Ours spills differently, a 4-byte bigger
   frame. Unresolved after six variants.
5. **SerializerV2::load(istream&)** 0x006fd070 (28.6%) is fully read: an `RBX_` int compare, peek loop,
   `char name[33]`, and an inline memcmp against kMagicHeader. It needs ContentProvider::readContent's 2013
   signature, which returns a value the caller discards (`V8DataModel/ContentProvider.h`, not mine).
6. **loadInstances** calls `G3D::Log::common()->println`. The Rendering G3D fork has no Log, so it is
   declared locally in SerializerV2.cpp and stays unresolved at link time. It is byte-identical but
   unpaired.
7. **SerializerBinary**: 36 functions are left. iOS 10-29 names readIntVector, readFloatVector,
   readUIntVector, readIdVector and readData (ours are inlined `read*Array` helpers), plus readPropertyValues
   (5,546 B) and deserializeImpl (1,109 B). PLAYER's PROP handler resolves a property by its plain name
   through `findPropertyDescriptor` (trie), not by our "Class-Prop" loop. The writer side
   (serialize/writeImpl) exists only in STUDIO (0x00745520, …).

## Unresolved (for other owners)

- `Client/Rendering` (other contributor): G3D 8.00 `G3D/Log.h` (`Log::common`, `println`) for
  `SerializerV2::loadInstances` 0x006fc990.
- `Util/ContentId.h`: a public accessor for the Name pointer at +0x1c, for serializeNode.
- `V8DataModel/ContentProvider.h`: readContent returns a value in 2013 (load(istream&) 0x006fd070).
- `Util/Handle.h`/`Handle.cpp`: 2013 has `InstanceHandle(DescribedBase*)` out of line at 0x006fa560 and
  `InstanceHandle(shared_ptr)` at 0x007a8a20 (the latter is used by MemoryBinder::processID).
- `Util/Name.cpp`: `Name::lookup(const char*)` is PLAYER 0x00613be0, the last function before XmlElement.obj.
- Client shell: instantiate MergeBinder (plus TextXmlParser and XmlElement's inline dtor) in the
  `/GL` set, so the shell copies get their own bodies. Move MergeBinder from `Reference.h` to
  `SerializerV2.h` (line 76 for announceIDREF's assert) when that is done.
- `xmlelement.h` still declares the six 2016 `tag_custom*` externs. They have no definitions and no users.
  They come out the next time the header is touched (that rebuilds 270 TUs).
- Owner sweep: regenerate `reccmp/*.csv` so 0x006fa690 and the template rows resolve against the new
  symbols.

## Descents kept (assert-pinned)

`SerializerV2.cpp`: MemoryBinder (assert at :50) comes before ArchiveBinder (:116, :137) in source, but its
bodies are at higher addresses. `XmlSerializer.cpp`: findNextToken (:310) follows removeTag (:291/:299) in
source but is emitted at 0x00627f90.

## Files

Changed: `Client/App/v8xml/{XmlElement,XmlSerializer,SerializerV2,SerializerBinary}.cpp`,
`Client/App/include/v8xml/{TextXmlParser,TextXmlWriter,XmlSerializer,SerializerV2,SerializerBinary}.h`,
`tools/row_coverage.py` (row 5c). No commits.
