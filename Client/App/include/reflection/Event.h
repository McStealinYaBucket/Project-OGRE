
#pragma once

#include "reflection/type.h"
#include "reflection/member.h"
#include "security/SecurityContext.h"
#include "util/standardout.h"
#include "boost/any.hpp"
#include "boost/cast.hpp"
#include "boost/bind.hpp"
#include "boost/static_assert.hpp"
#include "boost/shared_ptr.hpp"
#include <algorithm>

#include "rbx/Countable.h"
#include "rbx/signal.h"
#include "reflection/type.h"

namespace RBX
{
    class SystemAddress;

	namespace Reflection
	{
		/*
			TODO: Review all this code

			This is an event system inspired by boost's signal library.

			It allows clients to connect slots to a signal both in a typed way, but also
			using a generic argument list. This makes it possible to interface the signals
			with Lua and other runtime interfaces


		*/
		typedef std::vector< Variant > EventArguments;


		template<class GenericSlot>	class TGenericSlotWrapper;
		
		// A GenericSlot is a slot that takes "EventArguments" instead of typed function arguments
		// Thus, it can slot to any signal.
		// This wrapper defines an interface to such a slot
		class RBXInterface GenericSlotWrapper 
			: boost::noncopyable
			, public RBX::Diagnostics::Countable< GenericSlotWrapper >
		{
			// TODO: make members private/protected as much as possible 
		public:
			virtual ~GenericSlotWrapper() {}
			virtual void execute(const EventArguments& arguments) = 0;
			template<class Arg1>
			void execute1(const Arg1& arg1)
			{
				EventArguments args(1);
				args[0] = arg1;
				execute(args);
			}
			template<class Arg1, class Arg2>
			void execute2(const Arg1& arg1, const Arg2& arg2)
			{
				EventArguments args(2);
				args[0] = arg1;
				args[1] = arg2;
				execute(args);
			}
			template<class Arg1, class Arg2, class Arg3>
			void execute3(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
			{
				EventArguments args(3);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				execute(args);
			}
			
			template<class Arg1, class Arg2, class Arg3, class Arg4>
			void execute4(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
			{
				EventArguments args(4);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
				execute(args);
			}
            
            template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
			void execute5(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5)
			{
				EventArguments args(5);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
                args[4] = arg5;
				execute(args);
			}

			template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
			void execute6(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, const Arg6& arg6)
			{
				EventArguments args(6);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
				args[4] = arg5;
				args[5] = arg6;
				execute(args);
			}

			template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
			void execute7(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, const Arg6& arg6, const Arg7& arg7)
			{
				EventArguments args(7);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
				args[4] = arg5;
				args[5] = arg6;
				args[6] = arg7;
				execute(args);
			}
			template<class GenericSlot>
			static shared_ptr<GenericSlotWrapper> create(GenericSlot slot)
			{
				return shared_ptr<GenericSlotWrapper>(new TGenericSlotWrapper<GenericSlot>(slot));
			}
		};

		template<class GenericSlot>
		class TGenericSlotWrapper 
				: public GenericSlotWrapper
				, public RBX::Diagnostics::Countable< TGenericSlotWrapper<GenericSlot> >
		{
			friend class GenericSlotWrapper;
		public:
			TGenericSlotWrapper(const GenericSlot& slot)
				:slot(slot)
			{
			}
			GenericSlot slot;
			virtual void execute(const EventArguments& arguments) 
			{ 
				try
				{
					slot(arguments);
				}
				catch(RBX::base_exception& e)
				{
					RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "Exception caught in TGenericSlotWrapper. %s", e.what());
				}
			} 
		};

		//Forward declare EventDescriptor
		class EventDescriptor;
		// The base class of any object that fires Events
		// In 2013 the vftable 0x00b2a060 has 3 slots (dtor, processRemoteEvent 0x0066edf0,
		// raiseEventInvocation); 2016's useSubmitTaskForLuaListeners did not exist yet.
		// It is DescribedBase's first base subobject (DescribedBase ctor 0x0061c2c0).
		// SIZE 0x4
		class EventSource
		{
		public:
			virtual ~EventSource() {}

            // This is used for processing of remote events
            virtual void processRemoteEvent(const EventDescriptor& descriptor, const EventArguments& args, const SystemAddress& source);

			// This is used to replicate events:
			virtual void raiseEventInvocation(const EventDescriptor& descriptor, const EventArguments& args, const SystemAddress* target = NULL);
		};

		/*
			Describes a signal of a SignalSource/DescribedBase
		*/
		class Event;
		class RBXBaseClass EventDescriptor : public MemberDescriptor
		{
		public:
			typedef Event ConstMember;
			typedef Event Member;

		protected:
			SignatureDescriptor signature;
			EventDescriptor(ClassDescriptor& classDescriptor, const char* name, Security::Permissions security, Attributes attributes);

		public:
			virtual rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const = 0;
			const SignatureDescriptor& getSignature() const { return signature; }
			
			virtual bool isScriptable() const { return true; }
			bool isPublic() const { return isScriptable(); }

			virtual bool isBroadcast() const { return false; }
			virtual void fireEvent(DescribedBase* source, const EventArguments& args) const = 0;
			virtual void sendEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBXASSERT(false);	// Should only be called on RemoteEventDesc
			}

			bool operator==(const EventDescriptor& other) const {
				return this == &other;
			}
			bool operator!=(const EventDescriptor& other) const {
				return this != &other;
			}
			virtual void disconnectAll(DescribedBase* source) const = 0;
		};

