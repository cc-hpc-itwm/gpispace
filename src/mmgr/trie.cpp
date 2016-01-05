
#include <stdlib.h>

#include <mmgr/trie.h>

#include <util-generic/unused.hpp>

#if (TRIE_BITS < 1)
#undef TRIE_BITS
#define TRIE_BITS 1
#endif

#define NCHILD (1 << TRIE_BITS)

#define REM(x)  ((x) & (NCHILD - 1))
#define QUOT(x) ((x) >> TRIE_BITS)
#define DIV(x)  ((x) >>= TRIE_BITS)

typedef int ItChild_t;

#define FOR_CHILD(id) for (ItChild_t id = 0; id < NCHILD; ++id)

typedef struct node_t
{
  struct node_t *child[NCHILD];
  PValue_t data;
} Trie_t, *PTrie_t;

static inline PTrie_t
empty ()
{
  const PTrie_t t = static_cast<PTrie_t> (calloc (1, sizeof (Trie_t)));

  if (t == nullptr)
    TRIE_ERROR_MALLOC_FAILED;

  return t;
}

PValue_t
trie_ins (PTrieMap_t PPTrie, Key_t Key, bool* Pwas_there)
{
  if (PPTrie == nullptr)
    return nullptr;

  if (*(PTrie_t *) PPTrie == nullptr)
    {
      *(PTrie_t *) PPTrie = empty ();
    }

  PTrie_t PTrie = *(PTrie_t *) PPTrie;

  while (Key > 0)
    {
      if (PTrie->child[REM(Key)] == nullptr)
        {
          PTrie->child[REM(Key)] = empty ();
        }

      PTrie = PTrie->child[REM(Key)];

      DIV (Key);
    }

  if (Pwas_there != nullptr)
    *Pwas_there = (PTrie->data == nullptr) ? false : true;

  if (PTrie->data == nullptr)
    {
      PTrie->data = static_cast<PValue_t> (malloc (sizeof (Value_t)));

      if (PTrie->data == nullptr)
        TRIE_ERROR_MALLOC_FAILED;
    }

  return PTrie->data;
}

PValue_t
trie_getany (const PTrieMap_t PCTrie)
{
  PTrie_t PTrie = static_cast<PTrie_t> (PCTrie);

  while (PTrie != nullptr && PTrie->data == nullptr)
    {
      ItChild_t child = 0;

    CHILD:

      if (PTrie->child[child] != nullptr || child == NCHILD - 1)
        {
          PTrie = PTrie->child[child];
        }
      else
        {
          ++child;

          goto CHILD;
        }
    }

  return (PTrie == nullptr) ? nullptr : PTrie->data;
}

PValue_t
trie_get (const PTrieMap_t PCTrie, Key_t Key)
{
  PTrie_t PTrie = static_cast<PTrie_t> (PCTrie);

  while (PTrie != nullptr && Key > 0)
    {
      PTrie = PTrie->child[REM(Key)];

      DIV (Key);
    }

  return (PTrie == nullptr) ? nullptr : PTrie->data;
}

void
trie_del (PTrieMap_t PPTrie, const Key_t Key, const fUser_t fUser)
{
  if (PPTrie == nullptr || *(PTrie_t *) PPTrie == nullptr)
    return;

  if (Key > 0)
    {
      const PTrie_t PTrie = *(PTrie_t *) PPTrie;

      trie_del (&(PTrie->child[REM(Key)]), QUOT(Key), fUser);

      FOR_CHILD (child) if (PTrie->child[child] != nullptr)
        return;

      if (PTrie->data == nullptr)
        {
          free (PTrie);

          *(PTrie_t *) PPTrie = nullptr;
        }

      return;
    }

  const PTrie_t PTrie = *(PTrie_t *) PPTrie;

  if (fUser != nullptr && PTrie->data != nullptr)
    fUser (PTrie->data);

  free (PTrie->data);

  PTrie->data = nullptr;

  FOR_CHILD (child) if (PTrie->child[child] != nullptr)
    return;

  free (PTrie);

  *(PTrie_t *) PPTrie = nullptr;
}

Value_t
trie_free (PTrieMap_t PPTrie, const fUser_t fUser)
{
  if (PPTrie == nullptr || *(PTrie_t *) PPTrie == nullptr)
    return 0;

  Size_t Bytes = sizeof (Trie_t);

  const PTrie_t PTrie = *(PTrie_t *) PPTrie;

  if (PTrie->data != nullptr)
    {
      if (fUser != nullptr)
        Bytes += fUser (PTrie->data);

      free (PTrie->data);

      PTrie->data = nullptr;

      Bytes += sizeof (Value_t);
    }

  FOR_CHILD (child) Bytes += trie_free (PTrie->child + child, fUser);

  free (PTrie);

  *(PTrie_t *) PPTrie = nullptr;

  return Bytes;
}

Size_t
trie_memused (const TrieMap_t PCTrie, const fUser_t fUser)
{
  const PTrie_t PTrie = static_cast<PTrie_t> (PCTrie);

  if (PTrie == nullptr)
    return 0;

  Size_t Bytes = sizeof (Trie_t);

  if (PTrie->data != nullptr)
    {
      if (fUser != nullptr)
        Bytes += fUser (PTrie->data);

      Bytes += sizeof (Size_t);
    }

  FOR_CHILD (child) Bytes += trie_memused (PTrie->child[child], fUser);

  return Bytes;
}

static inline Key_t
patch (Key_t Key, const Word_t Level, const ItChild_t child)
{
  for (Word_t bit = 0; bit < TRIE_BITS; ++bit)
    {
      Word_t Bit = (1 << bit);

      if ((child & Bit) > 0)
        {
          Key |= Bit << Level;
        }
      else
        {
          Key &= ~(Bit << Level);
        }
    }

  return Key;
}

static void
trie_work_key (const PTrie_t PTrie, const fTrieWork_t fTrieWork,
               const Key_t Key, const Word_t Level, void *Pdat)
{
  if (PTrie == nullptr)
    return;

  if (PTrie->data != nullptr)
    {
      fTrieWork (Key, PTrie->data, Pdat);
    }

  FOR_CHILD (child) trie_work_key (PTrie->child[child], fTrieWork,
                                   patch (Key, Level, child),
                                   Level + TRIE_BITS, Pdat);
}

void
trie_work (const TrieMap_t PCTrie, const fTrieWork_t fTrieWork, void *Pdat)
{
  trie_work_key (static_cast<PTrie_t> (PCTrie), fTrieWork, 0, 0, Pdat);
}

static void
fCount ( const Key_t FHG_UTIL_UNUSED (Key, "size independent from Key")
       , const PValue_t FHG_UTIL_UNUSED (PVal, "size independent from Value")
       , void *PSize
       )
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
