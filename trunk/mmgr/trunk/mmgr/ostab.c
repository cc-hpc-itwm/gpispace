
#include <stdlib.h>

#include <mmgr/ostab.h>
#include <mmgr/unused.h>

#include <assert.h>

typedef struct
{
  Offset_t offset;
  Size_t size;
} data_t, *pdata_t;

Bool_t
ostab_ins (POStab_t postab, const Key_t Key, const Offset_t Offset,
           const Size_t Size)
{
  Bool_t was_there = False;
  const PValue_t PVal = trie_ins (postab, Key, &was_there);
  const pdata_t data =
    (was_there == True) ? ((pdata_t) (*PVal)) : malloc (sizeof (data_t));

  if (data == NULL)
    OSTAB_ERROR_MALLOC_FAILED;

  data->offset = Offset;
  data->size = Size;

  *PVal = (Value_t) data;

  return was_there;
}

Bool_t
ostab_get (const OStab_t ostab, const Key_t Key, POffset_t POffset,
           PSize_t PSize)
{
  const PValue_t PVal = trie_get (ostab, Key);

  if (PVal == NULL)
    return False;

  const pdata_t pdata = (pdata_t) (*PVal);

  if (POffset != NULL)
    *POffset = pdata->offset;

  if (PSize != NULL)
    *PSize = pdata->size;

  return True;
}

static Size_t
fUser_free (const PValue_t PVal)
{
  if (PVal == NULL)
    return 0;

  const pdata_t pdata = (pdata_t) (*PVal);

  free (pdata);

  return sizeof (data_t);
}

void
ostab_del (POStab_t postab, const Key_t Key)
{
  trie_del (postab, Key, &fUser_free);
}

Size_t
ostab_free (POStab_t postab)
{
  return trie_free (postab, &fUser_free);
}

static Size_t
fUser_count (const PValue_t UNUSED (PVal))
{
  return sizeof (data_t);
}

Size_t
ostab_memused (const OStab_t ostab)
{
  return trie_memused (ostab, &fUser_count);
}

Size_t
ostab_size (const OStab_t ostab)
{
  return trie_size (ostab);
}

typedef struct
{
  fOStabWork_t fOStabWork;
  void *Pdat;
} work_dat_t, *pwork_dat_t;

static void
fWork (const Key_t Key, const PValue_t PVal, void *Pwd)
{
  assert (PVal != NULL);

  const pdata_t pdata = (pdata_t) (*PVal);
  const pwork_dat_t pwd = Pwd;

  pwd->fOStabWork (Key, pdata->offset, pdata->size, pwd->Pdat);
}

void
ostab_work (const OStab_t ostab, const fOStabWork_t fOStabWork, void *Pdat)
{
  work_dat_t wd = { fOStabWork, Pdat };

  trie_work (ostab, &fWork, &wd);
}
