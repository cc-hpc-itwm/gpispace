/*! \file Univeral.h
	@brief This file contains basic define and include directives.*/


#ifndef UNIVERSAL_H
#define UNIVERSAL_H



//all includes
//#include <emmintrin.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <memory.h>
//#include <mmintrin.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <xmmintrin.h>
#include <float.h>

#include <vector>
#include <iostream>
#include <exception>
#include <list>


/*/ in order to switch of debug output with printf, remove comments
#define printf  printf_own
#ifdef __cplusplus
extern "C" {
#endif

  inline void printf_own(const char *fmt, ...) { };

#ifdef __cplusplus
}
#endif
//*/


#ifdef WIN32

#include <io.h>
#include <process.h>
#include <windows.h>

#else

#include <pthread.h>
#include <sched.h>


#define  O_BINARY	0

#endif



//defines

#define ALIGN16					__declspec(align(16))
#define ALIGN32					__declspec(align(32))


#define MAX(a,b)				(((a)<(b)) ? (b) : (a))
#define MIN(a,b)				(((a)>(b)) ? (b) : (a))

#define DTOR					0.0174532925
#define RTOD					57.2957795

#ifdef WIN32
#define M_PI					(3.14159265358979323846)
#endif

#define EPS						1.0e-4;


#endif
