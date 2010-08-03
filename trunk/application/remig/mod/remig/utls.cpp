/*****************************************************************************
utls.cpp ->  utilities,
         - some of them replacing standard FORTRAN functions 
         
         - memory management of 2D and 3D arrays as well here
           1D arrays allocated and freed directly in the code
   
         - 2 functions from ZOsupport.f90 : get_kxu(..) and staper(..)

***********************************************************/

#include <stdio.h>
#include <strings.h> // bzero()

#include "utls.h"


//========= some FOTRAN REPLACEMENTS ===============
//-------- utlIAsum ------------------------------
int utlIAsum(int *pArr, int startIndx, int lastIndxToSumUp)
// replaces the fortran function SUM()
// mind the fortran indexing -> for now it will be called with
// startIndx == 1
{
    int sum = 0;

    for(int i = startIndx; i <= lastIndxToSumUp; i++) {
        sum += pArr[i];
    }     

    return sum;
}

//-------- utlIAprint ------------------------------
int utlIAprint(int *pArr, int startIndx, int lastIndxToSumUp)
// prints the elements of an integer array
// mind the fortran indexing -> for now it will be called with
// startIndx == 1
{

    printf("\n");

    for(int i = startIndx; i <= lastIndxToSumUp; i++) {
        printf("[%d]:%d  ", i, pArr[i]);
    }     

    return 1;
}

//--------- maxValInFloatArray -------
float maxValInFloat3DarrayC(float ***pppA, int szX, int szY, int szZ)
// NB!! the indexing is in C-style :
// - starts from ZERO
// - the last index == array_size-1
{
    int i, j, k;
    double maxVal = 0;

    for(i=0; i < szX; i++) 
	for(j = 0; j < szY; j++)
	    for(k=0; k < szZ; k++) 
		if(pppA[i][j][k] > maxVal) maxVal = pppA[i][j][k]; 
    
    return maxVal;
}

//--------- maxValInFloatArray -------
float minValInFloat3DarrayC(float ***pppA, int szX, int szY, int szZ)
// NB!! the indexing is in C-style :
// - starts from ZERO
// - the last index == array_size-1
{
    int i, j, k;
    double minVal = 1e40;

    for(i=0; i < szX; i++) 
	for(j = 0; j < szY; j++)
	    for(k=0; k < szZ; k++) {
		if(pppA[i][j][k] < minVal) { 
                    minVal = pppA[i][j][k]; 
		      printf(" crr min val:%e at [%d][%d][%d] \n", minVal, i, j, k); 
                }
            } 
    
    return minVal;
}

//---------- maxValInFloat2DarrayF --------------------------
float maxValInFloat2DarrayF(float **ppA, int szX, int szY)
// NB!! the indexing is in Fortran-style :
// - starts from 1
// - the last index == array_size
{
    int i, j;
    double maxVal = 0;

    for(i=1; i <= szX; i++) 
	for(j = 1; j <= szY; j++)
		if(ppA[i][j] > maxVal) maxVal = ppA[i][j]; 
    
    return maxVal;
}

//---------- minValInFloat2DarrayF --------------------------
float minValInFloat2DarrayF(float **ppA, int szX, int szY)
// NB!! the indexing is in Fortran-style :
// - starts from 1
// - the last index == array_size
{
    int i, j;
    double minVal = 1e100;

    for(i=1; i <= szX; i++) 
	for(j = 1; j <= szY; j++)
		if(ppA[i][j] < minVal) minVal = ppA[i][j]; 
    
    return minVal;
}

//=========== addition to the VM-variant ------------
int indxOffsetIn3DarrayInZplaneChunks(int iz, int ix, int iy, int szZ, int szX, int szY)
// NB!! z -> slowest index/dir
//      x -> slower than y, faster than z
//      y -> fasters index/dir
// In the VM-variant the arrays have been authomatically allocated,
// one knows the base, this function returns the offset of the item
// The function arguments (their ordering) differ from the ordering
// used in the other similar functions (it was (z,y,x)), but
// we will not mix the VM-variant with the older funcs.
// If everything is OK, the older functions will be deleted.
{
    int offs = iz * (szX*szY) + ix*szY + iy;

    return offs;
}

