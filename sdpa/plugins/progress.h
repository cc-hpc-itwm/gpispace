#ifndef SDPA_PROGRESS_API_H
#define SDPA_PROGRESS_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  size_t set_progress(const char *name, size_t value);
  size_t get_progress(const char *name);

  /**
     Initializes a progress counter.

     The current value will be set to 0 and the maximum number of expected steps
     will be set to 'max'.

     @param name the name of the progress counter
     @param max the expected maximum value
   */
  int progress_initialize ( const char *name
                          , size_t max
                          );

  /**
     Get the current value of the progress counter.

     @param name the name of the progress counter
     @param value store the current value in *value
     @return -ESRCH when not found, -EINVAL when not a progress counter
   */
  int progress_current (const char *name, size_t * value);

  /**
     Count one tick on the given progress counter.

     The current value of the counter will be increased by one.

     @param name the name of the progress counter
     @return -EINVAL when not a counter
   */
  int progress_tick (const char *name);

  /**
     Count 'inc' ticks on the given progress counter.

     The current  value of the counter  will be increased by  'inc'. The counter
     does not check for overflow, it may happen that it over-counts.

     @param name the name of the progress counter
     @return -EINVAL when not a counter
   */
  int progress_n_tick (const char *name, int inc);

  /**
     Set a progress counter to its maximum value.

     @return -EINVAL when not a counter
   */
  int progress_finalize (const char *name);

#ifdef __cplusplus
}
#endif

#endif
