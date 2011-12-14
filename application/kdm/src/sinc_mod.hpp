#ifndef KDM_SINC_MOD_HPP
#define KDM_SINC_MOD_HPP 1

#include <vector>
#include <cassert>
#include "utils/sincinterpolator.h"

#ifdef __cplusplus
extern "C"
{
  extern SincInterpolator * getSincInterpolator (unsigned int thread_id, float dt);
  extern void resizeSincInterpolatorArray (unsigned int ntid);
}
#endif

#endif
