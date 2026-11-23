/* V8DataModel/NumberSequence.h -- October 2013 reconstruction.
   No 2013 trace: PLAYER has no "NumberSequence" string and iOS 10-29 has no
   symbol for it (the type arrives with ParticleEmitter, after 2013). Nothing
   in this tree includes the header; it is kept only so the path exists, and
   declares the minimal value type: a sorted list of (time, value, envelope)
   keypoints, at most kMaxSize of them.
   Dropped from the 2016 header: the clamping copy constructor and
   resample(), which no 2013 code could call. */
#pragma once

#include <vector>

namespace RBX {

struct NumberSequenceKeypoint
{
	float time;
	float value;
	float envelope;

	NumberSequenceKeypoint() : time(0), value(0), envelope(0) {}
	NumberSequenceKeypoint(float t, float v, float e) : time(t), value(v), envelope(e) {}

	bool operator==(const NumberSequenceKeypoint& other) const;
};

class NumberSequence
{
public:
	typedef NumberSequenceKeypoint Key;
	enum { kMaxSize = 20 };

	explicit NumberSequence(float constant = 0);
	NumberSequence(const std::vector<Key>& points, bool throwOnError = false);

	const std::vector<Key>& getPoints() const { return keys; }
	Key start() const { return keys.front(); }
	Key end() const { return keys.back(); }

	bool operator==(const NumberSequence& other) const;

	static bool validate(const std::vector<Key>& points, bool throwOnError);

private:
	std::vector<Key> keys;
};

} // namespace RBX
