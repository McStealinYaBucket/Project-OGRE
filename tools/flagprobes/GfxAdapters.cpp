// probe: GfxAdapters (PLAYER 0x609330-0x612550; SceneUpdater.cpp 0x60d020..). No 2013 or 2016 source exists
// for this Ogre-era library, so the probe uses the boost 1.53 unordered_set instantiation its TU emitted
// (identified by body: table copy ctor 0x60cd30 with its EH frame, table swap 0x60b930 with the float mlf_).
#include <boost/unordered_set.hpp>
namespace RBX { class GfxPart; }
template class boost::unordered::unordered_set<RBX::GfxPart*>;
