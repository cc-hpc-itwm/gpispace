#ifndef FHG_UTIL_SETPROC_TITLE_H
#define FHG_UTIL_SETPROC_TITLE_H

#ifdef __cplusplus
extern "C" {
#endif

  /** Lets you change the program name displayed by top and ps and such.

      beware: the implementation  uses the 'exec' syscall, so  make sure to call
      it at the beginning of your program.

      @param  title the  new program  title,  if it  is already  the same,  this
      function is a nop.

      @param argc the original argc
      @param argv the original argv
   */
  int setproctitle (const char *title, int argc, char * const argv[]);

#ifdef __cplusplus
}
#endif

#endif
