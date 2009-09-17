
#include <stdlib.h>

#include <trie.h>
#include <unused.h>

typedef struct node_t
{
  struct node_t *child[4];
  PValue_t data;
} Trie_t, *PTrie_t;

static inline PTrie_t
empty ()
{
  const PTrie_t t = calloc (1, sizeof (Trie_t));

  if (t == NULL)
    TRIE_ERROR_MALLOC_FAILED;

  return t;
}

PValue_t
trie_ins (PTrieMap_t PPTrie, Key_t Key, PBool_t Pwas_there)
{
  if (PPTrie == NULL)
    return NULL;

  if (*(PTrie_t *) PPTrie == NULL)
    {
      *(PTrie_t *) PPTrie = empty ();
    }

  PTrie_t PTrie = *(PTrie_t *) PPTrie;

  while (Key > 0)
    {
      if (PTrie->child[Key % 4] == NULL)
	{
	  PTrie->child[Key % 4] = empty ();
	}

      PTrie = PTrie->child[Key % 4];

      Key /= 4;
    }

  if (Pwas_there != NULL)
    *Pwas_there = (PTrie->data == NULL) ? False : True;

  if (PTrie->data == NULL)
    {
      PTrie->data = malloc (sizeof (Value_t));

      if (PTrie->data == NULL)
	TRIE_ERROR_MALLOC_FAILED;
    }

  return PTrie->data;
}

PValue_t
trie_getany (const PTrieMap_t PCTrie)
{
  PTrie_t PTrie = (PTrie_t) PCTrie;

  while (PTrie != NULL && PTrie->data == NULL)
    {
      PTrie = (PTrie->child[0] != NULL)
	? PTrie->child[0]
	: ((PTrie->child[1] != NULL)
	   ? PTrie->child[1]
	   : ((PTrie->child[2] != NULL) ? PTrie->child[2] : PTrie->child[3]));
    }

  return (PTrie == NULL) ? NULL : PTrie->data;
}

PValue_t
trie_get (const PTrieMap_t PCTrie, Key_t Key)
{
  PTrie_t PTrie = (PTrie_t) PCTrie;

  while (PTrie != NULL && Key > 0)
    {
      PTrie = PTrie->child[Key % 4];

      Key /= 4;
    }

  return (PTrie == NULL) ? NULL : PTrie->data;
}

Bool_t
trie_del (PTrieMap_t PPTrie, const Key_t Key, const fUser_t fUser)
{
  if (PPTrie == NULL || *(PTrie_t *) PPTrie == NULL)
    return False;

  Bool_t rc;
  const PTrie_t PTrie = *(PTrie_t *) PPTrie;

  if (Key > 0)
    {
      rc = trie_del (&(PTrie->child[Key % 4]), Key / 4, fUser);

      if (PTrie->child[0] == NULL && PTrie->child[1] == NULL
	  && PTrie->child[2] == NULL && PTrie->child[3] == NULL
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

      if (PTrie->child[0] == NULL && PTrie->child[1] == NULL
	  && PTrie->child[2] == NULL && PTrie->child[3] == NULL)
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
  if (PPTrie == NULL || *(PTrie_t *) PPTrie == NULL)
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
  Bytes += trie_free (PTrie->child + 2, fUser);
  Bytes += trie_free (PTrie->child + 3, fUser);

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
  Bytes += trie_memused (PTrie->child[2], fUser);
  Bytes += trie_memused (PTrie->child[3], fUser);

  return Bytes;
}

static inline Key_t
patch (const Key_t Key, const Word_t Level, const unsigned int slot)
{
  switch (slot)
    {
    case 0:
      return ((Key & ~(1 << Level)) & ~(2 << Level));
    case 1:
      return ((Key | (1 << Level)) & ~(2 << Level));
    case 2:
      return ((Key & ~(1 << Level)) | (2 << Level));
    case 3:
      return ((Key | (1 << Level)) | (2 << Level));
    }

  TRIE_ERROR_STRANGE;

  return -1;
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

  trie_work_key (PTrie->child[0], fTrieWork, patch (Key, Level, 0), Level + 2,
		 Pdat);
  trie_work_key (PTrie->child[1], fTrieWork, patch (Key, Level, 1), Level + 2,
		 Pdat);
  trie_work_key (PTrie->child[2], fTrieWork, patch (Key, Level, 2), Level + 2,
		 Pdat);
  trie_work_key (PTrie->child[3], fTrieWork, patch (Key, Level, 3), Level + 2,
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
