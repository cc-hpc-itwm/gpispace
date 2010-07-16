#ifndef KDM_SINC_MOD_HPP
#define KDM_SINC_MOD_HPP 1

#include "utils/sincinterpolator.h"

#ifdef __cplusplus
extern "C"
{
  extern SincInterpolator ** SincIntArray();
  extern void initSincIntArray (const unsigned int num_threads, float);
}

#endif

#endif
