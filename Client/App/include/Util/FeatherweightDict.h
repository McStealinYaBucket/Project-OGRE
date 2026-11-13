/* Util/FeatherweightDict.h — October 2013 flyweight ("featherweight") framework.
   SECOND PASS (binary reconstruction; Instance names get/set via FW since
   session 22; full interning semantics below).
   Evidence: F:\2013Decompile\exports\decomp\FW_core.cpp.txt (14 fns: init,
     FWBase ctor/eq, FWValue bool/string set, fw(), FWInstance ctor/eq/hash,
     insertCopy 0x6D328E, checkHash),
     F:\2013Decompile\exports\decomp\FW_part.txt (8 fns: FWPartInstance
     ctor 0x44AABA / eq 0x44AB3E / hash 0x44A9A7, registerFlyweight 0x44BCEC,
     valueUpdate 0x6D3058, checkIfInDict 0x6D33B0, destroyFlyweight 0x6D2FAC,
     PVInstance ctor 0x4687C8).
   Assert paths: "include/util/FeatherweightDict.h" (operator== :65, set :173,
     insertCopy :302, registerFlyweight :237/:244, valueUpdate :279,
     destroyFlyweight :259, erase :259) and "../App/include/util/..." (:173).
   Log channels FLog::FWLifetime + FLog::FWUpdate.
   There is NO March 2016 counterpart (whole system removed by 2016).

   Layout (Mach-O; MSVC sizes differ, ORDER preserved):
     FWBase: vftable(+0), zeros(+4/+8), holder(+12, shared_ptr<IFWHolder> —
       registerFlyweight assigns the dict (shared_from_this), valueUpdate's
       MISS path copies it with an incref), zero(+16), flag(+20, FWValue
       re-entrancy guard).
     FWInstance : FWBase (28 bytes Mach-O pool size): bytes +21/+22 = 0,
       +23 = 1, name at +24 (default "Instance"; Mach-O std::string is 4
       bytes — MSVC's is 28, so MSVC offsets differ; ORDER preserved).
     FWPVInstance : FWInstance, no payload (marker subclass for per-class
       dicts/inits; FWPartInstance fills the 56-byte pool exactly:
       28 + 28 payload).
     FWPartInstance : FWPVInstance (56 bytes Mach-O): +28 int 256,
       +32 int 194, +36/+40/+44 floats 0/0/1.0, +48 byte 0, +52 int 0.
       (+32 = 194 matches the default BrickColor number; +28 = 256 matches
       the default material — TODO: confirm identities at PartInstance
       decompile. Numeric field suffixes below are Mach-O offsets.)
     Instance+68 (this+17): shared_ptr<FWBase>; Instance::fw() returns raw.
     FWRef: a shared_ptr<FWBase> (px at +0 — the old "+0 = FWBase*" guess —
       with the shared_count at +4 — the old "+4 refcount-ish" guess).
       All three ref asserts read it: set :173 "base != NULL",
       registerFlyweight :237 and valueUpdate :279 "ref->fwPtr.get() != NULL".
     FWDictionnary<T>: unordered_set<DumbPtr<T>> at +4 (value hash + value
       equality — interning only works that way; DumbPtr::operator== is
       value-based), update counter at +28 (++ per valueUpdate).
   Ownership: dict sets hold RAW non-owning pointers; refs own through plain
     shared_ptr<FWBase>. Every flyweight is an FWFinal<T> pool object
     (Allocator<T>): its virtual destructor erases the dict entry, and the
     deleting destructor returns the chunk to the pool (2013, see init).
   DIVERGENCE from 2016: none — system absent there. */

#pragma once

#include <string>
#include <cstring>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>

#include "rbx/memory.h"
#include "rbx/Debug.h"
#include "Util/BrickColor.h"
#include "Util/Material.h"

LOGGROUP(FWLifetime)
LOGGROUP(FWUpdate)

namespace RBX {

	class FWBase;
	class FWRef;
	class FWDictionnaryBase;