//=========== another addition to the VM-variant ------------
int indxOffsetIn3DarrayInXplaneChunks(int ix, int iy, int iz, int szX, int szY, int szZ)
// NB!! x -> slowest index/dir
//      y -> slower than z, faster than y
//      z -> fasters index/dir
// In the VM-variant the arrays have been authomatically allocated,
// one knows the base, this function returns the offset of the item
// The function arguments (their ordering) differ from the ordering
// used in the other similar functions (it was (z,y,x)), but
// we will not mix the VM-variant with the older funcs.
// If everything is OK, the older functions will be deleted.
//
// This indexing is needed to transpose the output array -> z-dir
//    z-direction must be the fastest one at the output
{
    int offs = ix * (szY*szZ) + iy*szZ + iz;

    return offs;
}


//============ 2D and 3D arrays memory management =====================
//----------- utlAlloc3DcomplArrayInZplaneChunks ------------------
int utlAlloc3DcomplArrayInZplaneChunks(MKL_Complex8 ****ppppA, int szZ, int szY, int szX)
// allocates 3D array, each Z=const plane being a whole chunk
// the reason: to fit the fortran requirements (where in a 3D array  ARR(k,j,i)
// [k] is the fastest index and [i] is the slowest one, when the array is allocated).
// Therefore, in the code there are MPI-transfers for k=const, sending the data 
// for a whole k=const [i][j]-plane, with a 2nd index [j] "faster" than the 3rd one [i].
// In order to re-program these transfers in C, allocate such arrays 
// in the appropriate way here
{
    int j, k, size;
    int flg = 1;

    *ppppA = new MKL_Complex8** [szZ];    
    if(*ppppA == NULL) {
	printf("\n ERROR: Can not allocate 3D complex array in z=const chunks !!!\n");
        flg=0; 
        return flg;
    }
    
    //------- size on each z=const chunk
    size = szX * szY;

    for(k=0; k < szZ; k++) {

        //-------- alloc continuous array for each Z=const plane ------
        //------- mind, that the Y-direction must be "slower" than the X-direction

        (*ppppA)[k] = new MKL_Complex8* [szY];
        if((*ppppA)[k] == NULL) {
	   printf("\n ERROR: Can not allocate 3D complex array in z=const chunks !!!\n");
           flg=0; 
           return flg;
         }

         (*ppppA)[k][0] = new  MKL_Complex8 [size];
         if((*ppppA)[k][0] == NULL) {
	    printf("\n ERROR: Can not allocate 3D complex array in z=const chunks !!!\n");
            flg=0; 
            return flg;
         }

         for(j = 0; j < szY; j++) {
	    (*ppppA)[k][j] =  (*ppppA)[k][0] + j*szX;
         }

    } // for(k

    return flg;
}

//----------- utlFree3DcomplArrayInZplaneChunks ------------------
int utlFree3DcomplArrayInZplaneChunks(MKL_Complex8 ***pppA, int szZ, int szY, int szX)
{

    for(int k=0; k < szZ; k++) {
       delete [] pppA[k][0];
       delete [] pppA[k];
    }
    delete [] pppA;

    return 1;
}

//----------- utlSetToZero3DcomplArrayInZplaneChunks ------------------
int utlSetToZero3DcomplArrayInZplaneChunks(MKL_Complex8 ***pppA, int szZ, int szY, int szX)
{
    int k, size;

    size=szY*szX;

    for(k=0; k < szZ; k++) {           
            bzero(pppA[k][0], size*sizeof(MKL_Complex8));
    } // for(k
   
    return 1;
}