		template<class EventClass, typename Signature, typename SignalType, typename EventGetter = SignalType EventClass::*>
		class RBXBaseClass EventDescBase;

		template<class EventClass, typename Signature, typename SignalType>
		class RBXBaseClass EventDescBase<EventClass, Signature, SignalType, SignalType EventClass::*> : public EventDescriptor
		{
		private:
			SignalType EventClass::*sig;
		protected:
			EventDescBase(SignalType EventClass::*sig, const char* name, Security::Permissions security, Attributes attributes)
				:EventDescriptor(EventClass::classDescriptor(), name, security, attributes)
				,sig(sig)
			{}

			SignalType& getSignal(EventClass* obj) const
			{
				return (obj->*sig);
			}
		public:
			/* The source is checked via the reflection descriptor rather than a
			   polymorphic_downcast on EventSource (not yet verified against 2013). */
			inline rbx::signals::connection connect(DescribedBase* source, const boost::function<Signature>& slot) const
            {
				if (source)
				{
					EventClass* e = source->fastDynamicCast<EventClass>();
					return (e->*sig).connect(slot);
				}
				else
					return rbx::signals::connection();		// return an empty connection
			}

			inline void disconnectAll(DescribedBase* source) const
			{
				EventClass* e = source->fastDynamicCast<EventClass>();
				(e->*sig).disconnectAll();
			}
		};

		template<class EventClass, typename Signature, typename SignalType>
		class RBXBaseClass EventDescBase<EventClass, Signature, SignalType, SignalType* (EventClass::*)(bool)> : public EventDescriptor
		{
		private:
			SignalType* (EventClass::*getOrCreate)(bool);
		protected:
			EventDescBase(SignalType* (EventClass::*getOrCreate)(bool), const char* name, Security::Permissions security, Attributes attributes)
				:EventDescriptor(EventClass::classDescriptor(), name, security, attributes)
				,getOrCreate(getOrCreate)
			{}

			SignalType& getSignal(EventClass* obj) const
			{
				SignalType* s = (obj->*getOrCreate)(true);
                RBXASSERT(s);
                return *s;
			}
		public:
			inline rbx::signals::connection connect(DescribedBase* source, const boost::function<Signature>& slot) const
            {
				if (source)
				{
					EventClass* e = source->fastDynamicCast<EventClass>();
					return getSignal(e).connect(slot);
				}
				else
					return rbx::signals::connection();		// return an empty connection
			}

