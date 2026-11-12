/* App/v8datamodel/TouchDebouncer.cpp -- October 2013 RBX::TouchDebouncer::check.

   0x4E32F6, 623 bytes.  The body, in the shape the disasm actually has:

     0x4E330E  function-local static, __cxa_guard protected:
               expiration = 0x4014000000000000 = 5.0 seconds
     0x4E338x  Time now = Time::nowFast();

     size_t i = 0;
     while (i < items.size())
     {
         // -- at a FIXED index i, retried after every erase
         if (items[i].part.get() == part.get())        // 0x4E334C raw compare
         {
             items[i].expiration = now + expiration;   // 0x4E3364 movsd [+12]
             if (items[i].type != type)                // 0x4E33A9 cmp dword [+8]
             {
                 items[i].type = type;
                 return true;
             }
             return false;
         }
         if (now <= items[i].expiration)                // 0x4E3379 comisd
             break;                                     // -> push_back

         // expired: move the LAST element into slot i, then drop the tail.
         // mac: 0x4E33C6: `if (i < size()-1) items[i] = items.back();`
         // mac: 0x4E33EF: `items.resize(size()-1, Item());`
         // ... and then the SAME index is retried (0x4E33F7 reloads v5 from
         // the loop variable without incrementing it).
         if (i + 1 < items.size())
             items[i] = items.back();
         items.pop_back();
     }

     Item item;                                        // 0x4E3420
     item.part       = part;
     item.type       = type;
     item.expiration = now + expiration;
     items.push_back(item);                           // 0x4E3FEE
     return true;

   WHY THE ORDERING IS SAFE: every live entry gets `now + 5.0` and `now` is
   monotonically non-decreasing, so the vector stays sorted by expiration and
   the `now <= expiration` break really does stop the scan -- there is no
   live entry after a dead one.  The erase is the swap-and-pop idiom the
   disasm shows (a single 20-byte copy from the tail, then resize), and
   retrying the same index is what keeps the scan from skipping the element
   that was just moved into it.

   The binary's loop is written as a nested `while` (an outer scan that
   advances and an inner retry at a fixed index).  The two collapse into the
   single `for` above because `pop_back()` leaves `i` unchanged, which is
   exactly the inner retry, and the outer's `i + 1` is the `for`'s `++i`.

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TouchDebouncer.cpp */

#include "stdafx.h"

#include "V8DataModel/TouchDebouncer.h"

namespace RBX {

	bool TouchDebouncer::check(const shared_ptr<PartInstance>& part, TouchPair::Type type)
	{
		// 0x4014000000000000 -- five seconds.  The initializer is written as
		// two 32-bit stores (0x40140000 then 0x00000000) because it is a
		// double, and the guard is __cxa_guard_acquire on a local flag.
		//
		// It has to be an INTERVAL, not a Time: rbx/rbxtime.h's
		// `Time(double)` ctor is protected, so `static Time x = 5.0` cannot
		// be written outside Time.  Time::Interval::from_seconds is the
		// public spelling, and the single `addsd` in the disasm is
		// `Time::operator+(const Interval&)` (rbxtime.h:115) with the
		// const-folded 5.0.
		static Time::Interval expiration = Time::Interval::from_seconds(5.0);

		Time now = Time::nowFast();

		for (size_t i = 0; i < items.size(); ++i)
		{
			// A retry loop at a FIXED index.  It re-runs at i after every
			// erase, because the erase moves the tail element into slot i
			// and that element has not been examined yet.
			while (true)
			{
				// Raw-pointer identity, NOT shared_ptr identity: the disasm
				// compares the two POINTERS and never touches a control
				// block.
				if (items[i].part.get() == part.get())
				{
					// Refreshing the window first means a bouncing part
					// stays alive as long as it keeps touching, and only a
					// genuine Touched/Untouched transition is reported.
					items[i].expiration = now + expiration;

					if (items[i].type != type)
					{
						items[i].type = type;
						return true;
					}
					return false;
				}

				// Still live, so it is not ours to garbage-collect.  This
				// leaves the RETRY loop only -- the outer scan carries on
				// with i+1, which is why a live entry does not stop the
				// search for a later entry.
				if (now <= items[i].expiration)
					break;

				// Dead entry: swap the tail into this hole and drop it.
				if (i + 1 < items.size())
					items[i] = items.back();
				items.pop_back();

				// If i was the tail, the vector is now shorter than i and
				// the retry would run off the end.  The binary tests this
				// right after the resize and jumps straight to the
				// push_back arm; breaking out is the same place, because
				// the outer loop's ++i then fails its `i < size` test.
				if (i >= items.size())
					break;
			}
		}

		Item item;
		item.part = part;
		item.type = type;
		item.expiration = now + expiration;
		items.push_back(item);
		return true;
	}

} // namespace RBX
