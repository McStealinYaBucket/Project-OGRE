// Toolchain probe: the OBJECT library half.
#include <vector>

int ProbeSum(const std::vector<int>& v)
{
	int s = 0;
	for (size_t i = 0; i < v.size(); i++)
		s += v[i];
	return s;
}
