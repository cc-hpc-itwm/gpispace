#ifndef SDPA_PROGRESS_API_H
#define SDPA_PROGRESS_API_H

#ifdef __cplusplus
extern "C"
{
#endif

  int set_progress(const char *name, int value);
  int get_progress(const char *name, int *value);

#ifdef __cplusplus
}
#endif

#endif
