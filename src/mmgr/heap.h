
#ifndef HEAP_H
#define HEAP_H

#include <mmgr/word.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void *Heap_t, *PHeap_t;

  static const Size_t heap_initial_size = (1 << 6);
  static const Size_t heap_minimal_size = (1 << 4);

  extern void heap_ins (PHeap_t, const Offset_t);
  extern Size_t heap_size (const Heap_t);
  extern Offset_t heap_min (const Heap_t);
  extern void heap_delmin (PHeap_t);
  extern Size_t heap_free (PHeap_t);

  extern void heap_out (const Heap_t);

  /* *********************************************************************** */

#ifndef HEAP_ERROR_HANDLER
#include <mmgr/error.h>
#define HEAP_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"Tmmgr",fun)
#endif

#ifndef HEAP_ERROR_MALLOC_FAILED
#define HEAP_ERROR_MALLOC_FAILED HEAP_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifndef HEAP_ERROR_EMPTY
#define HEAP_ERROR_EMPTY HEAP_ERROR_HANDLER(__FILE__,__LINE__,"min: heap is empty")
#endif

#ifdef __cplusplus
}
#endif

#endif
