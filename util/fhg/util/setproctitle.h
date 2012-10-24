#ifndef FHG_UTIL_SETPROC_TITLE_H
#define FHG_UTIL_SETPROC_TITLE_H

#ifdef __cplusplus
extern "C" {
#endif

int setproctitle (const char *title, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
