
/* double table for two arenas, a global, growing upwards and a local
   growing downwards

   interface like in tmmgr, but each handle has to specify an arena
*/

#ifndef DTMMGR_H
#define DTMMGR_H

#include <mmgr/tmmgr.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void *DTmmgr_t, *PDTmmgr_t;

  extern void dtmmgr_init (PDTmmgr_t, const MemSize_t, const Align_t);
  extern MemSize_t dtmmgr_finalize (PDTmmgr_t);

  typedef enum
  { ARENA_UP = 0,
    ARENA_DOWN
  } Arena_t;

  extern AllocReturn_t dtmmgr_alloc (PDTmmgr_t, const Handle_t, const Arena_t,
                                     const MemSize_t);
  extern HandleReturn_t dtmmgr_free (PDTmmgr_t, const Handle_t,
                                     const Arena_t);
  extern HandleReturn_t dtmmgr_offset_size (const DTmmgr_t, const Handle_t,
                                            const Arena_t, POffset_t,
                                            PMemSize_t);

  extern MemSize_t dtmmgr_memsize (const DTmmgr_t);
  extern MemSize_t dtmmgr_memfree (const DTmmgr_t);
  extern MemSize_t dtmmgr_memused (const DTmmgr_t);

  extern Count_t dtmmgr_numhandle (const DTmmgr_t, const Arena_t);
  extern Count_t dtmmgr_numalloc (const DTmmgr_t, const Arena_t);
  extern Count_t dtmmgr_numfree (const DTmmgr_t, const Arena_t);
  extern MemSize_t dtmmgr_sumalloc (const DTmmgr_t, const Arena_t);
  extern MemSize_t dtmmgr_sumfree (const DTmmgr_t, const Arena_t);

  extern void dtmmgr_defrag (PDTmmgr_t, const Arena_t, const fMemmove_t,
                             const PMemSize_t, void *);

  /* *********************************************************************** */

  extern void dtmmgr_status (const DTmmgr_t);
  extern void dtmmgr_info (const DTmmgr_t);     /* short */

  /* *********************************************************************** */

#ifndef DTMMGR_ERROR_HANDLER
#include <mmgr/error.h>
#define DTMMGR_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"DTmmgr",fun)
#endif

#ifndef DTMMGR_ERROR_MALLOC_FAILED
#define DTMMGR_ERROR_MALLOC_FAILED DTMMGR_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifdef __cplusplus
}
#endif

#endif
