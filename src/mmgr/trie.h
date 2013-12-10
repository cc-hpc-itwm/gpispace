
/* mapping Key_t to Value_t */

#ifndef TRIE_H
#define TRIE_H

#include <mmgr/bool.h>
#include <mmgr/null.h>
#include <mmgr/word.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void *TrieMap_t, *PTrieMap_t;

  /* let N be the number of elements in the map */
  /* let b be the number of bits in one Key_t  */

  /* O(b) */ extern PValue_t trie_ins (PTrieMap_t, const Key_t, PBool_t);
  /* O(b) */ extern PValue_t trie_get (const TrieMap_t, const Key_t);
  /* O(b) */ extern PValue_t trie_getany (const TrieMap_t);
  /* O(b) */ extern void trie_del (PTrieMap_t, const Key_t, const fUser_t);

  /* O(N) */ extern Size_t trie_free (PTrieMap_t, const fUser_t);
  /* O(N) */ extern Size_t trie_memused (const TrieMap_t, const fUser_t);
  /* O(N) */ extern Size_t trie_size (const TrieMap_t);

  typedef void (*fTrieWork_t) (const Key_t, const PValue_t, void *);

  extern void trie_work (const TrieMap_t, const fTrieWork_t, void *);

#ifndef TRIE_BITS
#define TRIE_BITS 3
#endif

  /* *********************************************************************** */

#ifndef TRIE_ERROR_HANDLER
#include <mmgr/error.h>
#define TRIE_ERROR_HANDLER(file,line,fun) GEN_ERROR_HANDLER(file,line,"Trie",fun)
#endif

#ifndef TRIE_ERROR_MALLOC_FAILED
#define TRIE_ERROR_MALLOC_FAILED TRIE_ERROR_HANDLER(__FILE__,__LINE__,"malloc")
#endif

#ifndef TRIE_ERROR_STRANGE
#define TRIE_ERROR_STRANGE TRIE_ERROR_HANDLER(__FILE__,__LINE__,"STRANGE")
#endif

#ifdef __cplusplus
}
#endif

#endif
