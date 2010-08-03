/*****************************************************************************
utls.h -> header definitions of some utilities,
          some of them replacing standard FORTRAN functions 
         
          memory management of 2D and 3D arrays as well here
          1D arrays allocated and freed directly in the code

***********************************************************/
#ifndef C_UTLS_H
#define C_UTLS_H

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}
#include <math.h> // cos() in staper(..)

//----- integer array sum -----------
// sums from pArr[startIndx] till including  pArr[lastIndxToSumUp]
int utlIAsum(int *pArr, int startIndx, int lastIndxToSumUp); 

//----- integer array print -----------
// prints from pArr[startIndx] till including  pArr[lastIndxToSumUp]
int utlIAprint(int *pArr, int startIndx, int lastIndxToSumUp); 

//-------- find max value in a float 3D array -----------
// NB!! the indexing is in C-style :
// - starts from ZERO
// - the last index == array_size-1
float maxValInFloat3DarrayC(float ***pppA, int szX, int szY, int szZ);
float minValInFloat3DarrayC(float ***pppA, int szX, int szY, int szZ);

//-------- find max value in a float 2D array -----------
// NB!! the indexing is in Fortran-style :
// - starts from 1
// - the last index == array_size
float maxValInFloat2DarrayF(float **pppA, int szX, int szY);

//-------- find min value in a float 2D array -----------
// NB!! the indexing is in Fortran-style :
// - starts from 1
// - the last index == array_size
float minValInFloat2DarrayF(float **pppA, int szX, int szY);



//=========== addition to the VM-variant =============
int indxOffsetIn3DarrayInZplaneChunks(int iz, int ix, int iy, 
                                       int szZ, int szX, int szY);


int indxOffsetIn3DarrayInXplaneChunks(int ix, int iy, int iz, 
				      int szX, int szY, int szZ);

//====== NB!! the functions alloc, free, setToZero =============
//----- should be template-ized. Leave it for later on ---------
//----- because you do not know how the CELL-compiler behaves ---

//======= 3D complex array =========
//----------- utlAlloc3DcomplArrayInZplaneChunks ------------------
int utlAlloc3DcomplArrayInZplaneChunks(MKL_Complex8 ****ppppA, int szZ, int szY, int szX);
int utlFree3DcomplArrayInZplaneChunks(MKL_Complex8 ***pppA, int szZ, int szY, int szX);
int utlSetToZero3DcomplArrayInZplaneChunks(MKL_Complex8 ***pppA, int szZ, int szY, int szX);

//------ alloc 3D complex array -> non-continuous chunk ---
int utlAlloc3DcomplArray(MKL_Complex8 ****ppppA, int szX, int szY, int szZ);

int utlFree3DcomplArray(MKL_Complex8 ***pppA, int szX, int szY, int szZ);

//------ initialize 3D complex array -----------
int utlSetToZero3DcomplArray(MKL_Complex8 ***pppA, int szX, int szY, int szZ);


//======= 3D float array =========
//----------- utlAlloc3DfloatArrayInZplaneChunks ------------------
int utlAlloc3DfloatArrayInZplaneChunks(float ****ppppA, int szZ, int szY, int szX);
int utlFree3DfloatArrayInZplaneChunks(float ***pppA, int szZ, int szY, int szX);
int utlSetToZero3DfloatArrayInZplaneChunks(float ***pppA, int szZ, int szY, int szX);

//------ alloc 3D float array -> non-continuous chunk ---
int utlAlloc3DfloatArray(float ****ppppA, int szX, int szY, int szZ);

int utlFree3DfloatArray(float ***pppA, int szX, int szY, int szZ);

//------ initialize 3D float array -----------
int utlSetToZero3DfloatArray(float ***pppA, int szX, int szY, int szZ);


//======= 2D complex array =========
//------ alloc 2D complex array -> continuous chunk ---
int utlAlloc2DcomplArray(MKL_Complex8  ***pppA, int szX, int szY);

int utlFree2DcomplArray(MKL_Complex8 **ppA, int szX, int szY);

//------ initialize 2D complex array -----------
int utlSetToZero2DcomplArray(MKL_Complex8 **ppA, int szX, int szY);

//======= 2D float array =========
//------ alloc 2D float array -> continuous chunk ---
int utlAlloc2DfloatArray(float ***pppA, int szX, int szY);

int utlFree2DfloatArray(float **ppA, int szX, int szY);

//------ initialize 2D float array -----------
int utlSetToZero2DfloatArray(float **ppA, int szX, int szY);

//------ initialize 2D float array with some value -----------
// use C-style ZERO indexing,
int utlSetToVal2DfloatArrayC(float **ppA, int szX, int szY, float val);

///======= get_kxu(..) and staper(..) from zOsupport.f90 ====
int get_kxu(int nx, float dkx, float *kx, float *kx2);
int staper(float *taper, int ntaper);

#endif
