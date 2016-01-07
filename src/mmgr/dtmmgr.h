
/* double table for two arenas, a global, growing upwards and a local
   growing downwards

   interface like in tmmgr, but each handle has to specify an arena
*/

#pragma once

#include <mmgr/tmmgr.h>

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

  extern MemSize_t dtmmgr_memfree (const DTmmgr_t);

  extern Count_t dtmmgr_numhandle (const DTmmgr_t, const Arena_t);

  /* *********************************************************************** */

#ifndef DTMMGR_ERROR_HANDLER
#include <mmgr/error.h>
#define DTMMGR_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"DTmmgr",fun)
#endif

#ifndef DTMMGR_ERROR_MALLOC_FAILED
#define DTMMGR_ERROR_MALLOC_FAILED DTMMGR_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif
