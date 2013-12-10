
/* mapping from Key_t to (Offset_t, Size_t) */

#ifndef OSTAB_H
#define OSTAB_H

#include <mmgr/word.h>

#include <mmgr/trie.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef TrieMap_t OStab_t, *POStab_t;

  /* O(number of bits in Key_t) */
  extern Bool_t ostab_ins (POStab_t, const Key_t, const Offset_t,
                           const Size_t);
  extern Bool_t ostab_get (const OStab_t, const Key_t, POffset_t, PSize_t);
  extern void ostab_del (POStab_t, const Key_t);

  /* O(number of elements in map) */
  extern Size_t ostab_free (POStab_t);
  extern Size_t ostab_memused (const OStab_t);
  extern Size_t ostab_size (const OStab_t);

  typedef void (*fOStabWork_t) (const Key_t, const Offset_t, const Size_t,
                                void *);

  extern void ostab_work (const OStab_t, const fOStabWork_t, void *);

  /* *********************************************************************** */

#ifndef OSTAB_ERROR_HANDLER
#include <mmgr/error.h>
#define OSTAB_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"OStab",fun)
#endif

#ifndef OSTAB_ERROR_MALLOC_FAILED
#define OSTAB_ERROR_MALLOC_FAILED OSTAB_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifdef __cplusplus
}
#endif

#endif
