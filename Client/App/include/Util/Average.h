/* Util/Average.h -- October 2013.
   Evidence: Average<double>::Average(size_t, double) PLAYER 0x0084d260 (ret 0xc: samples at +0,
     tag 0 at +4, std::vector<double> at +8 resized to samples copies of the value through
     _Insert_n 0x0084d0a0 / erase 0x0084d020); the DataModel dtor frees the vector's buffer
     inline (0x0064da95, PhysicsInstructions+0x24). Size 0x18.
   Names from the 2016 Util/Average.h; only the constructor is confirmed by 2013 bytes. */

#pragma once

#include <vector>

namespace RBX {

	// SIZE 0x18
	template<typename T>
	class Average
	{
	private:
		size_t samples;			// 0x00
		size_t tag;				// 0x04
		std::vector<T> history;	// 0x08

	public:
		Average(size_t samples, T initValue)
			: samples(samples)
			, tag(0)
		{
			history.resize(samples, initValue);
		}
	};

} // namespace RBX