			inline void disconnectAll(DescribedBase* source) const
			{
				EventClass* e = source->fastDynamicCast<EventClass>();
                SignalType* s = (e->*getOrCreate)(false);

                if (s)
                    s->disconnectAll();
			}
		};

		template<int arity, class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl;

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<0, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const 
			{
				EventArguments foo2;
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute, wrapper, foo2));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 0);
				EventClass* e = source->fastDynamicCast<EventClass>();
				return this->getSignal(e)();
			}
			void fireEvent(EventClass* instance) const
			{
				getSignal(instance)();
			}
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<1, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute1<
					typename boost::function_traits<Signature>::arg1_type>, 
					wrapper, _1));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 1);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
					args[0].cast<typename boost::function_traits<Signature>::arg1_type>()
					);
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1) const
			{
				this->getSignal(instance)(arg1);
			}

		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<2, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute2<
					typename boost::function_traits<Signature>::arg1_type, 
					typename boost::function_traits<Signature>::arg2_type>, 
					wrapper, _1, _2));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 2);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
					args[0].cast<typename boost::function_traits<Signature>::arg1_type>(),
					args[1].cast<typename boost::function_traits<Signature>::arg2_type>()
					);
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2) const
			{
				this->getSignal(instance)(arg1,arg2);
			}

		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<3, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute3<
				typename boost::function_traits<Signature>::arg1_type, 
				typename boost::function_traits<Signature>::arg2_type, 
				typename boost::function_traits<Signature>::arg3_type>, 
				wrapper, _1, _2, _3));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 3);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
					args[0].cast<typename boost::function_traits<Signature>::arg1_type>(),
					args[1].cast<typename boost::function_traits<Signature>::arg2_type>(),
					args[2].cast<typename boost::function_traits<Signature>::arg3_type>()
					);
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3) const
			{
				this->getSignal(instance)(arg1,arg2,arg3);
			}

		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<4, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute4<
				typename boost::function_traits<Signature>::arg1_type, 
				typename boost::function_traits<Signature>::arg2_type, 
				typename boost::function_traits<Signature>::arg3_type, 
				typename boost::function_traits<Signature>::arg4_type>, 
				wrapper, _1, _2, _3, _4));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 4);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
					args[0].cast<typename boost::function_traits<Signature>::arg1_type>(),
					args[1].cast<typename boost::function_traits<Signature>::arg2_type>(),
					args[2].cast<typename boost::function_traits<Signature>::arg3_type>(),
					args[3].cast<typename boost::function_traits<Signature>::arg4_type>()
					);
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3, typename boost::function_traits<Signature>::arg4_type arg4) const
			{
				this->getSignal(instance)(arg1,arg2,arg3,arg4);
			}

		};
        
        template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<5, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
            :EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute5<
                                                         typename boost::function_traits<Signature>::arg1_type,
                                                         typename boost::function_traits<Signature>::arg2_type,
                                                         typename boost::function_traits<Signature>::arg3_type,
                                                         typename boost::function_traits<Signature>::arg4_type,
                                                         typename boost::function_traits<Signature>::arg5_type>,
                                                         wrapper, _1, _2, _3, _4, _5));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 5);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
                                   args[0].cast<typename boost::function_traits<Signature>::arg1_type>(),
                                   args[1].cast<typename boost::function_traits<Signature>::arg2_type>(),
                                   args[2].cast<typename boost::function_traits<Signature>::arg3_type>(),
                                   args[3].cast<typename boost::function_traits<Signature>::arg4_type>(),
                                   args[4].cast<typename boost::function_traits<Signature>::arg5_type>()
                                   );
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3, typename boost::function_traits<Signature>::arg4_type arg4, typename boost::function_traits<Signature>::arg5_type arg5) const
			{
				this->getSignal(instance)(arg1,arg2,arg3,arg4,arg5);
			}
            
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<6, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute6<
					typename boost::function_traits<Signature>::arg1_type,
					typename boost::function_traits<Signature>::arg2_type,
					typename boost::function_traits<Signature>::arg3_type,
					typename boost::function_traits<Signature>::arg4_type,
					typename boost::function_traits<Signature>::arg5_type,
					typename boost::function_traits<Signature>::arg6_type>,
					wrapper, _1, _2, _3, _4, _5, _6));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 6);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
					args[0].cast<typename boost::function_traits<Signature>::arg1_type>(),
					args[1].cast<typename boost::function_traits<Signature>::arg2_type>(),
					args[2].cast<typename boost::function_traits<Signature>::arg3_type>(),
					args[3].cast<typename boost::function_traits<Signature>::arg4_type>(),
					args[4].cast<typename boost::function_traits<Signature>::arg5_type>(),
					args[5].cast<typename boost::function_traits<Signature>::arg6_type>()
					);
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3, typename boost::function_traits<Signature>::arg4_type arg4, typename boost::function_traits<Signature>::arg5_type arg5, typename boost::function_traits<Signature>::arg6_type arg6) const
			{
				this->getSignal(instance)(arg1,arg2,arg3,arg4,arg5,arg6);
			}

		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class EventDescImpl<7, EventClass, Signature, SignalType, SignalGetter> : public EventDescBase<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			EventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDescBase<EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			rbx::signals::connection connectGeneric(DescribedBase* source, shared_ptr<GenericSlotWrapper> wrapper) const {
				return this->connect(source, boost::bind(&GenericSlotWrapper::execute7<
					typename boost::function_traits<Signature>::arg1_type,
					typename boost::function_traits<Signature>::arg2_type,
					typename boost::function_traits<Signature>::arg3_type,
					typename boost::function_traits<Signature>::arg4_type,
					typename boost::function_traits<Signature>::arg5_type,
					typename boost::function_traits<Signature>::arg6_type,
					typename boost::function_traits<Signature>::arg7_type>,
					wrapper, _1, _2, _3, _4, _5, _6, _7));
			}
			inline void fireEvent(DescribedBase* source, const EventArguments& args) const
			{
				RBX_SIGNALS_ASSERT(args.size() == 7);
				EventClass* e = source->fastDynamicCast<EventClass>();
				this->getSignal(e)(
					args[0].cast<typename boost::function_traits<Signature>::arg1_type>(),
					args[1].cast<typename boost::function_traits<Signature>::arg2_type>(),
					args[2].cast<typename boost::function_traits<Signature>::arg3_type>(),
					args[3].cast<typename boost::function_traits<Signature>::arg4_type>(),
					args[4].cast<typename boost::function_traits<Signature>::arg5_type>(),
					args[5].cast<typename boost::function_traits<Signature>::arg6_type>(),
					args[6].cast<typename boost::function_traits<Signature>::arg7_type>()
					);
			}
			void fireEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3, typename boost::function_traits<Signature>::arg4_type arg4, typename boost::function_traits<Signature>::arg5_type arg5, typename boost::function_traits<Signature>::arg6_type arg6, typename boost::function_traits<Signature>::arg7_type arg7) const
			{
				this->getSignal(instance)(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
			}

		};


		// This is the final class used for defining Events.
		template<
			class EventClass,	// The class type that fires the event
			typename Signature, // The signature of the event
			typename SignalType = rbx::signal<Signature>, // Optional specialization of the signal type
			typename SignalGetter = SignalType EventClass::*  // Optional signal getter, raw member pointer by default
		>
		class EventDesc : public EventDescImpl<boost::function_traits<Signature>::arity, EventClass, Signature, SignalType, SignalGetter>
		{
		public:
			EventDesc(SignalGetter sig, const char* name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<0, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 0);
			}
			EventDesc(SignalGetter sig, const char* name, Descriptor::Attributes attributes)
				:EventDescImpl<0, EventClass, Signature, SignalType, SignalGetter>(sig, name, Security::None, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 0);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<1, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 1);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, Descriptor::Attributes attributes)
				:EventDescImpl<1, EventClass, Signature, SignalType, SignalGetter>(sig, name, Security::None, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 1);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<2, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 2);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
				SignatureDescriptor::Item arg2(RBX::Name::declare(arg2name), &Type::singleton<typename boost::function_traits<Signature>::arg2_type>());
				this->signature.arguments.push_back(arg2);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<3, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 3);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
				SignatureDescriptor::Item arg2(RBX::Name::declare(arg2name), &Type::singleton<typename boost::function_traits<Signature>::arg2_type>());
				this->signature.arguments.push_back(arg2);
				SignatureDescriptor::Item arg3(RBX::Name::declare(arg3name), &Type::singleton<typename boost::function_traits<Signature>::arg3_type>());
				this->signature.arguments.push_back(arg3);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<4, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 4);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
				SignatureDescriptor::Item arg2(RBX::Name::declare(arg2name), &Type::singleton<typename boost::function_traits<Signature>::arg2_type>());
				this->signature.arguments.push_back(arg2);
				SignatureDescriptor::Item arg3(RBX::Name::declare(arg3name), &Type::singleton<typename boost::function_traits<Signature>::arg3_type>());
				this->signature.arguments.push_back(arg3);
				SignatureDescriptor::Item arg4(RBX::Name::declare(arg4name), &Type::singleton<typename boost::function_traits<Signature>::arg4_type>());
				this->signature.arguments.push_back(arg4);
			}
            EventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
            :EventDescImpl<5, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 5);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
				SignatureDescriptor::Item arg2(RBX::Name::declare(arg2name), &Type::singleton<typename boost::function_traits<Signature>::arg2_type>());
				this->signature.arguments.push_back(arg2);
				SignatureDescriptor::Item arg3(RBX::Name::declare(arg3name), &Type::singleton<typename boost::function_traits<Signature>::arg3_type>());
				this->signature.arguments.push_back(arg3);
				SignatureDescriptor::Item arg4(RBX::Name::declare(arg4name), &Type::singleton<typename boost::function_traits<Signature>::arg4_type>());
				this->signature.arguments.push_back(arg4);
                SignatureDescriptor::Item arg5(RBX::Name::declare(arg5name), &Type::singleton<typename boost::function_traits<Signature>::arg5_type>());
				this->signature.arguments.push_back(arg5);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, const char* arg6name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<6, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 6);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
				SignatureDescriptor::Item arg2(RBX::Name::declare(arg2name), &Type::singleton<typename boost::function_traits<Signature>::arg2_type>());
				this->signature.arguments.push_back(arg2);
				SignatureDescriptor::Item arg3(RBX::Name::declare(arg3name), &Type::singleton<typename boost::function_traits<Signature>::arg3_type>());
				this->signature.arguments.push_back(arg3);
				SignatureDescriptor::Item arg4(RBX::Name::declare(arg4name), &Type::singleton<typename boost::function_traits<Signature>::arg4_type>());
				this->signature.arguments.push_back(arg4);
				SignatureDescriptor::Item arg5(RBX::Name::declare(arg5name), &Type::singleton<typename boost::function_traits<Signature>::arg5_type>());
				this->signature.arguments.push_back(arg5);
				SignatureDescriptor::Item arg6(RBX::Name::declare(arg6name), &Type::singleton<typename boost::function_traits<Signature>::arg6_type>());
				this->signature.arguments.push_back(arg6);
			}
			EventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, const char* arg6name, const char* arg7name, Security::Permissions security = Security::None, Descriptor::Attributes attributes = Descriptor::Attributes())
				:EventDescImpl<7, EventClass, Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 7);
				SignatureDescriptor::Item arg1(RBX::Name::declare(arg1name), &Type::singleton<typename boost::function_traits<Signature>::arg1_type>());
				this->signature.arguments.push_back(arg1);
				SignatureDescriptor::Item arg2(RBX::Name::declare(arg2name), &Type::singleton<typename boost::function_traits<Signature>::arg2_type>());
				this->signature.arguments.push_back(arg2);
				SignatureDescriptor::Item arg3(RBX::Name::declare(arg3name), &Type::singleton<typename boost::function_traits<Signature>::arg3_type>());
				this->signature.arguments.push_back(arg3);
				SignatureDescriptor::Item arg4(RBX::Name::declare(arg4name), &Type::singleton<typename boost::function_traits<Signature>::arg4_type>());
				this->signature.arguments.push_back(arg4);
				SignatureDescriptor::Item arg5(RBX::Name::declare(arg5name), &Type::singleton<typename boost::function_traits<Signature>::arg5_type>());
				this->signature.arguments.push_back(arg5);
				SignatureDescriptor::Item arg6(RBX::Name::declare(arg6name), &Type::singleton<typename boost::function_traits<Signature>::arg6_type>());
				this->signature.arguments.push_back(arg6);
				SignatureDescriptor::Item arg7(RBX::Name::declare(arg7name), &Type::singleton<typename boost::function_traits<Signature>::arg7_type>());
				this->signature.arguments.push_back(arg7);
			}
		};

		// A light-weight convenience class that associates a EventDescriptor
		// with a described object to create a "Event"
		class Event
		{
		protected:
			const EventDescriptor* descriptor;
			shared_ptr<DescribedBase> instance;

		public:
			inline Event(const EventDescriptor& descriptor, const shared_ptr<DescribedBase>& instance)
				:descriptor(&descriptor),instance(instance)
			{}

			inline Event(const Event& other)
				:descriptor(other.descriptor),instance(other.instance)
			{}
			inline Event& operator =(const Event& other)
			{
				this->descriptor = other.descriptor;
				this->instance = other.instance;
				return *this;
			}

			inline const RBX::Name& getName() const { 
				return *descriptor->name; 
			}

			inline const EventDescriptor* getDescriptor() const { 
				return descriptor; 
			}
			inline shared_ptr<DescribedBase> getInstance() const
			{
				return instance;
			}

			bool operator==(const Event& other) const {
				return this->descriptor == other.descriptor && this->instance == other.instance;
			}
			bool operator!=(const Event& other) const {
				return !operator==(other);
			}


		};


		std::size_t hash_value(const Event& prop);

		// class used to hold the arguments/EventDesc of an event
		class EventInvocation
		{
		public:
			const Event event;
			EventArguments args;

			EventInvocation(const Event& event, const EventArguments& args) 
				: event(event)
				, args(args)
			{}
			EventInvocation(const Event& event)
				: event(event)
				, args()
			{}

			void fireEvent()
			{
				event.getDescriptor()->fireEvent(event.getInstance().get(), args);
			}

			void replicateEvent()
			{
				event.getDescriptor()->sendEvent(event.getInstance().get(), args);
			}	

			bool operator==(const EventInvocation& other) const {
				return this->event == other.event/* && this->args == other.args*/;
			}
			bool operator!=(const EventInvocation& other) const {
				return this->event != other.event/* || this->args == other.args*/;
			}

		};


		template<int arity, class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl;
		
		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<0,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalGetter sig, const char* name, Security::Permissions security, Descriptor::Attributes attributes)
				: EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, security, attributes)
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance)
			{
				this->getSignal(instance)();
				replicateEvent(instance);
			}
			void replicateEvent(DescribedBase* instance)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 0);
				EventArguments args(0);

				instance->raiseEventInvocation(*this, args);
			}
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<1,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalGetter sig, const char* name, const char* arg1name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1)
			{
				this->fireEvent(instance, arg1);
				replicateEvent(instance, arg1);
			}

			void replicateEvent(DescribedBase* instance, typename boost::function_traits<Signature>::arg1_type arg1)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 1);
				EventArguments args(1);
				args[0] = arg1;

				instance->raiseEventInvocation(*this, args);
			}
			
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<2,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, arg2name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2)
			{
				this->fireEvent(instance, arg1,arg2);
				replicateEvent(instance, arg1, arg2);
			}

			void replicateEvent(DescribedBase* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 2);
				EventArguments args(2);
				args[0] = arg1;
				args[1] = arg2;

				instance->raiseEventInvocation(*this, args);
			}
			
		};
		
		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<3,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, arg2name, arg3name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3)
			{
				this->fireEvent(instance, arg1,arg2,arg3);
				replicateEvent(instance, arg1, arg2, arg3);
			}

			void replicateEvent(EventSource* instance, typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, typename boost::function_traits<Signature>::arg3_type arg3)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 3);
				EventArguments args(3);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;

				instance->raiseEventInvocation(*this, args);
			}			
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<4,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalType EventClass::*sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3,
				typename boost::function_traits<Signature>::arg4_type arg4)
			{
				this->fireEvent(instance, arg1,arg2,arg3,arg4);
				replicateEvent(instance, arg1, arg2, arg3, arg4);
			}

			void replicateEvent(EventSource* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3, 
				typename boost::function_traits<Signature>::arg4_type arg4)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 4);
				EventArguments args(4);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;

				instance->raiseEventInvocation(*this, args);
			}			
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<5,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalType EventClass::*sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, arg5name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3,
				typename boost::function_traits<Signature>::arg4_type arg4,
				typename boost::function_traits<Signature>::arg5_type arg5)
			{
				this->fireEvent(instance, arg1,arg2,arg3,arg4,arg5);
				replicateEvent(instance, arg1, arg2, arg3, arg4, arg5);
			}

			void replicateEvent(EventSource* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3, 
				typename boost::function_traits<Signature>::arg4_type arg4,
				typename boost::function_traits<Signature>::arg5_type arg5)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 5);
				EventArguments args(5);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
				args[4] = arg5;

				instance->raiseEventInvocation(*this, args);
			}			
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<6,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalType EventClass::*sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, const char* arg6name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, arg5name, arg6name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3,
				typename boost::function_traits<Signature>::arg4_type arg4,
				typename boost::function_traits<Signature>::arg5_type arg5,
				typename boost::function_traits<Signature>::arg6_type arg6)
			{
				this->fireEvent(instance, arg1,arg2,arg3,arg4,arg5,arg6);
				replicateEvent(instance, arg1, arg2, arg3, arg4, arg5, arg6);
			}

			void replicateEvent(EventSource* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3, 
				typename boost::function_traits<Signature>::arg4_type arg4,
				typename boost::function_traits<Signature>::arg5_type arg5,
				typename boost::function_traits<Signature>::arg6_type arg6)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 6);
				EventArguments args(6);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
				args[4] = arg5;
				args[5] = arg6;

				instance->raiseEventInvocation(*this, args);
			}			
		};

		template<class EventClass, typename Signature, typename SignalType, typename SignalGetter>
		class RemoteEventDescImpl<7,EventClass,Signature,SignalType,SignalGetter> : public EventDesc<EventClass, Signature, SignalType, SignalGetter>
		{
		protected:
			RemoteEventDescImpl(SignalType EventClass::*sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, const char* arg6name, const char* arg7name, Security::Permissions security, Descriptor::Attributes attributes)
				:EventDesc<EventClass,Signature, SignalType, SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, arg5name, arg6name, arg7name, security, attributes) 
			{}
		public:
			void fireAndReplicateEvent(EventClass* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3,
				typename boost::function_traits<Signature>::arg4_type arg4,
				typename boost::function_traits<Signature>::arg5_type arg5,
				typename boost::function_traits<Signature>::arg6_type arg6,
				typename boost::function_traits<Signature>::arg7_type arg7)
			{
				this->fireEvent(instance, arg1,arg2,arg3,arg4,arg5,arg6,arg7);
				replicateEvent(instance, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
			}

			void replicateEvent(EventSource* instance, 
				typename boost::function_traits<Signature>::arg1_type arg1, 
				typename boost::function_traits<Signature>::arg2_type arg2, 
				typename boost::function_traits<Signature>::arg3_type arg3, 
				typename boost::function_traits<Signature>::arg4_type arg4,
				typename boost::function_traits<Signature>::arg5_type arg5,
				typename boost::function_traits<Signature>::arg6_type arg6,
				typename boost::function_traits<Signature>::arg7_type arg7)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 7);
				EventArguments args(7);
				args[0] = arg1;
				args[1] = arg2;
				args[2] = arg3;
				args[3] = arg4;
				args[4] = arg5;
				args[5] = arg6;
				args[6] = arg7;

				instance->raiseEventInvocation(*this, args);
			}			
		};

		
		class RemoteEventCommon
		{
		public:
			typedef enum { 
				SCRIPTING		= 1,
				REPLICATE_ONLY	= 0
			} Functionality;

			struct Attributes : public Descriptor::Attributes
			{
				Functionality flags;

				Attributes(Functionality flags):flags(flags) {}
				static Attributes deprecated(Functionality flags, const MemberDescriptor* preferred);
			};
			typedef enum { 
				CLIENT_SERVER	= 0,			// Used to communicate between clients and servers
				BROADCAST		= 1				// A full broadcasts, all clients will recieve the message
			} Behavior;
		};

		template<
			class EventClass,	// The class type that fires the event
			typename Signature, // The signature of the event
			typename SignalType = rbx::remote_signal<Signature>, // Optional specialization of the signal type
			typename SignalGetter = SignalType EventClass::*  // Optional signal getter, raw member pointer by default
		>
		class RemoteEventDesc : public RemoteEventDescImpl<boost::function_traits<Signature>::arity, EventClass, Signature, SignalType, SignalGetter>
			, public RemoteEventCommon
		{
		protected:
			const RemoteEventCommon::Behavior behavior;
			const RemoteEventCommon::Functionality flags;
		public:
			RemoteEventDesc(SignalGetter sig, const char* name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<0,EventClass,Signature,SignalType,SignalGetter>(sig, name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 0);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<1,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, security, attributes) 
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 1);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<2,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, arg2name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 2);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<3,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, arg2name, arg3name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 3);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<4,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 4);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name ,const char* arg5name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<5,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, arg5name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 5);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, const char* arg6name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<6,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, arg5name, arg6name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 6);
			}
			RemoteEventDesc(SignalGetter sig, const char* name, const char* arg1name, const char* arg2name, const char* arg3name, const char* arg4name, const char* arg5name, const char* arg6name, const char* arg7name, Security::Permissions security, RemoteEventCommon::Attributes attributes, RemoteEventCommon::Behavior behavior)
				:RemoteEventDescImpl<7,EventClass,Signature,SignalType,SignalGetter>(sig, name, arg1name, arg2name, arg3name, arg4name, arg5name, arg6name, arg7name, security, attributes)
				,flags(attributes.flags)
				,behavior(behavior)
			{
				BOOST_STATIC_ASSERT(boost::function_traits<Signature>::arity == 7);
			}

			SignalType* getSignalPtr(DescribedBase* source)
			{
				if (source)
				{
					EventClass* e = source->fastDynamicCast<EventClass>();

					return &this->getSignal(e);
				}
				RBXASSERT(0);
				return NULL;
			}

			/*override*/ bool isScriptable() const 
			{ 
				return (flags & 1) != 0; 
			}

			/*override*/ bool isBroadcast() const 
			{ 
				return (behavior & 1) != 0; 
			}

			/*override*/ void sendEvent(EventSource* instance, const EventArguments& args) const
			{
				instance->raiseEventInvocation(*this, args);
			}
		};
	}
}
