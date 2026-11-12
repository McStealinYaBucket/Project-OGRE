#pragma once

#include "reflection/Descriptor.h"
#include "util/Exception.h"
#include <vector>
#include "security/SecurityContext.h"

#include "rbx/DenseHash.h"
#include "rbx/trie.h"

namespace RBX
{
	namespace Reflection
	{
		class ClassDescriptor;
		class DescribedBase;

        struct StringHashPredicate
        {
            size_t operator()(const char* s) const;
        };

        struct StringEqualPredicate
        {
            bool operator()(const char* lhs, const char* rhs) const
            {
                return strcmp(lhs, rhs) == 0;
            }
        };

		// Base class of describing a described object's member: (Member, Event, etc.)
		class RBXBaseClass MemberDescriptor : public Descriptor
		{
		public:
			static void (*memberHidingHook)(MemberDescriptor*, MemberDescriptor*);

			// Category is a name used to group properties in the UI
			const RBX::Name* category;

			const ClassDescriptor& owner;
			const Security::Permissions security;

		protected:
			MemberDescriptor(const ClassDescriptor& owner, const char* name, const char* category, Attributes attributes, Security::Permissions security)
				:Descriptor(name, attributes)
				,owner(owner)
				,category(RBX::Name::declare(category))
				,security(security)
			{
			}
			virtual ~MemberDescriptor() {}
		public:
			bool isMemberOf(const ClassDescriptor& classDescriptor) const;
			bool isMemberOf(const DescribedBase* instance) const;
		};

		
		class MemberException : public std::runtime_error
		{
		public:
			const MemberDescriptor& desc;
			MemberException(const MemberDescriptor& desc, const std::string& _Message)
				:std::runtime_error(_Message)
				,desc(desc)
			{
			}
		};
		
		// 2013 layout (ClassDescriptor ctor 0x006182e0; lookup ctor 0x00615850 on
		// ClassDescriptor+0x20): descriptors @0x0, descriptorLookup @0x10 (rbx::trie: node
		// pool @+0, value pool @+0x1c, root @+0x38 = pool.construct(1) via 0x006154a0),
		// derivedContainers @0x4c, base @0x5c. 2016 used a DenseHashMap (0x1c) here; 2013's
		// lookup is rbx::trie<T*, 50> (the node ctor 0x00614f60 throws depth_exceeded_exception
		// when depth > 0x32; vftable rbx::trie<PropertyDescriptor*,50>::depth_exceeded_exception).
		// SIZE 0x60
		template<class MemberDescriptorType>
		class MemberDescriptorContainer
		{
			// Used for sorting
			static bool compare(const MemberDescriptorType* a, const MemberDescriptorType* b)
			{
				return a->name < b->name;
			}
		public:
			class Collection : public std::vector<MemberDescriptorType*>
			{
			};

			typedef rbx::trie<MemberDescriptorType*, 50> DescriptorLookup;

			typedef typename MemberDescriptorType::ConstMember ConstMemberType;
			typedef typename MemberDescriptorType::Member MemberType;
			class ConstIterator : public std::iterator<std::forward_iterator_tag, MemberType, void, MemberType>
			{
				friend class ClassDescriptor;
				const DescribedBase* instance;
				typename Collection::const_iterator iter;
			public:
				ConstIterator(const typename Collection::const_iterator& iter, const DescribedBase* instance)
					:iter(iter),instance(instance)
				{}
				ConstMemberType operator*() const
				{	// return designated object
					return ConstMemberType(**iter, instance);
				}

				bool operator==(const ConstIterator& other) const { return iter==other.iter; }
				bool operator!=(const ConstIterator& other) const { return iter!=other.iter; }
				ConstIterator& operator++()
				{	// preincrement
					++iter;
					return (*this);
				}

				ConstIterator operator++(int)
					{	// postincrement
					ConstIterator _Tmp = *this;
					++*this;
					return (_Tmp);
					}

				const MemberDescriptorType& getDescriptor() const { return **iter; }
			};

			class Iterator : public std::iterator<std::forward_iterator_tag, MemberType, void, MemberType>
			{
				friend class ClassDescriptor;
				DescribedBase* instance;
				// iter is a const iterator, since we never modifiy the collection of descriptors
				typename Collection::const_iterator iter;		
			public:
				Iterator(const typename Collection::const_iterator& iter, DescribedBase* instance)
					:iter(iter),instance(instance)
				{}
				MemberType operator*() const
				{	// return designated object
					return MemberType(**iter, instance);
				}

				bool operator==(const Iterator& other) const { return iter==other.iter; }
				bool operator!=(const Iterator& other) const { return iter!=other.iter; }
				Iterator& operator++()
				{	// preincrement
					++iter;
					return (*this);
				}

				Iterator operator++(int)
					{	// postincrement
					Iterator _Tmp = *this;
					++*this;
					return (_Tmp);
					}
			};

		protected:
			Collection descriptors;				// 0x00
			DescriptorLookup descriptorLookup;	// 0x10
		private:
			static Collection& staticData()
			{
				static Collection result;
				return result;
			}
			static void initStaticData()
			{
				staticData();
			}
			static Collection& allDescriptors()
			{
				static boost::once_flag flag = BOOST_ONCE_INIT;
				boost::call_once(&initStaticData, flag);
				return staticData();
			}

		protected:
			// This is a list of "subclasses"
			std::vector<MemberDescriptorContainer*> derivedContainers;	// 0x4c

