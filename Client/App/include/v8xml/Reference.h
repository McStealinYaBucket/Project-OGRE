
#ifndef _568E368F53F1431aB7D4923F6D45021A
#define _568E368F53F1431aB7D4923F6D45021A

#include "rbx/Debug.h"

#include "Util/Object.h"
#include "Util/Handle.h"

#include <list>
#include <map>
#include <set>
#include <string>

class XmlNameValuePair;

namespace RBX {

	namespace Reflection
	{
		class DescribedBase;
	}
	
	// Used for streaming
	class RBXInterface IIDREF
	{
	private:
		friend class IReferenceBinder;
		virtual void assignIDREF(Reflection::DescribedBase* propertyOwner, const InstanceHandle& handle) const = 0;
	};

	// Used for streaming
	class RBXBaseClass IReferenceBinder
	{
	public:
		virtual void announceID(const XmlNameValuePair* valueID, Reflection::DescribedBase* source) = 0;
		virtual void announceIDREF(const XmlNameValuePair* valueIDREF, Reflection::DescribedBase* propertyOwner, const IIDREF* idref) = 0;
		virtual bool resolveRefs() = 0;
		virtual ~IReferenceBinder(){}
	protected:
		void assign(const IIDREF* idref, Reflection::DescribedBase* propertyOwner, const InstanceHandle& handle) {
			idref->assignIDREF(propertyOwner, handle);
		}
	};

	// MergeBinder is used to merge an XML stream into an existing world (for undo/redo operations).
	// Inline bodies verified identical to the Oct 2013 binary (processID 0x5B1404,
	// processIDREF 0x5B1290, resolveRefs 0x5B1970, announceID 0x5B0F9A/IDREF 0x5B0F98).
	class MergeBinder : public IReferenceBinder
	{
		struct IDREFItem {
			const IIDREF* idref;
			Reflection::DescribedBase* propertyOwner;
			RBX::InstanceHandle value;
		};

		// TODO: vector or list???
		std::vector<IDREFItem> deferredIDREFItems;

	public:
		virtual void announceID(const XmlNameValuePair* valueID, Reflection::DescribedBase* target) {
			processID(valueID, target);
		}
		// STUB: PLAYER 0x00447460
		// STUB: STUDIO 0x00435250
		virtual void announceIDREF(const XmlNameValuePair* valueIDREF, Reflection::DescribedBase* propertyOwner, const IIDREF* idref) {
			bool processedIDREF = processIDREF(valueIDREF, propertyOwner, idref);
			RBXASSERT(processedIDREF);
		}

		// FUNCTION: PLAYER 0x004474c0
		// FUNCTION: STUDIO 0x004360b0
		virtual bool resolveRefs() {
			for (std::vector<IDREFItem>::iterator iter = deferredIDREFItems.begin(); iter!=deferredIDREFItems.end(); ++iter)
			{
				assign(iter->idref, iter->propertyOwner, iter->value);


			}
			deferredIDREFItems.clear();

			return true;
		}

	protected:
		// STUB: PLAYER 0x00447530
		// STUB: STUDIO 0x00435430
		virtual bool processID(const XmlNameValuePair* valueID, Reflection::DescribedBase* source) {
			RBX::InstanceHandle h;
			if (valueID->getValue(h)) {
				h.linkTo(shared_from(source));
				return true;
			} else if (valueID->isValueEqual(&value_IDREF_nil))
				return true;		// "nil" means "skip this value", like xsi:nil does for other types
			else
				return false;
		}
		// STUB: PLAYER 0x004476a0
		// STUB: STUDIO 0x00435060
		virtual bool processIDREF(const XmlNameValuePair* valueIDREF, Reflection::DescribedBase* propertyOwner, const IIDREF* idref) {
			RBX::InstanceHandle value;
			if (valueIDREF->getValue(value)) {
				if (!value.empty())
					assign(idref, propertyOwner, value);
				else {
					IDREFItem item = {idref, propertyOwner, value};
					deferredIDREFItems.push_back(item);
				}
				return true;
			} else if (valueIDREF->isValueEqual(&value_IDREF_nil))
				return true;		// "nil" means "skip this value", like xsi:nil does for other types
			else
				return false;
		}
	};

}

#endif
