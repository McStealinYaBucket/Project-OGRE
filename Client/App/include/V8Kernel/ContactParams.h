/* V8Kernel/ContactParams.h -- October 2013 (contact material params).
   2013 shape is 12B (ContactConnector C2 copies 12B verbatim); 2016 has
   16B (adds kElasticity). The absent 2013 member is unidentified --
   modeled as kSpring/kNeg/kFriction (C2 zeroes; the C2 copy is
   byte-blind so behavior is exact regardless).
   Original file: Client/App/include/V8Kernel/ContactParams.h (path
   from 2016). */

#pragma once

namespace RBX {

	class ContactParams {
	public:
		float kSpring;
		float kNeg;
		float kFriction;

		ContactParams()
			: kSpring(0.0f)
			, kNeg(0.0f)
			, kFriction(0.0f)
		{}
	};

} // namespace RBX
