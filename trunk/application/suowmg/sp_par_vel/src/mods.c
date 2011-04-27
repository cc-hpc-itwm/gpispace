#include "mods.h"

modsFILE *mods_fopen(char *traceFile,char *permission)
{
  FILE *fp;
  modsFILE *mFILE = NULL;
  

  if ((fp = fopen(traceFile, permission)) != NULL) {
    mFILE = (modsFILE *)malloc(sizeof(modsFILE));
    if(mFILE==NULL) {
      fprintf(stderr, "Could not alloc memory to mFILE pointer!\n");
      return NULL;
    }
    mFILE->fp=fp;
    mFILE->currentPos=0;
    return mFILE;
  } else {
    return NULL;
  }

}

void mods_fclose(modsFILE *mFILE) { 
  
  fclose(mFILE->fp);
  free(mFILE);

  return;
}

/**
 */
void mods_fread(modsFILE *mFILE, float *buf, long long int pos, long long int count)
{
  int ier;
  long long int posChange = pos - mFILE->currentPos;
  mFILE->currentPos = pos + count;

  ier = fseek(mFILE->fp, posChange*sizeof(float), SEEK_CUR);
  if (ier != 0) {
    printf("MODS: fseek failed with error code %i\n", ier);
    printf("MODS: fseek errno code %i\n", errno);
    printf("MODS: fseek debug: pos = %lli\n", pos);
    printf("MODS: fseek debug: count = %lli\n", count);
    printf("MODS: fseek debug: mFile->currentPos = %lli\n", mFILE->currentPos);
    printf("MODS: fseek debug: posChange = %lli\n", posChange);
    abort();
    return;
  }

  ier = fread(buf, sizeof(float), count, mFILE->fp);
  if (ier != count) {
    printf("MODS: fread failed with return value %i\n", ier);
    printf("MODS: fread errno code %i\n", errno);
    printf("MODS: fread debug: pos = %lli\n", pos);
    printf("MODS: fread debug: count = %lli\n", count);
    abort();
    return;
  }

  return;
}