//----------- utlAlloc3DfloatArrayInZplaneChunks ------------------
int utlAlloc3DfloatArrayInZplaneChunks(float ****ppppA, int szZ, int szY, int szX)
// allocates 3D array, each Z=const plane being a whole chunk
// the reason: to fit the fortran requirements (where in a 3D array  ARR(k,j,i)
// [k] is the fastest index and [i] is the slowest one, when the array is allocated).
// Therefore, in the code there are MPI-transfers for k=const, sending the data 
// for a whole k=const [i][j]-plane, with a 2nd index [j] "faster" than the 3rd one [i].
// In order to re-program these transfers in C, allocate such arrays 
// in the appropriate way here
{
    int j, k, size;
    int flg = 1;

    *ppppA = new float** [szZ];    
    if(*ppppA == NULL) {
	printf("\n ERROR: Can not allocate 3D float array in z=const chunks !!!\n");
        flg=0; 
        return flg;
    }
    
    //------- size on each z=const chunk
    size = szX * szY;


    for(k=0; k < szZ; k++) {

       //-------- alloc continuous array for each Z=const plane ------
        //------- mind, that the Y-direction must be "slower" than the X-direction
        (*ppppA)[k] = new float* [szY];
        if((*ppppA)[k] == NULL) {
	   printf("\n ERROR: Can not allocate 3D float array in z=const chunks !!!\n");
 	   printf("\n ERROR: Can not allocate 3D float array in z=const chunks !!!\n");
          flg=0; 
           return flg;
         }

         (*ppppA)[k][0] = new  float [size];
         if((*ppppA)[k][0] == NULL) {
	    printf("\n ERROR: Can not allocate 3D float array in z=const chunks !!!\n");
            flg=0; 
            return flg;
         }

         for(j = 0; j < szY; j++) {
	     (*ppppA)[k][j] =  (*ppppA)[k][0] + j*szX;
         }

    } // for(k

    return flg;
}

//----------- utlFree3DfloatArrayInZplaneChunks ------------------
int utlFree3DfloatArrayInZplaneChunks(float ***pppA, int szZ, int szY, int szX)
{

    for(int k=0; k < szZ; k++) {
       delete [] pppA[k][0];
       delete [] pppA[k];
    }
    delete [] pppA;

    return 1;
}

//----------- utlSetToZero3DfloatArrayInZplaneChunks ------------------
int utlSetToZero3DfloatArrayInZplaneChunks(float ***pppA, int szZ, int szY, int szX)
{
    int k, size;

    size=szY*szX;

    for(k=0; k < szZ; k++) {           
            bzero(pppA[k][0], size*sizeof(float));
    } // for(k
   
    return 1;
}

//----------- alloc 3D complex array -> non-continuous chunk ---
int utlAlloc3DcomplArray(MKL_Complex8 ****ppppA, int szX, int szY, int szZ)
{

    int i, j;
    int flg = 1;

    *ppppA = new MKL_Complex8** [szX];
    if(*ppppA == NULL) {
	printf("\n ERROR: Can not allocate 3D complex array !!!\n");
        flg=0; 
        return flg;
    }
    for(i=0; i < szX; i++) {
	(*ppppA)[i] = new MKL_Complex8* [szY];
        if((*ppppA)[i] == NULL) {
	  printf("\n ERROR: Can not allocate 3D complex array !!!\n");
          flg=0; 
          return flg;
        } 
        for(j = 0; j < szY; j++) {
	    (*ppppA)[i][j] = new MKL_Complex8 [szZ];
            if((*ppppA)[i][j] == NULL) {
	       printf("\n ERROR: Can not allocate 3D complex array !!!\n");
              flg=0; 
              return flg;
             } 
        }  // for(j
    } // for(i


    return flg;
}

//------------ utlFree3DcomplArray ---------------------------
int utlFree3DcomplArray(MKL_Complex8 ***pppA, int szX, int szY, int szZ)
{
    int i, j;

    for(i=0; i < szX; i++) 
	for(j=0; j < szY; j++) 
	    delete [] pppA[i][j];

    for(i=0; i < szX; i++) 
	 delete [] pppA[i];   

    delete [] pppA; 
 
    return 1;
}

