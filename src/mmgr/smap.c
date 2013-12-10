
#include <stdlib.h>

#include <mmgr/smap.h>
#include <mmgr/unused.h>

typedef struct node_t
{
  struct node_t *child[2];
  Key_t key;
  Value_t value;
} Tree_t, *PTree_t;

static inline PTree_t
empty ()
{
  const PTree_t t = malloc (sizeof (Tree_t));

  if (t == NULL)
    {
      SMAP_ERROR_MALLOC_FAILED;
    }
  else
    {
      t->child[0] = t->child[1] = NULL;
    }

  return t;
}

Bool_t
smap_ins (PSMap_t PPTree, const Key_t Key, const Value_t Value)
{
  if (PPTree == NULL)
    return False;

  if (*(PTree_t *) PPTree == NULL)
    {
      *(PTree_t *) PPTree = empty ();
      (*(PTree_t *) PPTree)->key = Key;
      (*(PTree_t *) PPTree)->value = Value;

      return False;
    }
  else
    {
      const PTree_t PTree = *(PTree_t *) PPTree;

      if (Key < PTree->key)
        {
          return smap_ins (&(PTree->child[0]), Key, Value);
        }
      else if (Key > PTree->key)
        {
          return smap_ins (&(PTree->child[1]), Key, Value);
        }
      else
        {
          PTree->value = Value;

          return True;
        }
    }
}

PValue_t
smap_get (const SMap_t PCTree, const Key_t Key)
{
  PTree_t PTree = PCTree;

  while (PTree != NULL && Key != PTree->key)
    {
      PTree = (Key < PTree->key) ? PTree->child[0] : PTree->child[1];
    }

  return (PTree == NULL) ? NULL : &(PTree->value);
}

PValue_t
smap_get_atleast (const SMap_t PCTree, PKey_t PWantHave)
{
  if (PWantHave == NULL)
    return NULL;

  PTree_t PTree = PCTree;

  while (PTree != NULL && *PWantHave > PTree->key)
    {
      PTree = PTree->child[1];
    }

  if (PTree != NULL)
    {
      *PWantHave = PTree->key;
    }

  return (PTree == NULL) ? NULL : &(PTree->value);
}

PValue_t
smap_get_atleast_minimal (const SMap_t PCTree, PKey_t PWantHave)
{
  if (PWantHave == NULL)
    return NULL;

  PTree_t PTree = PCTree;

  if (PTree == NULL)
    return NULL;

  if (*PWantHave == PTree->key)
    {
      return &(PTree->value);
    }
  else if (*PWantHave > PTree->key)
    {
      PValue_t resMore =
        smap_get_atleast_minimal (PTree->child[1], PWantHave);

      if (resMore != NULL)
        {
          return resMore;
        }
      else
        {
          return NULL;
        }
    }
  else
    {
      PValue_t resLess =
        smap_get_atleast_minimal (PTree->child[0], PWantHave);

      if (resLess != NULL)
        {
          return resLess;
        }
      else
        {
          *PWantHave = PTree->key;

          return &(PTree->value);
        }
    }
}

Bool_t
smap_del (PSMap_t PPTree, const Key_t Key, const SMAP_DEL_SEL_t del_sel)
{
  if (PPTree == NULL)
    return False;

  PTree_t *PPParent = PPTree;
  PTree_t PTree = *(PTree_t *) PPTree;

  while (PTree != NULL && Key != PTree->key)
    {
      PPParent = (Key < PTree->key) ? (PTree->child + 0) : (PTree->child + 1);
      PTree = (Key < PTree->key) ? PTree->child[0] : PTree->child[1];
    }

  if (PTree == NULL)
    return False;

  const unsigned int PTreeChildMap
    = ((PTree->child[0] == NULL) ? 0 : (1 << 0))
    + ((PTree->child[1] == NULL) ? 0 : (1 << 1));

  switch (PTreeChildMap)
    {
    case 0:                    /* no children */
      *(PTree_t *) PPParent = NULL;
      free (PTree);
      break;

    case 1:                    /* left child only */
      *(PTree_t *) PPParent = PTree->child[0];
      free (PTree);
      break;

    case 2:                    /* right child only */
      *(PTree_t *) PPParent = PTree->child[1];
      free (PTree);
      break;

    case 3:                    /* both children */
      {
        /* search inorder successor or predecessor */

        const unsigned int sel = (del_sel == SMAP_DEL_INORDER_SUCC) ? 0 : 1;

        PTree_t *PPChildParent = PTree->child + (1 - sel);
        PTree_t PChild = PTree->child[1 - sel];

        while (PChild->child[sel] != NULL)
          {
            PPChildParent = PChild->child + sel;
            PChild = PChild->child[sel];
          }

        *(PTree_t *) PPChildParent = PChild->child[1 - sel];

        PTree->key = PChild->key;
        PTree->value = PChild->value;

        free (PChild);
      }

      break;

    default:
      SMAP_ERROR_HANDLER (__FILE__, __LINE__, "smap_del: STRANGE");
      exit (EXIT_FAILURE);
    }

  return True;
}

static void
smap_work_level (PTree_t PTree, const fSMapWorkCut_t fSMapWorkCut,
                 const Level_t Level, void *Pdat)
{
  if (PTree == NULL)
    return;

  const Cut_t cut = fSMapWorkCut (PTree->key, PTree->value, Level, Pdat);

  if (cut == NoCut)
    {
      smap_work_level (PTree->child[0], fSMapWorkCut, Level + 1, Pdat);
      smap_work_level (PTree->child[1], fSMapWorkCut, Level + 1, Pdat);
    }
}

void
smap_work_preorder (const SMap_t PCTree, const fSMapWorkCut_t fSMapWorkCut,
                    void *Pdat)
{
  smap_work_level (PCTree, fSMapWorkCut, 0, Pdat);
}

void
smap_work_inorder (const SMap_t PCTree, const fSMapWork_t fSMapWork,
                   void *Pdat)
{
  const PTree_t PTree = PCTree;

  if (PTree == NULL)
    return;

  smap_work_inorder (PTree->child[0], fSMapWork, Pdat);

  fSMapWork (PTree->key, PTree->value, Pdat);

  smap_work_inorder (PTree->child[1], fSMapWork, Pdat);
}

void
smap_work (const SMap_t PCTree, const fSMapWork_t fSMapWork, void *Pdat)
{
  smap_work_inorder (PCTree, fSMapWork, Pdat);
}

Size_t
smap_free (PSMap_t PPTree)
{
  if (PPTree == NULL || *(PTree_t *) PPTree == NULL)
    return 0;

  Size_t Bytes = sizeof (Tree_t);

  const PTree_t PTree = *(PTree_t *) PPTree;

  Bytes += smap_free (PTree->child + 0);
  Bytes += smap_free (PTree->child + 1);

  free (PTree);

  *(PTree_t *) PPTree = NULL;

  return Bytes;
}

static void
fCountMem (const Key_t UNUSED (Key), const Value_t UNUSED (Value), void *Pdat)
{
  *(PSize_t) Pdat += sizeof (Tree_t);
}

Size_t
smap_memused (const SMap_t PCTree)
{
  Size_t Bytes = 0;

  smap_work_inorder (PCTree, &fCountMem, &Bytes);

  return Bytes;
}

static void
fCountSize (const Key_t UNUSED (Key), const Value_t UNUSED (Value),
            void *Pdat)
{
  *(PSize_t) Pdat += 1;
}


Size_t
smap_size (const SMap_t PCTree)
{
  Size_t Size = 0;

  smap_work_inorder (PCTree, &fCountSize, &Size);

  return Size;
}
