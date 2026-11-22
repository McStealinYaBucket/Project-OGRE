/* V8DataModel/EventReplicator.h -- October 2013.
   The 2013 assert path is include\V8DataModel/EventReplicator.h (setListenerMode's
   RBXASSERT(connectionSignal.empty()) at line 72, PLAYER 0x007c2e80 for <GuiObject, void(int,int)>);
   the file is laid out so it lands there. 2016 keeps the same file
   (App/include/v8datamodel/EventReplicator.h).
   A remote event fires across the network only while somebody listens to it. The 2013 shape is the
   2016 one (RTTI: EventReplicatorBase / EventReplicatorImpl<N> / EventReplicator, e.g. the
   callable_slot RTTI names over EventReplicatorBase<GuiObject, void(int,int)>::listenerConnectionAdded).
   Layout from the GuiObject ctor 0x007c77c0 (0x1c bytes each, replicator at GuiObject+0xd0):
     +0x00 vftable (connectSignalListener; EventReplicator<GuiObject, void(int,int)> 0x00b66ad0),
     +0x04 connectionCount (the BoundProp global), +0x08 instance (NULL, set last by setInstance),
     +0x0c remoteEvent (the RemoteEventDesc global), +0x10 connectionSignal (remote_signal + 4),
     +0x14 listenerConnection, +0x18 signalConnection.
   ~EventReplicatorBase (PLAYER 0x006bc300 for <GuiObject, void(int,int)>) is user-declared: it stores
   EventReplicatorBase's vftable 0x00b3a5b8 and disconnects both connections. */

#pragma once

#include "rbx/signal.h"
#include "rbx/Debug.h"
#include "reflection/property.h"
#include "reflection/Event.h"

#include <boost/bind.hpp>
#include <boost/type_traits/function_traits.hpp>

namespace RBX {

	template<typename Parent, typename Signature>
	class EventReplicatorBase
	{
	protected:
		Reflection::BoundProp<int>& connectionCount;
		Parent* instance;
		Reflection::RemoteEventDesc<Parent, Signature>& remoteEvent;
		rbx::signal<void()>& connectionSignal;

		rbx::signals::connection listenerConnection;
		void listenerConnectionAdded()
		{
			connectionCount.setValue(instance, std::max(connectionCount.getValue(instance) + 1, 1));
		}

		rbx::signals::connection signalConnection;
		virtual void connectSignalListener() {}

	public:
		EventReplicatorBase(rbx::signal<void()>& connectionSignal,
			Reflection::RemoteEventDesc<Parent, Signature>& remoteEvent,
			Reflection::BoundProp<int>& connectionCount)
			: connectionCount(connectionCount)
			, instance(NULL)
			, remoteEvent(remoteEvent)
			, connectionSignal(connectionSignal)
		{
		}

		~EventReplicatorBase()
		{
			if (listenerConnection.connected())
				listenerConnection.disconnect();
			if (signalConnection.connected())
				signalConnection.disconnect();
		}

		void setInstance(Parent* value) { instance = value; }

		void setListenerMode(bool alreadyConnected)
		{
			if (!listenerConnection.connected())
			{
				RBXASSERT(connectionSignal.empty());
				listenerConnection = connectionSignal.connect(boost::bind(&EventReplicatorBase::listenerConnectionAdded, this));
				if (alreadyConnected)
					listenerConnectionAdded();
			}
		}

		void onPropertyChanged(const Reflection::PropertyDescriptor& descriptor)
		{
			if (!listenerConnection.connected())
			{
				if (descriptor.name == connectionCount.name)
				{
					if (connectionCount.getValue(instance) > 0)
					{
						if (!signalConnection.connected())
							connectSignalListener();
					}
					else if (signalConnection.connected())
					{
						signalConnection.disconnect();
					}
				}
			}
		}
	};

	template<int arity, class Parent, typename Signature>
	class EventReplicatorImpl;

	template<class Parent, typename Signature>
	class EventReplicatorImpl<0, Parent, Signature> : public EventReplicatorBase<Parent, Signature>
	{
	public:
		EventReplicatorImpl(rbx::signal<void()>& connectionSignal,
			Reflection::RemoteEventDesc<Parent, Signature>& remoteEvent,
			Reflection::BoundProp<int>& connectionCount)
			: EventReplicatorBase<Parent, Signature>(connectionSignal, remoteEvent, connectionCount)
		{
		}

