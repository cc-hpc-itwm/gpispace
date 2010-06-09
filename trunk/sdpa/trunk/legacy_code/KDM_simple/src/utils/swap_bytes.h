#ifndef SWAP_BYTES_H
#define SWAP_BYTES_H

#define BIGENDIAN   0
#define LITENDIAN   1

#include <strings.h>
void swap_bytes(void *a, int n, int nb);
void float2ibm(float* data, const int n, const int endianess);
void ibm2float(float* data, const int n, const int endianess);
int TestByteOrder(void);
#endif
