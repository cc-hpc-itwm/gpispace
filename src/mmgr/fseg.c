
#include <stdlib.h>

#include <mmgr/fseg.h>
#include <mmgr/trie.h>
#include <mmgr/unused.h>

void
fseg_ins (PFSeg_t PFSeg, const Key_t Key, const Value_t Value)
{
  PValue_t PVal = (PFSeg == NULL) ? NULL : smap_get (*PFSeg, Key);
  TrieMap_t TrieMap = (PVal == NULL) ? NULL : (TrieMap_t) (*PVal);

  *trie_ins (&TrieMap, Value, NULL) = Value;

  if (PVal == NULL)
    smap_ins (PFSeg, Key, (Value_t) TrieMap);
}

PValue_t
fseg_get (const FSeg_t FSeg, const Key_t Key)
{
  PValue_t PVal = smap_get (FSeg, Key);

  return (PVal == NULL) ? NULL : trie_getany ((TrieMap_t) (*PVal));
}

PValue_t
fseg_get_atleast (const FSeg_t FSeg, PKey_t PKey)
{
  PValue_t PVal = smap_get_atleast (FSeg, PKey);

  return (PVal == NULL) ? NULL : trie_getany ((TrieMap_t) (*PVal));
}

PValue_t
fseg_get_atleast_minimal (const FSeg_t FSeg, PKey_t PKey)
{
  PValue_t PVal = smap_get_atleast_minimal (FSeg, PKey);

  return (PVal == NULL) ? NULL : trie_getany ((TrieMap_t) (*PVal));
}

void
fseg_del (PFSeg_t PFSeg, const Key_t Key, const Value_t Value,
          const SMAP_DEL_SEL_t del_sel)
{
  if (PFSeg == NULL)
    return;

  PValue_t PVal = smap_get (*PFSeg, Key);

  if (PVal != NULL)
    {
      TrieMap_t tm = (TrieMap_t) (*PVal);

      trie_del (&tm, Value, fUserNone);

      if (tm == NULL)
        {
          smap_del (PFSeg, Key, del_sel);
        }
    }
}

static void
fFree (const Key_t UNUSED (Key), const Value_t Value, void *Pdat)
{
  TrieMap_t TrieMap = (TrieMap_t) Value;

  *(PSize_t) Pdat += trie_free (&TrieMap, fUserNone);
}

Size_t
fseg_free (PFSeg_t PFSeg)
{
  if (PFSeg == NULL)
    return 0;

  Size_t Size = 0;

  smap_work (*PFSeg, fFree, &Size);

  Size += smap_free (PFSeg);

  return Size;
}

static void
fMemUsed (const Key_t UNUSED (Key), const Value_t Value, void *Pdat)
{
  TrieMap_t TrieMap = (TrieMap_t) Value;

  *(PSize_t) Pdat += trie_memused (TrieMap, fUserNone);
}

Size_t
fseg_memused (const FSeg_t FSeg)
{
  Size_t Size = 0;

  smap_work (FSeg, fMemUsed, &Size);

  return Size + smap_memused (FSeg);
}

static void
fSize (const Key_t UNUSED (Key), const Value_t Value, void *Pdat)
{
  TrieMap_t TrieMap = (TrieMap_t) Value;

  *(PSize_t) Pdat += trie_size (TrieMap);
}

Size_t
fseg_size (const FSeg_t FSeg)
{
  Size_t Size = 0;

  smap_work (FSeg, fSize, &Size);

  return Size;
}
