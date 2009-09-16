
#include <stdlib.h>

#include <trie.h>
#include <unused.h>

typedef struct node_t
{
  struct node_t *child[2];
  PValue_t data;
} Trie_t, *PTrie_t;

static inline PTrie_t
empty ()
{
  const PTrie_t t = malloc (sizeof (Trie_t));

  if (t == NULL)
    {
      TRIE_ERROR_MALLOC_FAILED;
    }
  else
    {
      t->child[0] = t->child[1] = NULL;

      t->data = NULL;
    }

  return t;
}

PValue_t
trie_ins (PTrieMap_t PPTrie, Key_t Key, PBool_t Pwas_there)
{
  if (*(PTrie_t *) PPTrie == NULL)
    {
      *(PTrie_t *) PPTrie = empty ();
    }

  PTrie_t PTrie = *(PTrie_t *) PPTrie;

  while (Key > 0)
    {
      if (PTrie->child[Key & 1] == NULL)
        {
          PTrie->child[Key & 1] = empty ();
        }

      PTrie = PTrie->child[Key & 1];

      Key >>= 1;
    }

  if (PTrie->data == NULL)
    {
      PTrie->data = malloc (sizeof (Value_t));

      if (PTrie->data == NULL)
        TRIE_ERROR_MALLOC_FAILED;

      if (Pwas_there != NULL)
        *Pwas_there = False;
    }
  else
    {
      if (Pwas_there != NULL)
        *Pwas_there = True;
    }

  return PTrie->data;
}

PValue_t
trie_getany (const PTrieMap_t PCTrie)
{
  PTrie_t PTrie = (PTrie_t) PCTrie;

  while (PTrie != NULL && PTrie->data == NULL)
    {
      PTrie = (PTrie->child[0] == NULL) ? PTrie->child[1] : PTrie->child[0];
    }

  return (PTrie == NULL) ? NULL : PTrie->data;
}

PValue_t
trie_get (const PTrieMap_t PCTrie, Key_t Key)
{
  PTrie_t PTrie = (PTrie_t) PCTrie;

  while (PTrie != NULL && Key > 0)
    {
      PTrie = PTrie->child[Key & 1];

      Key >>= 1;
    }

  return (PTrie == NULL) ? NULL : PTrie->data;
}

Bool_t
trie_del (PTrieMap_t PPTrie, const Key_t Key, const fUser_t fUser)
{
  if (*(PTrie_t *) PPTrie == NULL)
    return False;

  Bool_t rc;
  const PTrie_t PTrie = *(PTrie_t *) PPTrie;

  if (Key > 0)
    {
      rc = trie_del (&(PTrie->child[Key & 1]), Key >> 1, fUser);

      if (PTrie->child[0] == NULL && PTrie->child[1] == NULL
          && PTrie->data == NULL)
        {
          free (PTrie);

          *(PTrie_t *) PPTrie = NULL;
        }
    }
  else
    {
      rc = (PTrie->data == NULL) ? False : True;

      if (fUser != NULL && PTrie->data != NULL)
        fUser (PTrie->data);

      free (PTrie->data);

      PTrie->data = NULL;

      if (PTrie->child[0] == NULL && PTrie->child[1] == NULL)
        {
          free (PTrie);

          *(PTrie_t *) PPTrie = NULL;
        }
    }

  return rc;
}

Value_t
trie_free (PTrieMap_t PPTrie, const fUser_t fUser)
{
  if (*(PTrie_t *) PPTrie == NULL)
    return 0;

  Size_t Bytes = sizeof (Trie_t);

  const PTrie_t PTrie = *(PTrie_t *) PPTrie;

  if (PTrie->data != NULL)
    {
      if (fUser != NULL)
        Bytes += fUser (PTrie->data);

      free (PTrie->data);

      PTrie->data = NULL;

      Bytes += sizeof (Value_t);
    }

  Bytes += trie_free (PTrie->child + 0, fUser);
  Bytes += trie_free (PTrie->child + 1, fUser);

  free (PTrie);

  *(PTrie_t *) PPTrie = NULL;

  return Bytes;
}

Size_t
trie_memused (const TrieMap_t PCTrie, const fUser_t fUser)
{
  const PTrie_t PTrie = (PTrie_t) PCTrie;

  if (PTrie == NULL)
    return 0;

  Size_t Bytes = sizeof (Trie_t);

  if (PTrie->data != NULL)
    {
      if (fUser != NULL)
        Bytes += fUser (PTrie->data);

      Bytes += sizeof (Size_t);
    }

  Bytes += trie_memused (PTrie->child[0], fUser);
  Bytes += trie_memused (PTrie->child[1], fUser);

  return Bytes;
}

static void
trie_work_key (const PTrie_t PTrie, const fTrieWork_t fTrieWork,
               const Key_t Key, const Word_t Level, void *Pdat)
{
  if (PTrie == NULL)
    return;

  if (PTrie->data != NULL)
    {
      fTrieWork (Key, PTrie->data, Pdat);
    }

  trie_work_key (PTrie->child[0], fTrieWork, Key & ~(1 << Level), Level + 1,
                 Pdat);
  trie_work_key (PTrie->child[1], fTrieWork, Key | (1 << Level), Level + 1,
                 Pdat);
}

void
trie_work (const TrieMap_t PCTrie, const fTrieWork_t fTrieWork, void *Pdat)
{
  trie_work_key (PCTrie, fTrieWork, 0, 0, Pdat);
}

static void
fCount (const Key_t UNUSED (Key), const PValue_t UNUSED (PVal), void *PSize)
{
  *(PSize_t) PSize += 1;
}

Size_t
trie_size (const TrieMap_t PCTrie)
{
  Size_t Size = 0;

  trie_work (PCTrie, &fCount, &Size);

  return Size;
}
