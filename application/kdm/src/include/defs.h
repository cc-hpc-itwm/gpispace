/***************************************************************************
                          defs.h  -  description
                             -------------------
    begin                : Wed Dec 7 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/
#ifndef DEFS_H
#define DEFS_H

#define GRT_VERSION "2.0.3"
#define SEISRAY_VERSION "2.0.3"
#define REFTOMO_VERSION "1.0"

enum ERROR_TYPE {OK_GE, FATAL_ERROR_GE, WARNING_GE};

#ifdef __GNUC__
#define myaligned(var, a) var  __attribute__ ((aligned(a)))
#elif defined __INTEL_COMPILER
#define myaligned(var, a) __declspec(align(a))
#endif

#ifndef __SSE__
#define __SSE__
#endif

// defines for abstracted logging access
// need to inlcude since scons can not trace dependencies via define
//#include "utils/loggingclass.h"

#endif //DEFS_H
