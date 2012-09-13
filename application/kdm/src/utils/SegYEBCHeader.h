/***************************************************************************
                          SegYEBCHeader.h  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#ifndef SEGYEBCHEADER_H
#define SEGYEBCHEADER_H

#include <algorithm>

/**
 *@author Dirk Merten
 */
struct SegYEBCHeader{	/* bhed - binary header */
    char desc[3200];
  ~SegYEBCHeader () { std::fill (desc, desc+3200, 0); }
};
#endif //SEGYEBCHEADER_H

