/* V8DataModel/StockSound.h -- October 2013 STUB (rebased to Soundscape::SoundChannel).
   Evidence: FactoryProduct<StockSound, Soundscape::SoundChannel> 0x247452
     (Creator 0x24A1D0; StockSound = a DescribedCreatable over SoundChannel,
     RUNTIME_LOCAL). The actual class body (loadStockSound arms) is landed in
     App/util/Sound.cpp (SoundService::loadStockSound).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StockSound.h */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/SoundChannel.h"

namespace RBX {

	extern const char* const sStockSound;

	class StockSound
		: public DescribedCreatable<StockSound, Soundscape::SoundChannel, sStockSound, Reflection::ClassDescriptor::RUNTIME_LOCAL>
	{
	public:
		StockSound() {}
	};

} // namespace RBX