	// Raw-pointer entry for flyweight dict sets (mirrors the
	// DumbPtr<FWPartInstance> unordered_set in insertCopy 0x6D328E).
	// Equality is VALUE-based (*ptr == *other.ptr) — interning requires it
	// (registerFlyweight's dup path exists, so lookup must match by value).
	template<class T>
	class DumbPtr
	{
	public:
		T* ptr;
		DumbPtr() : ptr(NULL) {}
		explicit DumbPtr(T* p) : ptr(p) {}
		bool operator==(const DumbPtr& other) const
		{
			return ptr == other.ptr
				|| (ptr != NULL && other.ptr != NULL && *ptr == *other.ptr);
		}
		bool operator!=(const DumbPtr& other) const { return !(*this == other); }
	};

	// Value hash over the pointed-to flyweight (consistent with ==).
	template<class T>
	struct FWDumbHash
	{
		size_t operator()(const DumbPtr<T>& d) const
		{
			// 2013 FW classes have no hash virtual (FWInstance's vftable 0x00b2a234 holds only
			// the deleting destructor): the free hash_value overloads, found at instantiation.
			return d.ptr != NULL ? hash_value(*d.ptr) : 0;
		}
	};

	template<class T>
	struct FWDumbEqual
	{
		bool operator()(const DumbPtr<T>& a, const DumbPtr<T>& b) const
		{
			return a == b;
		}
	};

	// Holder (interning-dictionary) interface behind FWBase+12. The binary
	// vtable carries update/notify/name slots plus the shared-holder provider
	// (registerFlyweight's +20 call); names below are ours except where noted.
	// 2013 slot order (FWDictionnary<FWPartInstance> vftable PLAYER 0x00b79100): no virtual
	// dtor; FWValue<bool>::set 0x0052c000 calls +4 (valueUpdate), +8 (insertCopy) and +0x10
	// (checkHash, for its FWUpdate log).
	class IFWHolder
	{
	public:
		virtual void destroyFlyweight(FWBase* base) = 0;		// +0x00  0x00845d20
		virtual FWBase* valueUpdate(FWRef* ref) = 0;			// +0x04  0x00845f80
		virtual void insertCopy(FWBase* base, FWRef* ref) = 0;	// +0x08  0x00846130
		virtual bool checkIfInDict(FWBase* base) = 0;			// +0x0c  0x00845da0
		virtual unsigned int checkHash(FWBase* base) = 0;		// +0x10  0x00845b60
	};

	class FWDictionnaryBase
		: public IFWHolder
		, public boost::enable_shared_from_this<FWDictionnaryBase>
	{
	public:
		virtual ~FWDictionnaryBase() {}
	};

	// Owning flyweight reference. Single field: shared_ptr<FWBase>.
	// (FWValue::set reads base = fwPtr.get(); registerFlyweight/valueUpdate
	// assert fwPtr.get() != NULL.)
	class FWRef
	{
	public:
		shared_ptr<FWBase> fwPtr;
	};

	// TEMPLATE: PLAYER 0x0052c000
	// ?set@?$FWValue@_N@RBX@@QAEXAB_NPAVFWRef@2@@Z

	// A flyweight field: the value plus its copy-on-write setter (2013 mangles FWValue<bool>::set(bool const&, FWRef*), a member; bodies below FWBase).
	template<class T>
	class FWValue
	{
	public:
		T value;
		FWValue() {}	// trivial in 2013: FWBase ctor 0x00619740 leaves 0x18..0x1a alone
		FWValue(const T& v) : value(v) {}
		operator const T&() const { return value; }
		void set(const T& v, FWRef* ref);
		static void set(T& holder, const T& value, FWRef* ref);
	};

