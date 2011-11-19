#ifndef SDPA_PROGRESS_API_H
#define SDPA_PROGRESS_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  int set_progress(const char *name, size_t value);
  int get_progress(const char *name);

#ifdef __cplusplus
}
#endif

#endif
