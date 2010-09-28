#ifndef KDM_SINC_MOD_HPP
#define KDM_SINC_MOD_HPP 1

#include <vector>
#include <cassert>
#include "utils/sincinterpolator.h"

#ifdef __cplusplus
extern "C"
{
  extern SincInterpolator ** SincIntArray();
  extern void initSincIntArray (const unsigned int num_threads, float);
  extern SincInterpolator * getSincInterpolator (unsigned int thread_id, float dt);
}
#endif

#endif
