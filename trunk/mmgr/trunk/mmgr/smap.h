
/* sorted map from Key_t to Value_t, based on a simple binary search
   tree, can degenerate to a sequential list
*/

#ifndef SMAP_H
#define SMAP_H

#include <mmgr/bool.h>
#include <mmgr/word.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void *SMap_t, *PSMap_t;

  typedef enum
  { SMAP_DEL_INORDER_SUCC,
    SMAP_DEL_INORDER_PRED,
    SMAP_DEL_DEFAULT = SMAP_DEL_INORDER_SUCC
  } SMAP_DEL_SEL_t;

  extern Bool_t smap_ins (PSMap_t, const Key_t, const Value_t);
  extern PValue_t smap_get (const SMap_t, const Key_t);
  extern PValue_t smap_get_atleast (const SMap_t, PKey_t);
  extern PValue_t smap_get_atleast_minimal (const SMap_t, PKey_t);
  extern Bool_t smap_del (PSMap_t, const Key_t, const SMAP_DEL_SEL_t);

  extern Size_t smap_free (PSMap_t);
  extern Size_t smap_memused (const SMap_t);
  extern Size_t smap_size (const SMap_t);

  typedef Word_t Level_t, *PLevel_t;
  typedef Bool_t Cut_t, *PCut_t;

  static const Cut_t Cut = True;
  static const Cut_t NoCut = False;

  typedef void (*fSMapWork_t) (const Key_t, const Value_t, void *);
  typedef Cut_t (*fSMapWorkCut_t) (const Key_t, const Value_t,
                                   const Level_t, void *);

  extern void smap_work_preorder (const SMap_t, const fSMapWorkCut_t, void *);
  extern void smap_work_inorder (const SMap_t, const fSMapWork_t, void *);
  extern void smap_work (const SMap_t, const fSMapWork_t, void *);

  /* *********************************************************************** */

#ifndef SMAP_ERROR_HANDLER
#include <mmgr/error.h>
#define SMAP_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"Smap",fun)
#endif

#ifndef SMAP_ERROR_MALLOC_FAILED
#define SMAP_ERROR_MALLOC_FAILED SMAP_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifdef __cplusplus
}
#endif


#endif
