
#ifndef BOOL_H
#define BOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOOL_T
#define BOOL_T
  typedef enum
  { False = 0,
    True
  } Bool_t, *PBool_t;
#endif

#ifdef __cplusplus
}
#endif

#endif
