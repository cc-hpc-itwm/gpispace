#include "swap_bytes.h"

void swap_bytes(void *a, int n, int nb) {
  char tmp[16];
  char* b=(char*)a;
  int i,j;
  char* cd=tmp;
  char* cs;

  for (i=0;i<n;i++) {
    cs=&b[nb*i];
    for (j=0;j<nb;j++)
      cd[j]=cs[nb-1-j];
    bcopy(cd,cs,nb);
  }
};

int TestByteOrder(void)
{
    short int word = 0x0001;
    char *byte = (char *) &word;
    return(byte[0] ? LITENDIAN : BIGENDIAN);
}

void float2ibm(float* data, const int n, const int endianess)
{
    register int fconv, fmant, t; 
    for (int i = 0; i < n; i++)
    {
	
	fconv = ((int*)data)[i];
	if (fconv)
	{
	    fmant = (0x007fffff & fconv) | 0x00800000;
	    t = (int) ((0x7f800000 & fconv) >> 23) - 126;
	    while (t&0x3) { ++t; fmant>>=1;}
	    fconv = (0x80000000 & fconv) |(((t>>2) + 64) << 24) | fmant;
	}

	if (endianess == LITENDIAN)
	  fconv = (fconv<<24) | ((fconv>>24)&0xff) |
	    ((fconv&0xff00)<<8) | ((fconv&0xff0000)>>8);
	
	((int*)data)[i] = fconv;
	
    }
}


void ibm2float(float* data, const int n, const int endianess)
{
    register int fconv, fmant, t; 
    for (int i = 0; i < n; i++)
    {
	fconv = ((int*)data)[i];

	if (endianess == LITENDIAN)
	  fconv = (fconv<<24) | ((fconv>>24)&0xff) |
	    ((fconv&0xff00)<<8) | ((fconv&0xff0000)>>8);

	if (fconv)
	{
	    fmant = 0x00ffffff & fconv;
	    t = (int) ((0x7f000000 & fconv) >> 22) - 130;
	    while (!(fmant & 0x00800000)) { --t; fmant <<= 1; }
	    if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
	    else if (t <= 0) fconv = 0;
	    else fconv = (0x80000000 & fconv) |(t << 23)|(0x007fffff &
							  fmant);
	}
	
	((int*)data)[i] = fconv;
    }
}