//----------- utlSetToZero3DcomplArray -----------------------------
int utlSetToZero3DcomplArray(MKL_Complex8 ***pppA, int szX, int szY, int szZ)
{
    int i, j;

    for(i=0; i < szX; i++) {
        for(j = 0; j < szY; j++) {
           
            bzero(pppA[i][j], szZ*sizeof(MKL_Complex8));

        }  // for(j
    } // for(i



    return 1;
}

//------ alloc 3D float array -> non-continuous chunk ---
int utlAlloc3DfloatArray(float ****ppppA, int szX, int szY, int szZ)
{
    int i, j;
    int flg = 1;

    *ppppA = new float** [szX];
    if(*ppppA == NULL) {
	printf("\n ERROR: Can not allocate 3D float array !!!\n");
        flg=0; 
        return flg;
    }
    for(i=0; i < szX; i++) {
	(*ppppA)[i] = new float* [szY];
        if((*ppppA)[i] == NULL) {
	  printf("\n ERROR: Can not allocate 3D float array !!!\n");
          flg=0; 
          return flg;
        } 
        for(j = 0; j < szY; j++) {
	    (*ppppA)[i][j] = new float [szZ];
            if((*ppppA)[i][j] == NULL) {
	       printf("\n ERROR: Can not allocate 3D float array !!!\n");
              flg=0; 
              return flg;
            } 
        }  // for(j
    } // for(i

    return flg;
}

//---------- utlFree3DfloatArray -----------------
int utlFree3DfloatArray(float  ***pppA, int szX, int szY, int szZ)
{
    int i, j;

    for(i=0; i < szX; i++) 
	for(j=0; j < szY; j++) 
	    delete [] pppA[i][j];

    for(i=0; i < szX; i++) 
	 delete [] pppA[i];   

    delete [] pppA; 

    return 1;
}

//------ initialize 3D float array -----------
int utlSetToZero3DfloatArray(float ***pppA, int szX, int szY, int szZ)
{
    int i, j;

    for(i=0; i < szX; i++) {
        for(j = 0; j < szY; j++) {
           
            bzero(pppA[i][j], szZ*sizeof(float));

        }  // for(j
    } // for(i

    return 1;
}


//------ alloc 2D complex array -> continuous chunk ---
int utlAlloc2DcomplArray(MKL_Complex8  ***pppA, int szX, int szY)
{
    int size, i, j, indx;
    int flg = 1;

    //-------- alloc continuous array ------
    size = szX * szY;

    *pppA = new MKL_Complex8* [szX];
    if(*pppA == NULL) {
	printf("\n ERROR: Can not allocate 2D complex array !!!\n");
        flg=0; 
        return flg;
    }

    (*pppA)[0] = new  MKL_Complex8 [size];
    if((*pppA)[0] == NULL) {
	printf("\n ERROR: Can not allocate 2D complex array !!!\n");
        flg=0; 
        return flg;
    }

    for(i = 0; i < szX; i++) {
	(*pppA)[i] =  (*pppA)[0] + i*szY;
    }

    return flg;
}

//--------- utlFree2DcomplArray ---------------------
int utlFree2DcomplArray(MKL_Complex8 **ppA, int szX, int szY)
{
    delete [] ppA[0];

    delete [] ppA;


    return 1;
}

//------ initialize 2D complex array -----------
int utlSetToZero2DcomplArray(MKL_Complex8 **ppA, int szX, int szY)
{

    int size = szX*szY;

    bzero(ppA[0], size*sizeof(MKL_Complex8));

    return 1;
}

//------ alloc 2D float array -> continuous chunk ---
int utlAlloc2DfloatArray(float ***pppA, int szX, int szY)
{
    int size, i, j, indx;
    int flg = 1;

    //-------- alloc continuous array ------
    size = szX * szY;

    *pppA = new float* [szX];
    if(*pppA == NULL) {
	printf("\n ERROR: Can not allocate 2D float array !!!\n");
        flg=0; 
        return flg;
    }

    (*pppA)[0] = new  float [size];
    if((*pppA)[0] == NULL) {
	printf("\n ERROR: Can not allocate 2D float array !!!\n");
        flg=0; 
        return flg;
    }

    for(i = 0; i < szX; i++) {
	(*pppA)[i] =  (*pppA)[0] + i*szY;
    }

    return flg;
}


//---------- utlFree2DfloatArray ---------------------
int utlFree2DfloatArray(float **ppA, int szX, int szY)
{
    delete [] ppA[0];

    delete [] ppA;

    return 1;
}


//------ initialize 2D float array -----------
int utlSetToZero2DfloatArray(float **ppA, int szX, int szY)
{

    int size = szX*szY;

    bzero(ppA[0], size*sizeof(float));

    return 1;
}

//------ initialize 2D float array with some value -----------
int utlSetToVal2DfloatArrayC(float **ppA, int szX, int szY, float val)
// use C-style ZERO indexing,
{

    int i, j;

    for(i=0; i < szX; i++) {
	for(j=0; j < szY; j++) {
	    ppA[i][j] = val;
        }
    }

    return 1;
}


//======= get_kxu(..) and staper(..) from zOsupport.f90 ====

//------ get_kxu --------------------------------
//!***************************************************
//! Horizontal wavenumber, kx(nx)
//!***************************************************
//subroutine get_kxu(nx, dkx, kx, kx2)                            
//      implicit none
//      integer  nx,nxhalf,nx0,nxp2,ix
//      real     dkx
//      real     kx(nx), kx2(nx)
//      return
      
//end subroutine get_kxu

int get_kxu(int nx, float dkx, float *kx, float *kx2)
{
    int nxhalf,nx0,nxp2,ix;

    nxhalf = nx/2;
    nx0    = nxhalf + 1;
    nxp2   = nx + 2; 
                       
    kx[1]=0;
    kx2[1]=0;
                      
    for(ix=2; ix <= nxhalf; ix++) { //do ix=2,nxhalf 
	kx[ix]=(ix-1)*dkx; //kx(ix)=(ix-1)*dkx
        kx2[ix]=kx[ix]*kx[ix]; // kx2(ix)=kx(ix)*kx(ix)
    }//  enddo
         
    kx[nx0] = -nxhalf*dkx;    //  kx(nx0)=-nxhalf*dkx 
    kx2[nx0] = kx[nx0]*kx[nx0]; //  kx2(nx0)=kx(nx0)*kx(nx0)
      
    for(ix=2; ix <= nxhalf; ix++) { //  do ix=2,nxhalf
        kx[nxp2-ix] = -kx[ix]; // kx(nxp2-ix)=-kx(ix) 
	kx2[nxp2-ix] = kx2[ix];  //kx2(nxp2-ix)=kx2(ix)
    } //  enddo

    return 1;
}

//!**********************************************************
//!Taper in space, (quasi)Hanning type
//!Input  : taper(1:ntaper) : (empty array)
//!Output : taper(1:ntaper) : taper coefficients
//!**********************************************************
// subroutine staper(taper,ntaper)
// implicit none
// integer :: k,ntp        
// integer, intent(in)                  :: ntaper
// real*4, dimension(ntaper),intent(out) :: taper
//!

// return
//end subroutine staper


int staper(float *taper, int ntaper)
{
    int k, ntp;

    ntp=ntaper-1;
    for(k=0; k <= ntaper; k++) taper[k]=1.; //taper=1.

    if(ntaper <= 3) { //if (ntaper.lt.3) then
	printf("\n Note : ntaper<3, thus no wavefield taper applied! \n");
        return 1;
    } //end if

//! Cosine taper
    for(k=1; k <= ntaper; k++) { //do k=1,ntaper
	taper[k]=0.5-0.5*cos(3.1415926*(k-1)/ntp); //taper(k)=0.5-0.5*cos(3.1415926*(k-1)/ntp)
    } //end do

    return 1;
}