	template<class T> class FWFinal;
	// VTABLE: PLAYER 0x00b2a214
	// VTABLE: STUDIO 0x00becfd4
	class FWBase : public boost::enable_shared_from_this<FWBase>
	{
	public:
		// 2013 vftable 0x00b2a214: the deleting destructor only (0x006197b0; dtor 0x00619690).
		// FUNCTION: PLAYER 0x00619690
		// FUNCTION: STUDIO 0x006ee360
		virtual ~FWBase()
		{
			FASTLOG1(FLog::FWLifetime, "FW instance destroyed: %p", this);
		}

		// SYNTHETIC: PLAYER 0x006197b0
		// SYNTHETIC: STUDIO 0x006ee480
		// RBX::FWBase::`scalar deleting destructor'

		// FUNCTION: PLAYER 0x00619740
		// FUNCTION: STUDIO 0x006ee410
		FWBase()
			: holder()
			, flag(false)
		{
			FASTLOG1(FLog::FWLifetime, "FW instance created: %p", this);
		}

		// STUB: PLAYER 0x00618cf0
		// STUB: STUDIO 0x006ed9f0
		bool operator==(const FWBase& lhs) const
		{
			// mac: 0x546CF8: "!holder || !lhs.holder || holder == lhs.holder"
			// (FeatherweightDict.h:65).
			RBXASSERT(!holder || !lhs.holder || holder.get() == lhs.holder.get());
			if (this == &lhs)
			{
				if (FLog::FWUpdate)
					FLog::FastLog(FLog::FWUpdate, "comparing FWBase %p", this);
				return !flag;
			}
			return true;
		}

		shared_ptr<FWBase> sharedSelf()
		{
			return shared_from_this();
		}

		// init<FWInstance> 0x0061fe70: Allocator<FWInstance>::operator new(0x38) 0x0061bbd0,
		// FWInstance() 0x0061d7e0, then the FWFinal<FWInstance> vptr 0x00b2a8b4. The pool goes
		// back through FWFinal's virtual destructor (0x0061f220 frees with Allocator's delete),
		// so the refs hold plain shared_ptr<FWBase> (sp_counted_impl_p<FWBase> RTTI).
		// TEMPLATE: PLAYER 0x0061fe70
		// TEMPLATE: STUDIO 0x006f4730
		// ??$init@VFWInstance@RBX@@@FWBase@RBX@@SAPAVFWInstance@1@PAV21@@Z
		template<class T>
		static T* init(T* t)
		{
			if (!t)
				t = new FWFinal<T>();
			return t;
		}

		shared_ptr<FWDictionnaryBase> holder;	// 0x0c (compared in operator==)
		bool flag;								// 0x14 (FWValue re-entrancy guard; set 0x0052c139)
		// FWBase ends at 0x18 (ctor 0x00619740 stores 0x4..0x14).
	};


	// Plain string holder (FWStringValue(ptr, "Instance") in FWInstance ctor).
	class FWStringValue
	{
	public:
		std::string value;
		FWStringValue() {}
		FWStringValue(const char* s) { value = s; }	// default ctor + operator=(const char*): Instance ctor 0x00620fce
	};

	class FWInstance;
	class FWPartInstance;

	// Free hash entry points (binary-faithful; defined below the classes).
	// Forward-declared: MSVC resolves overloads eagerly, and the fwHash
	// virtuals call these mid-class.
	inline size_t hash_value(const FWInstance& fw);
	inline size_t hash_value(const FWPartInstance& fw);
	inline size_t hash_value(int v);

	// VTABLE: PLAYER 0x00b2a234
	// VTABLE: STUDIO 0x00becff4
	class FWInstance : public FWBase, public Allocator<FWInstance>
	{
	public:
		// mac: 0x5457B8: base + bytes(+21/+22 = 0, +23 = 1) + name "Instance".
		// name("Instance"): FWStringValue's own ctor keeps its this (+0x1c) in a frame slot for
		// the unwind of value (0x0061d82d).
		// FUNCTION: PLAYER 0x0061d7e0
		// FUNCTION: STUDIO 0x006f2210
		FWInstance()
			: b21(false), b22(false), b23(true), name("Instance")
		{
		}