	protected:
		void signalProducedIncremented()
		{
			this->remoteEvent.replicateEvent(this->instance);
		}

		/*implement*/ void connectSignalListener()
		{
			this->signalConnection = this->remoteEvent.getSignalPtr(this->instance)->connect(
				boost::bind(&EventReplicatorImpl::signalProducedIncremented, this));
		}
	};

	template<class Parent, typename Signature>
	class EventReplicatorImpl<1, Parent, Signature> : public EventReplicatorBase<Parent, Signature>
	{
	public:
		EventReplicatorImpl(rbx::signal<void()>& connectionSignal,
			Reflection::RemoteEventDesc<Parent, Signature>& remoteEvent,
			Reflection::BoundProp<int>& connectionCount)
			: EventReplicatorBase<Parent, Signature>(connectionSignal, remoteEvent, connectionCount)
		{
		}

	protected:
		void signalProducedIncremented(typename boost::function_traits<Signature>::arg1_type arg1)
		{
			this->remoteEvent.replicateEvent(this->instance, arg1);
		}

		/*implement*/ void connectSignalListener()
		{
			this->signalConnection = this->remoteEvent.getSignalPtr(this->instance)->connect(
				boost::bind(&EventReplicatorImpl<1, Parent, Signature>::signalProducedIncremented, this, _1));
		}
	};

	template<class Parent, typename Signature>
	class EventReplicatorImpl<2, Parent, Signature> : public EventReplicatorBase<Parent, Signature>
	{
	public:
		EventReplicatorImpl(rbx::signal<void()>& connectionSignal,
			Reflection::RemoteEventDesc<Parent, Signature>& remoteEvent,
			Reflection::BoundProp<int>& connectionCount)
			: EventReplicatorBase<Parent, Signature>(connectionSignal, remoteEvent, connectionCount)
		{
		}

	protected:
		void signalProducedIncremented(typename boost::function_traits<Signature>::arg1_type arg1,
			typename boost::function_traits<Signature>::arg2_type arg2)
		{
			this->remoteEvent.replicateEvent(this->instance, arg1, arg2);
		}

		/*implement*/ void connectSignalListener()
		{
			this->signalConnection = this->remoteEvent.getSignalPtr(this->instance)->connect(
				boost::bind(&EventReplicatorImpl<2, Parent, Signature>::signalProducedIncremented, this, _1, _2));
		}
	};

	template<class Parent, typename Signature>
	class EventReplicator : public EventReplicatorImpl<boost::function_traits<Signature>::arity, Parent, Signature>
	{
	public:
		EventReplicator(rbx::signal<void()>& connectionSignal,
			Reflection::RemoteEventDesc<Parent, Signature>& remoteEvent,
			Reflection::BoundProp<int>& connectionCount)
			: EventReplicatorImpl<boost::function_traits<Signature>::arity, Parent, Signature>(connectionSignal, remoteEvent, connectionCount)
		{
		}
	};

} // namespace RBX

#define CHELPER2(a, b) a##b
#define CHELPER3(a, b, c) a##b##c

#define DECLARE_EVENT_REPLICATOR_SIG(Parent, eventName, signature) \
	int CHELPER3(var, eventName, ConnectionCount); \
	static Reflection::BoundProp<int> CHELPER3(prop_, eventName, ConnectionCount); \
	RBX::EventReplicator<Parent, signature> CHELPER2(eventReplicator, eventName);

#define category_EventReplicator "EventReplicator"
#define IMPLEMENT_EVENT_REPLICATOR(Parent, eventDesc, eventText, eventName) \
	Reflection::BoundProp<int> Parent::CHELPER3(prop_, eventName, ConnectionCount)(eventText "ConnectionCount", category_EventReplicator, &Parent::CHELPER3(var, eventName, ConnectionCount), RBX::Reflection::PropertyDescriptor::REPLICATE_ONLY);

#define CONSTRUCT_EVENT_REPLICATOR(Parent, remoteSignalName, eventDesc, eventName) \
	CHELPER3(var, eventName, ConnectionCount)(0) \
	, CHELPER2(eventReplicator, eventName)(remoteSignalName.connectionSignal, eventDesc, CHELPER3(prop_, eventName, ConnectionCount))

#define CONNECT_EVENT_REPLICATOR(eventName) \
	CHELPER2(eventReplicator, eventName).setInstance(this)
