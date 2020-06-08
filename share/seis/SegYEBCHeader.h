#pragma once

#include <algorithm>

/**
 *@author Dirk Merten
 */
struct SegYEBCHeader{	/* bhed - binary header */
    char desc[3200];
  ~SegYEBCHeader () { std::fill (desc, desc+3200, 0); }
};
