
/* sorted map from Key_t to SetOf(Value_t), based on a simple binary
   search tree, can degenerate to a sequential list
*/

#ifndef FSEG_H
#define FSEG_H

#include <mmgr/smap.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef SMap_t FSeg_t, *PFSeg_t;

  extern void fseg_ins (PFSeg_t, const Key_t, const Value_t);
  extern PValue_t fseg_get (const FSeg_t, const Key_t);
  extern PValue_t fseg_get_atleast (const FSeg_t, PKey_t);
  extern PValue_t fseg_get_atleast_minimal (const FSeg_t, PKey_t);
  extern void fseg_del (PFSeg_t, const Key_t, const Value_t,
                        const SMAP_DEL_SEL_t);

  extern Size_t fseg_free (PFSeg_t);
  extern Size_t fseg_memused (const FSeg_t);
  extern Size_t fseg_size (const FSeg_t);

  /* *********************************************************************** */

#ifndef FSEG_ERROR_HANDLER
#include <mmgr/error.h>
#define FSEG_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"FSeg",fun)
#endif

#ifndef FSEG_ERROR_MALLOC_FAILED
#define FSEG_ERROR_MALLOC_FAILED FSEG_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifdef __cplusplus
}
#endif


#endif