		// The implicit destructor is out of line (string at +0x1c, then ~FWBase).
		// SYNTHETIC: PLAYER 0x006197d0
		// SYNTHETIC: STUDIO 0x006ee4a0
		// RBX::FWInstance::~FWInstance

		// SYNTHETIC: PLAYER 0x0061c320
		// SYNTHETIC: STUDIO 0x007be5f0
		// RBX::FWInstance::`scalar deleting destructor'

		// mac: 0x545822: base eq + bytes + name compare.
		// FUNCTION: PLAYER 0x00619140
		// FUNCTION: STUDIO 0x006ede60
		bool operator==(const FWInstance& other) const
		{
			return FWBase::operator==(other)
				&& b23 == other.b23 && b21 == other.b21 && b22 == other.b22
				&& name.value == other.name.value;
		}

		// The ctor 0x0061d7e0 stores 0x18 = 0, 0x19 = 0, 0x1a = 1 after Allocator<FWInstance>().
		FWValue<bool> b21;						// 0x18 parent lock (setAndLockParent, DataModel ctor)
		FWValue<bool> b22;						// 0x19 RobloxLocked (setRobloxLocked 0x0061d780)
		FWValue<bool> b23;						// 0x1a Archivable (setIsArchivable 0x0061d7b0)
		FWStringValue name;						// 0x1c
	};

	// Marker subclass for per-class PV dicts/inits (no payload of its own —
	// FWPartInstance fills the 56-byte pool exactly: 28 + 28).
	class FWPVInstance : public FWInstance
	{
	public:
		FWPVInstance() {}
	};

	// 2013 RTTI (vftable 0x00b4c968): Allocator<FWPartInstance> @0x38, pool size 0x54 (D0
	// 0x00724030 gives 0x54 back to 0x00df44a0).
	class FWPartInstance : public FWPVInstance, public Allocator<FWPartInstance>
	{
	public:
		using Allocator<FWPartInstance>::operator new;
		using Allocator<FWPartInstance>::operator delete;

		// mac: 0x44AABA: FWInstance::FWInstance() + payload defaults.
		// (Allocator() call + vtable stores are MSVC-automatic.)
		FWPartInstance()
			: material(Plastic)
			, brickColor()
			, transparency(0.0f)
			, reflectance(0.0f)
			, alphaModifier(1.0f)
			, locked(false)
			, fw52(0)
		{
		}

		// mac: 0x44AB3E: base eq + payload compare (binary order kept).
		// STUB: PLAYER 0x0071c8f0
		// STUB: STUDIO 0x007b8350
		bool operator==(const FWPartInstance& other) const
		{
			if (!FWInstance::operator==(other))
				return false;
			return material == other.material
				&& brickColor == other.brickColor
				&& transparency == other.transparency
				&& reflectance == other.reflectance
				&& locked == other.locked
				&& alphaModifier == other.alphaModifier
				&& fw52 == other.fw52;
		}

		// Field identities from the property setters (Mach-O offsets kept as
		// documentation; MSVC layout differs, ORDER preserved):
		// +28 material via setRenderMaterial 0x448F10 (FWValue<Material>;
		//   256 = default; int storage until Material.h is reconstructed).
		// +32 BrickColor via setColor 0x449056 (FWValue<BrickColor>).
		// +36 transparency via setTransparency 0x448F92 (FWValue<float>).
		// +40 reflectance via setReflectance 0x448FFA (FWValue<float>).
		// +44 alphaModifier via setAlphaModifier 0x4427A2 (FWValue<float>).
		// +48 Locked via setPartLocked 0x448D8C (FWValue<bool>).
		// +52 int in ==/hash with no FWValue writer found — TODO identity.
		// (Friction/elasticity/anchored/dragging live on Primitive, not FW —
		// mac: 0x4490B6/0x449118/0x448AC4/0x442A12; specificGravity is derived.)
		Material material;			// +28 (FWValue<Material>; Plastic default)
		BrickColor brickColor;		// +32
		float transparency;			// +36
		float reflectance;			// +40
		float alphaModifier;		// +44
		bool locked;				// +48
		unsigned char _pad49[3];
		int fw52;					// +52
	};

	// Leaf class every flyweight is created as (init<T>, valueUpdate's copy): its destructor
	// takes the entry out of the holder dict. FWFinal<FWInstance> dtor 0x0061f1c0 (holder at
	// +0xc, IFWHolder slot 0), FWFinal<FWPartInstance>(const FWPartInstance&) 0x00845c00.
	template<class T>
	class FWFinal : public T
	{
	public:
		FWFinal() {}
		FWFinal(const T& other) : T(other) {}
		// TEMPLATE: PLAYER 0x0061f1c0
		// TEMPLATE: STUDIO 0x006f3a80
		// ??1?$FWFinal@VFWInstance@RBX@@@RBX@@UAE@XZ
		virtual ~FWFinal()
		{
			if (this->holder)
				this->holder->destroyFlyweight(this);
		}

		// SYNTHETIC: PLAYER 0x0061f220
		// SYNTHETIC: STUDIO 0x006f3ae0
		// RBX::FWFinal<RBX::FWInstance>::`scalar deleting destructor'
	};

	// boost::hash_combine-style hash over (b23, b21, b22, name chars) — 0x54587B.
	inline size_t hash_value(const FWInstance& fw)
	{
		const unsigned int k = 0x9E3779B9u;  // -1640531527
		size_t seed = (size_t)(fw.b23 - (int)k);
		seed ^= ((seed >> 2) + (seed << 6) + (size_t)fw.b21 - k);
		seed ^= (((seed >> 2) + (seed << 6) + (size_t)fw.b22 - k));
		const std::string& s = fw.name.value;
		unsigned int h = 0;
		bool anyChars = false;
		for (size_t i = 0; i < s.size(); ++i)
		{
			h ^= (unsigned char)s[i] + (h >> 2) + (h << 6) - k;
			anyChars = true;
		}
		unsigned int v4 = anyChars ? (h - k) : (unsigned int)-(int)k;
		seed ^= (v4 + ((seed << 6) + (seed >> 2)));
		return seed;
	}

	// mac: 0x44A9A7: base hash + int mixes + conditional float-bit mixes
	// (zero floats contribute -k) + byte mix + trailing int mix with the
	// extra >>3 term — binary order kept verbatim.
	// STUB: PLAYER 0x00720f90
	// STUB: STUDIO 0x007bcab0
	inline size_t hash_value(const FWPartInstance& fw)
	{
		const int k = -1640531527;
		size_t seed = hash_value(static_cast<const FWInstance&>(fw));
		seed ^= ((seed >> 2) + (unsigned int)fw.material + (seed << 6) + (size_t)k);
		seed ^= ((seed >> 2) + (seed << 6) + hash_value(fw.brickColor.asInt()) + (size_t)k);
		int transparencyMix = k;
		if (fw.transparency != 0.0f)
		{
			int bits = 0;
			memcpy(&bits, &fw.transparency, sizeof(bits));
			transparencyMix = bits + k;
		}
		int reflectanceMix = k;
		if (fw.reflectance != 0.0f)
		{
			int bits = 0;
			memcpy(&bits, &fw.reflectance, sizeof(bits));
			reflectanceMix = bits + k;
		}
		seed ^= (transparencyMix + (seed << 6) + (seed >> 2));
		seed ^= (reflectanceMix + (seed << 6) + (seed >> 2));
		int alphaMix = k;
		if (fw.alphaModifier != 0.0f)
		{
			int bits = 0;
			memcpy(&bits, &fw.alphaModifier, sizeof(bits));
			alphaMix = bits + k;
		}
		seed ^= ((seed >> 2) + (seed << 6) + (size_t)fw.locked + (size_t)k);
		seed ^= (alphaMix + (seed << 6) + (seed >> 2));
		seed ^= ((seed >> 2) + (unsigned int)fw.fw52
			+ ((unsigned int)fw.fw52 >> 3) + (seed << 6) + (size_t)k);
		return seed;
	}