			MemberDescriptorContainer* const base;						// 0x5c
		protected:
			MemberDescriptorContainer(MemberDescriptorContainer* base)
				:base(base)
			{
				if (base!=NULL)
				{
					// Grab base members that have already been declared
					mergeMembers(base);

					// Subsequent members declared in a base class will be pushed down in the declare() function
					base->derivedContainers.push_back(this);
				}
			}

			void declareSub(MemberDescriptorType* descriptor, MemberDescriptorType* replaceable)
			{
				RBXASSERT(replaceable != descriptor);
				{
					typename Collection::iterator iter = std::lower_bound(descriptors.begin(), descriptors.end(), descriptor, compare);
					if (iter == descriptors.end())
					{
						descriptors.insert(iter, descriptor);
						descriptorLookup[descriptor->name->c_str()] = descriptor;
					}
					else
					{
						RBXASSERT(*iter != descriptor);

						if (*iter == replaceable)
						{
							// Replace it
							*iter = descriptor;
							descriptorLookup[descriptor->name->c_str()] = descriptor;
						}
						else if ((*iter)->name != descriptor->name)
						{
							descriptors.insert(iter, descriptor);
							descriptorLookup[descriptor->name->c_str()] = descriptor;
						}
						else
						{
							// We've hit upon a member that will hide this member
							if (MemberDescriptor::memberHidingHook)
								(*MemberDescriptor::memberHidingHook)(descriptor, replaceable);
							return;	// No need to continue
						}
					}
				}

				// recurse:
				for (typename std::vector<MemberDescriptorContainer*>::iterator iter = derivedContainers.begin(); iter != derivedContainers.end(); ++iter)
					(*iter)->declareSub(descriptor, replaceable);
			}
		public:
			void declare(MemberDescriptorType* descriptor)
			{
				MemberDescriptorType* replaceable = NULL;

				{
					typename Collection::iterator iter = std::lower_bound(descriptors.begin(), descriptors.end(), descriptor, compare);
					if (iter == descriptors.end())
					{
						// add a new one
						descriptors.insert(iter, descriptor);
						descriptorLookup[descriptor->name->c_str()] = descriptor;
					}
					else if (*iter == descriptor) 
					{
						// drop out if we've been here before
						return;
					}
					else if ((*iter)->name != descriptor->name)
					{
						// add a new one
						descriptors.insert(iter, descriptor);
						descriptorLookup[descriptor->name->c_str()] = descriptor;
					}
					else
					{
						// hide a member of a base class
						// TODO: Eventually we'd like to nuke this feature, but it is
						//       required for some legacy things, like BoolValue
						replaceable = *iter;
						*iter = descriptor;
						descriptorLookup[descriptor->name->c_str()] = descriptor;
						if (MemberDescriptor::memberHidingHook)
							(*MemberDescriptor::memberHidingHook)(descriptor, replaceable);
					}
				}

				// Also declare this member in sub-classes
				for (typename std::vector<MemberDescriptorContainer*>::iterator iter = derivedContainers.begin(); iter != derivedContainers.end(); ++iter)
					(*iter)->declareSub(descriptor, replaceable);

				// Add this to allDescriptors (in a determanistic order) 
				{
					typename Collection::iterator iter = allDescriptors().begin();
					while (iter!=allDescriptors().end())
					{
						MemberDescriptorType* desc = *iter;
						if (desc==descriptor)
							goto SKIP;
						int compare = descriptor->name->compare(*desc->name);
						if (compare<0)
							break;
						if (compare==0)
						{
							// This descriptor name already exists in a different class
							compare = descriptor->owner.name->compare(*desc->owner.name);
							// Enforce order using class name
							if (compare<0)
								break;
						}
						++iter;
					}
					allDescriptors().insert(iter, descriptor);
SKIP:				;
				}

			}

			/////////////////////////////////////////////////////////////////
			// Type info enumeration
			typename Collection::const_iterator descriptors_begin() const {
				return descriptors.begin();
			}
			typename Collection::const_iterator descriptors_end() const {
				return descriptors.end();
			}
            size_t descriptor_size() const
            {
                return descriptors.size();
            }

			/////////////////////////////////////////////////////////////////
			// Enumeration of all descriptors
			static typename Collection::const_iterator all_begin() {
				return allDescriptors().begin();
			}
			static typename Collection::const_iterator all_end() {
				return allDescriptors().end();
			}

			/////////////////////////////////////////////////////////////////
			// Type info query
			MemberDescriptorType* findDescriptor(const char* name) const
			{
				MemberDescriptorType* result;
				if (descriptorLookup.lookup(name, result))
					return result;
				return NULL;
			}

			/////////////////////////////////////////////////////////////////
			// Member enumeration
			ConstIterator members_begin(const DescribedBase* instance) const {
				return ConstIterator(descriptors.begin(), instance);
			}
			ConstIterator members_end(const DescribedBase* instance) const {
				return ConstIterator(descriptors.end(), instance);
			}

			Iterator members_begin(DescribedBase* instance) const {
				return Iterator(descriptors.begin(), instance);
			}
			Iterator members_end(DescribedBase* instance) const {
				return Iterator(descriptors.end(), instance);
			}
		protected:
			void mergeMembers(const MemberDescriptorContainer* source)
			{
				for (typename Collection::const_iterator iter = source->descriptors.begin(); iter != source->descriptors.end(); ++iter)
					declare(*iter);

				// Recursively merge parent members as well
				if (source->base!=NULL)
					mergeMembers(source->base);
			}
		};

	}
}
