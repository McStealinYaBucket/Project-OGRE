
#pragma once

#include <string>
#include <stdlib.h>
#include <map>

#include "util/name.h"
#include "util/Object.h"
#include "util/utilities.h"

#include "rbx/intrusive_ptr_target.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr.hpp>
#include <tbb/queuing_rw_mutex.h>

namespace RBX
{
	// A simple class that is a globally unique identifier
	// Evidence: 0x204336 Guid::Guid, 0x204380 generateStandardGUID, 0x2044D0 generateRBXGUID,
	// mac: 0x204618 assign, 0x20462A Data::operator<, 0x204658/0x2046AC compare, 0x204720 Data::readableString
	// Asserts in the 2013 binary reference "include/Util/Guid.h" lines 89/144/150 -> this header
	class Guid : boost::noncopyable
	{
	public:
		struct Scope
		{
		public:
			Scope() { setNull(); }

			void setNull() { name = &RBX::Name::getNullName(); }
			bool isNull() const {
				return *name == RBX::Name::getNullName();
			}
			void set( const std::string& s )
			{
				name = RBX::Name::declare(s.c_str());
			}
			void set( const char* s )
			{
				name = RBX::Name::declare(s);
			}

			const RBX::Name* getName() const {
				return name;
			}

			int compare(const Scope& other) const
			{
				return name->compare(*other.name);
			}
			bool operator ==(const Scope& other) const {
				return name->compare(*other.name) == 0;
			}
			bool operator  <(const Scope& other) const {
				return name->compare(*other.name) < 0;
			}

			static const Scope& null();

		private:
			static Scope nullScope;

			const RBX::Name* name;
		};

		struct Data
		{
			Scope scope;
			int index;
			bool operator ==(const Data& other) const;
			bool operator <(const Data& other) const;
			// For debugging only. A string that is not guaranteed to be unique
			std::string readableString(int scopeLength = 4) const;
		};
	private:
		Data data;
	public:
		Guid();
		bool operator ==(const Guid& other) const { return data == other.data; }
		bool operator <(const Guid& other) const { return data < other.data; }

		// Compare 2 pairs of Guids. a0-a1 and b0-b1 are commutativity. Any item may be NULL
		static int compare(const Guid* a, const Guid* b);
		static int compare(const Guid* a0, const Guid* a1, const Guid* b0, const Guid* b1);

		// Used for serialization:
		void assign(Data data);
		void extract(Data &data) const { data = this->data; }

		void copyDataFrom(const Guid& other) {
			Data data;
			other.extract(data);
			this->assign(data);
		}

		// For debugging only. A string that is not guaranteed to be unique
		std::string readableString(int scopeLength = 4) const { return data.readableString(scopeLength); }

		static const RBX::Guid::Scope& getLocalScope();

		static void generateRBXGUID(RBX::Guid::Scope& result);

		// Creates a string like this: RBXc200e36038c511ceae6208002b2b79ef
		static void generateRBXGUID(std::string& result);

		// Creates a string like this: {c200e360-38c5-11ce-ae62-08002b2b79ef}
		static void generateStandardGUID(std::string& result);
	};

    inline size_t hash_value(const Guid::Data& data)
    {
        size_t result = 0;
        boost::hash_combine(result, data.scope.getName());
        boost::hash_combine(result, data.index);
        return result;
    }

