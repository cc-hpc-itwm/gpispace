
#ifndef TMMGR_H
#define TMMGR_H

#include <mmgr/word.h>
#include <mmgr/bool.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void *Tmmgr_t, *PTmmgr_t;

  typedef Word_t MemSize_t, *PMemSize_t;
  typedef Word_t Align_t;

#define FMT_MemSize_t FMT_Word_t
#define FMT_Align_t FMT_Word_t

  extern MemSize_t tmmgr_init (PTmmgr_t, const MemSize_t, const Align_t);
  extern MemSize_t tmmgr_finalize (PTmmgr_t);

  typedef enum
  { RESIZE_SUCCESS,
    RESIZE_BELOW_HIGHWATER,
    RESIZE_BELOW_MEMUSED,
    RESIZE_FAILURE
  } ResizeReturn_t;

  extern ResizeReturn_t tmmgr_resize (PTmmgr_t, const MemSize_t);

  typedef enum
  { ALLOC_SUCCESS,
    ALLOC_DUPLICATE_HANDLE,
    ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY,
    ALLOC_INSUFFICIENT_MEMORY,
    ALLOC_FAILURE
  } AllocReturn_t;

  extern AllocReturn_t tmmgr_alloc (PTmmgr_t, const Handle_t,
                                    const MemSize_t);

  typedef enum
  { RET_SUCCESS,
    RET_HANDLE_UNKNOWN,
    RET_FAILURE
  } HandleReturn_t;

  extern HandleReturn_t tmmgr_free (PTmmgr_t, const Handle_t);
  extern HandleReturn_t tmmgr_offset_size (const Tmmgr_t, const Handle_t,
                                           POffset_t, PMemSize_t);

  extern MemSize_t tmmgr_memsize (const Tmmgr_t);
  extern MemSize_t tmmgr_memfree (const Tmmgr_t);
  extern MemSize_t tmmgr_memused (const Tmmgr_t);
  extern MemSize_t tmmgr_minfree (const Tmmgr_t);
  extern MemSize_t tmmgr_highwater (const Tmmgr_t);

  typedef Word_t Count_t;

#define FMT_Count_t FMT_Word_t

  extern Count_t tmmgr_numhandle (const Tmmgr_t);
  extern Count_t tmmgr_numalloc (const Tmmgr_t);
  extern Count_t tmmgr_numfree (const Tmmgr_t);
  extern MemSize_t tmmgr_sumalloc (const Tmmgr_t);
  extern MemSize_t tmmgr_sumfree (const Tmmgr_t);

  typedef Offset_t OffsetDest_t;
  typedef Offset_t OffsetSrc_t;

#define FMT_OffsetDest_t FMT_Offset_t
#define FMT_OffsetSrc_t FMT_Offset_t

  typedef void (*fMemmove_t) (const OffsetDest_t, const OffsetSrc_t,
                              const MemSize_t, void *);

  /* stops, when enough contiguous memory is available */
  extern void tmmgr_defrag (PTmmgr_t, const fMemmove_t, const PMemSize_t,
                            void *);

  /* *********************************************************************** */

  extern void tmmgr_status (const Tmmgr_t, const char *);
  extern void tmmgr_info (const Tmmgr_t, const char *); /* short */

  /* *********************************************************************** */

#ifndef TMMGR_ERROR_HANDLER
#include <mmgr/error.h>
#define TMMGR_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"Tmmgr",fun)
#endif

#ifndef TMMGR_ERROR_MALLOC_FAILED
#define TMMGR_ERROR_MALLOC_FAILED TMMGR_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifndef TMMGR_ERROR_STRANGE
#define TMMGR_ERROR_STRANGE TMMGR_ERROR_HANDLER(__FILE__,__LINE__,"STRANGE")
#endif

#ifdef __cplusplus
}
#endif

#endif
