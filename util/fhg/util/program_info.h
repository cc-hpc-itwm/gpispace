#ifndef FHG_UTIL_PROGRAM_INFO_H
#define FHG_UTIL_PROGRAM_INFO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  /** Discovers the full path to the binary of the current program. */
  int fhg_get_executable_path (char *path, size_t len);

#ifdef __cplusplus
}
#endif

#endif