	// A base class for objects that contain a Guid and that want lookup-by-Guid
	// 2013 layout (dtor 0x0061b060, Registry::unregister 0x0058dd90; GuidItem @Instance+4):
	// vfptr @0 (onGuidChanged), shared_ptr<Registry> @0x4, Guid @0xc. The 2016
	// intrusive_ptr made it 0x10.
	// SIZE 0x14
	template<class T>
	class RBXBaseClass GuidItem
	{
	public:
		// A Registry maintains lookup information for Guids.
		// Each instance can belong to only one Registry.
		// Usually a Registry is associated with a DataModel
		// 2013 layout (reg 0x00563570): enable_shared_from_this @0 (reg assigns
		// item->registry = shared_from_this()), map @0x8, mutex @0x14. Owned by
		// shared_ptr (sp_counted_impl_p<Registry>::dispose 0x005bad40), not by the
		// 2016 intrusive_ptr.
		class Registry
			: public boost::enable_shared_from_this<Registry>
		{
			friend class GuidItem;
			// 2013 binary: std::_Rb_tree<RBX::Guid::Data, std::pair<RBX::Guid::Data const, RBX::Instance*>,
			// std::_Select1st<...>, std::less<RBX::Guid::Data>, std::allocator<...>> (0x54722C etc)
			// i.e. a std::map<Guid::Data, T*> of raw pointers -- NOT the 2016 unordered_map<Guid::Data, weak_ptr<T>>
			typedef std::map<Guid::Data, T*> Map;
			Map map;
			// 2013 binary: Registry::unregister (0x5470D6) acquires tbb::queuing_rw_mutex::scoped_lock
			// with write=true; NOT RBX::mutex as in 2016.
			mutable tbb::queuing_rw_mutex mutex;

			Registry()
			{}
		public:
			static shared_ptr<Registry> create()
			{
				return shared_ptr<Registry>(new Registry());
			}

			~Registry()
			{
				RBXASSERT(map.size() ==0);
			}

			// Returns true for empty Guid data, false for unregistered Guid data
			bool lookupByGuid(const Guid::Data& data, shared_ptr<T> &result)
			{
				if(data.scope.isNull())
				{
					result.reset();
					return true;
				}

				tbb::queuing_rw_mutex::scoped_lock lock(mutex, /*write=*/false);
				typename Map::const_iterator iter = map.find(data);
				if (iter!=map.end())
				{
					result = shared_from(iter->second);
				}
				else
				{
					result.reset();
				}
				return !!result;
			}

			shared_ptr<T> getByGuid(const Guid::Data& data)
			{
				shared_ptr<T> result;
				lookupByGuid(data, result);
				return result;
			}

			// Ensure that this Guid is in the registry (thread-safe)
			void registerGuid(const T* item)
			{
				reg(item);
			}

			// Assigns a new guid to item. (not thread-safe) -- 0x84E59C
			void assignGuid(T* item, const Guid::Data& guidData)
			{
				if (item->registry)
					item->registry->unregister(item);

				item->guid.assign(guidData);

				reg(item);

				item->onGuidChanged();
			}

			void tryUnregister(GuidItem* item)
			{
				// In ClientReplicator::streamOutInstance(), we traverse all the part's descendants and unregister them
				// It is possible some descendants have been already GC'd and unregistered earlier from the same GC loop in GCJob::gcRegion()
				// Here we simply skip it if the item has already been unregistered.
				if (item->registry)
				{
					unregister(item);
				}
			}

			// mac: 0x5470D6 -- asserts "item->registry.get()==this" (Guid.h line 144) and
			// "num == 1" (Guid.h line 150)
			// STUB: PLAYER 0x0058dd90
			// STUB: STUDIO 0x00633190
			void unregister(GuidItem* item)
			{
				RBXASSERT(item->registry.get()==this);
				Guid::Data data;
				item->guid.extract(data);
				{
					tbb::queuing_rw_mutex::scoped_lock lock(mutex, /*write=*/true);
					int num = map.erase(data);
					RBXASSERT(num == 1);
				}
				item->registry.reset();
			}

		private:
			void reg(const T* item)
			{
				if (!item->registry)
				{
					Guid::Data data;
					item->guid.extract(data);
					tbb::queuing_rw_mutex::scoped_lock lock(mutex, /*write=*/true);
					if (!item->registry)	// thread-safe check
					{
						bool ok = map.insert(std::make_pair(data, const_cast<T*>(item))).second;
						RBXASSERT(ok);
						item->registry = this->shared_from_this();
					}
				}
				else
					RBXASSERT(item->registry.get()==this);
			}

		};
		friend class Registry;

	private:
		mutable shared_ptr<Registry> registry;	// 0x04
		Guid guid;								// 0x0c

	public:
		GuidItem()
		{
		}

		// FUNCTION: PLAYER 0x0061b060
		~GuidItem()
		{
			if (registry)
				registry->unregister(this);
		}

		const Guid& getGuid() const
		{
			return guid;
		}

	protected:
		// mac: 0x4798F0 -- virtual no-op in the 2013 binary (GuidItem<T> carries a vtable)
		// STUB: PLAYER 0x0040b950
		virtual void onGuidChanged()
		{
		}
	};
};

