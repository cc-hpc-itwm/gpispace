// {petry,rahn}@itwm.fhg.de

#ifndef FHG_UTIL_WFHD_H_53b4553b18374d95a35f85cca99e0040
#define FHG_UTIL_WFHD_H_53b4553b18374d95a35f85cca99e0040

#ifdef __cplusplus
extern "C" {
#endif

  extern int wfhd_wait (const char *info_file);
  extern int wfhd_signal (const char *info_file);
  extern void wfhd_cancel (void*);

#ifdef __cplusplus
}
#endif

#endif
