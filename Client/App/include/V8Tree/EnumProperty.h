/* V8Tree/EnumProperty.h -- October 2013 reconstruction.
   Holds no declarations: the enum property machinery lives in
   reflection/enumconverter.h (the spelling most 2013 TUs use, see the
   EnumDesc addPair copies in CLAUDE.md). Nothing in the tree includes this
   header; it only forwards to the two headers an enum property needs. */
#pragma once

#include "reflection/enumconverter.h"
#include "V8Tree/Property.h"
