
#ifndef ERROR_H
#define ERROR_H

#ifndef GEN_ERROR_HANDLER
#include <stdio.h>
#include <stdlib.h>
#define GEN_ERROR_HANDLER(file,line,msg,fun)                                  \
    do                                                                        \
      {                                                                       \
        fprintf(stderr, "%s [%d]: %s: Error in %s.\n", file, line, msg, fun); \
        exit(EXIT_FAILURE);                                                   \
      }                                                                       \
    while (0)
#endif

#endif