	// int mix used by the part hash (+7 slot) — same combine idiom.
	inline size_t hash_value(int v)
	{
		const int k = -1640531527;
		return (size_t)((v >> 2) + (unsigned int)v + ((unsigned int)v << 6) + (size_t)k);
	}

	// FWValue<T>::set bodies (declared above FWBase).
	// 2013 member form (PLAYER 0x0052c000 for bool, 0x00619a70 for std::string): `this` is
	// the value inside the flyweight. Copy-on-write: save old, flag, store new, valueUpdate
	// (lookup under the new value), restore old, unflag, insertCopy on a miss.
	template<class T>
	void FWValue<T>::set(const T& v, FWRef* ref)
	{
		if (v == value)
			return;
		shared_ptr<FWBase> base = ref->fwPtr;
		RBXASSERT(base != NULL);
		if (!base->holder)
		{
			value = v;
			return;
		}
		FWBase* node = ref->fwPtr.get();
		FASTLOG3(FLog::FWUpdate, "Updating node %p (ref %p), hash before mutation: %u", node, ref, base->holder->checkHash(node));
		T old = value;
		node->flag = true;
		value = v;
		FWBase* copy = base->holder->valueUpdate(ref);
		value = old;
		node->flag = false;
		if (copy)
			base->holder->insertCopy(copy, ref);
		FASTLOG3(FLog::FWUpdate, "Update completed, newnode: %p, old node hash: %u, new node hash: %u",
			ref->fwPtr.get(), base->holder->checkHash(node), base->holder->checkHash(ref->fwPtr.get()));
	}

	// Pre-2013-layout static form, kept for the FWPartInstance fields that are still plain
	// members (PartInstance.cpp) and the name string (Instance.cpp).
	template<class T>
	void FWValue<T>::set(T& holder, const T& value, FWRef* ref)
	{
		if (value == holder)
			return;
		FWBase* base = ref->fwPtr.get();
		RBXASSERT(base != NULL);
		shared_ptr<FWDictionnaryBase> dict = base->holder;
		if (dict == NULL)
		{
			holder = value;
			return;
		}
		T oldValue = holder;
		base->flag = true;
		holder = value;
		FWBase* copy = dict->valueUpdate(ref);
		holder = oldValue;
		base->flag = false;
		if (copy)
			dict->insertCopy(copy, ref);
	}

	// Interning dictionary. valueUpdate 0x6D3058 returns NULL when the ref was
	// redirected onto an interned entry, else a raw copy for insertCopy;
	// checkHash is trivially hash_value (0x6D33DE). MSVC port of the
	// boost::unordered_set<DumbPtr<T>> mechanics (dict set at +4, update
	// counter at +28).
	template<class T>
	class FWDictionnary : public FWDictionnaryBase
	{
	public:
		FWDictionnary()
			: updateCounter(0)
		{
		}

		// FUNCTION: PLAYER 0x00845b60
		// FUNCTION: STUDIO 0x0098b530
		virtual unsigned int checkHash(FWBase* base)  // 0x6D33DE
		{
			return (unsigned int)hash_value(*static_cast<T*>(base));
		}

		// STUB: PLAYER 0x00845da0
		// STUB: STUDIO 0x0098b770
		virtual bool checkIfInDict(FWBase* base)  // 0x6D33B0
		{
			DumbPtr<T> key(static_cast<T*>(base));
			return dictSet.find(key) != dictSet.end();
		}

		// mac: 0x6D328E: intern the copy valueUpdate made and point the ref at it (plain
		// ownership), holder = this. Asserts the entry is new (FeatherweightDict.h:302).
		// STUB: PLAYER 0x00846130
		// STUB: STUDIO 0x0098bb00
		virtual void insertCopy(FWBase* base, FWRef* ref)
		{
			T* adopted = static_cast<T*>(base);
			std::pair<typename Set::iterator, bool> result = dictSet.insert(DumbPtr<T>(adopted));
			RBXASSERT(result.second == true);  // "result.second == true" (:302)
			ref->fwPtr.reset(adopted);
			adopted->holder = selfShared();
			if (FLog::FWLifetime)
				FLog::FastLog(FLog::FWLifetime, "insertCopy %p", adopted);
		}

		// mac: 0x44BCEC: intern ref->fwPtr (assert shared-owned, :237); new ->
		// holder = self; dup -> ref adopts the interned entry (assert its
		// holder is this, :244).
		// Self-provision is the dict vtable +20 slot: heap dicts use
		// shared_from_this; the FWService-embedded dict is NOT shared-owned,
		// so FWService wires selfProvider to its aliasing getUniqueSharedPtr.
		boost::function<shared_ptr<FWDictionnaryBase>()> selfProvider;
		shared_ptr<FWDictionnaryBase> selfShared()
		{
			if (!selfProvider.empty())
				return selfProvider();
			return shared_from_this();
		}
		// STUB: PLAYER 0x00725d90
		// STUB: STUDIO 0x007c1310
		void registerFlyweight(FWRef* ref)
		{
			FWBase* base = ref->fwPtr.get();
			RBXASSERT(base != NULL);  // "ref->fwPtr.get() != NULL" (:237)
			DumbPtr<T> key(static_cast<T*>(base));
			std::pair<typename Set::iterator, bool> result = dictSet.insert(key);
			if (result.second)
			{
				if (FLog::FWLifetime)
					FLog::FastLog(FLog::FWLifetime, "registerFlyweight new %p", base);
				base->holder = selfShared();
			}
			else
			{
				if (FLog::FWLifetime)
					FLog::FastLog(FLog::FWLifetime, "registerFlyweight dup %p", base);
				ref->fwPtr = result.first->ptr->sharedSelf();
				RBXASSERT(ref->fwPtr->holder.get() == this);  // (:244)
			}
		}

		// mac: 0x6D3058: re-intern after a value write. Hit -> redirect ref onto
		// the entry, token 0. Miss -> pool-copy the payload (binary
		// field-copies incl. holder incref + flag clear; the copy ctor +
		// flag reset below is the MSVC equivalent), token = raw copy for
		// notifyRef.
		// STUB: PLAYER 0x00845f80
		// STUB: STUDIO 0x0098b950
		virtual FWBase* valueUpdate(FWRef* ref)
		{
			++updateCounter;
			FWBase* base = ref->fwPtr.get();
			RBXASSERT(base != NULL);  // "ref->fwPtr.get() != NULL" (:279)
			DumbPtr<T> key(static_cast<T*>(base));
			typename Set::iterator it = dictSet.find(key);
			if (it != dictSet.end())
			{
				if (FLog::FWLifetime)
					FLog::FastLog(FLog::FWLifetime, "valueUpdate redirect %p", base);
				ref->fwPtr = it->ptr->sharedSelf();
				return NULL;
			}
			T* copy = new FWFinal<T>(*static_cast<const T*>(base));
			copy->flag = false;
			if (FLog::FWLifetime)
				FLog::FastLog(FLog::FWLifetime, "valueUpdate copy %p", copy);
			return copy;
		}

		// STUB: PLAYER 0x00845d20
		// STUB: STUDIO 0x0098b6f0
		virtual void destroyFlyweight(FWBase* base)  // 0x6D2FAC
		{
			if (FLog::FWLifetime)
				FLog::FastLog(FLog::FWLifetime, "destroyFlyweight %p", base);
			size_t erased = dictSet.erase(DumbPtr<T>(static_cast<T*>(base)));
			RBXASSERT(erased == 1);  // "erased == 1" (:259)
		}

	private:
		typedef boost::unordered_set<
			DumbPtr<T>,
			FWDumbHash<T>,
			FWDumbEqual<T> > Set;
		Set dictSet;			// +4
		int updateCounter;		// +28
	};

} // namespace RBX
