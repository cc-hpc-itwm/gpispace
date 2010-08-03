/* Copyright (c) Colorado School of Mines, 1995.*/
/* All rights reserved.                       */

/*********************** self documentation **********************/
/*****************************************************************************
PFAFFT - Functions to perform Prime Factor (PFA) FFT's, in place

npfao		return optimal n for complex-to-complex PFA
pfacc		1D PFA complex to complex

*****************************************************************************
Function Prototypes:
int npfao (int nmin, int nmax);
void pfacc (int isign, int n, complex z[]);

******************************************************************************
npfao
Input:
nmin		lower bound on returned value (see notes below)
nmax		desired (but not guaranteed) upper bound on returned value

Returned:	valid n for prime factor fft

******************************************************************************
pfacc:
Input:
isign		sign of isign is the sign of exponent in fourier kernel
n		length of transform (see notes below)
z		array[n] of complex numbers to be transformed in place

Output:
z		array[n] of complex numbers transformed

******************************************************************************
Notes:
Table of valid n and cost for prime factor fft.  For each n, cost
was estimated to be the inverse of the number of ffts done in 1 sec
on an IBM RISC System/6000 Model 320H, by Dave Hale, 08/04/91.
(Redone by Jack Cohen for 15 sec to rebuild NTAB table on advice of
David and Gregory Chudnovsky, 05/03/94).
Cost estimates are least accurate for very small n.  An alternative method
for estimating cost would be to count multiplies and adds, but this
method fails to account for the overlapping of multiplies and adds
that is possible on some computers, such as the IBM RS/6000 family.

npfao:
The returned n will be composed of mutually prime factors from
the set {2,3,4,5,7,8,9,11,13,16}.  Because n cannot exceed
720720 = 5*7*9*11*13*16, 720720 is returned if nmin exceeds 720720.
If nmin does not exceed 720720, then the returned n will not be 
less than nmin.  The optimal n is chosen to minimize the estimated
cost of performing the fft, while satisfying the constraint, if
possible, that n not exceed nmax.

pfacc:
n must be factorable into mutually prime factors taken 
from the set {2,3,4,5,7,8,9,11,13,16}.  in other words,
	n = 2**p * 3**q * 5**r * 7**s * 11**t * 13**u
where
	0 <= p <= 4,  0 <= q <= 2,  0 <= r,s,t,u <= 1
is required for pfa to yield meaningful results.  this
restriction implies that n is restricted to the range
	1 <= n <= 720720 (= 5*7*9*11*13*16)

******************************************************************************
References:  
Temperton, C., 1985, Implementation of a self-sorting
in-place prime factor fft algorithm:  Journal of
Computational Physics, v. 58, p. 283-299.

Temperton, C., 1988, A new set of minimum-add rotated
rotated dft modules: Journal of Computational Physics,
v. 75, p. 190-198.

Press et al, 1988, Numerical Recipes in C, p. 417.

******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 04/27/89
*****************************************************************************/
/**************** end self doc ********************************/

#include "cwp.h"

#define NTAB 240
static struct {
int n;  float c;
} nctab[NTAB] = {
{       1, 0.000052 },
{       2, 0.000061 },
{       3, 0.000030 },
{       4, 0.000053 },
{       5, 0.000066 },
{       6, 0.000067 },
{       7, 0.000071 },
{       8, 0.000062 },
{       9, 0.000079 },
{      10, 0.000080 },
{      11, 0.000052 },
{      12, 0.000069 },
{      13, 0.000103 },
{      14, 0.000123 },
{      15, 0.000050 },
{      16, 0.000086 },
{      18, 0.000108 },
{      20, 0.000101 },
{      21, 0.000098 },
{      22, 0.000135 },
{      24, 0.000090 },
{      26, 0.000165 },
{      28, 0.000084 },
{      30, 0.000132 },
{      33, 0.000158 },
{      35, 0.000138 },
{      36, 0.000147 },
{      39, 0.000207 },
{      40, 0.000156 },
{      42, 0.000158 },
{      44, 0.000176 },
{      45, 0.000171 },
{      48, 0.000185 },
{      52, 0.000227 },
{      55, 0.000242 },
{      56, 0.000194 },
{      60, 0.000215 },
{      63, 0.000233 },
{      65, 0.000288 },
{      66, 0.000271 },
{      70, 0.000248 },
{      72, 0.000247 },
{      77, 0.000285 },
{      78, 0.000395 },
{      80, 0.000285 },
{      84, 0.000209 },
{      88, 0.000332 },
{      90, 0.000321 },
{      91, 0.000372 },
{      99, 0.000400 },
{     104, 0.000391 },
{     105, 0.000358 },
{     110, 0.000440 },
{     112, 0.000367 },
{     117, 0.000494 },
{     120, 0.000413 },
{     126, 0.000424 },
{     130, 0.000549 },
{     132, 0.000480 },
{     140, 0.000450 },
{     143, 0.000637 },
{     144, 0.000497 },
{     154, 0.000590 },
{     156, 0.000626 },
{     165, 0.000654 },
{     168, 0.000536 },
{     176, 0.000656 },
{     180, 0.000611 },
{     182, 0.000730 },
{     195, 0.000839 },
{     198, 0.000786 },
{     208, 0.000835 },
{     210, 0.000751 },
{     220, 0.000826 },
{     231, 0.000926 },
{     234, 0.000991 },
{     240, 0.000852 },
{     252, 0.000820 },
{     260, 0.001053 },
{     264, 0.000987 },
{     273, 0.001152 },
{     280, 0.000952 },
{     286, 0.001299 },
{     308, 0.001155 },
{     312, 0.001270 },
{     315, 0.001156 },
{     330, 0.001397 },
{     336, 0.001173 },
{     360, 0.001259 },
{     364, 0.001471 },
{     385, 0.001569 },
{     390, 0.001767 },
{     396, 0.001552 },
{     420, 0.001516 },
{     429, 0.002015 },
{     440, 0.001748 },
{     455, 0.001988 },
{     462, 0.001921 },
{     468, 0.001956 },
{     495, 0.002106 },
{     504, 0.001769 },
{     520, 0.002196 },
{     528, 0.002127 },
{     546, 0.002454 },
{     560, 0.002099 },
{     572, 0.002632 },
{     585, 0.002665 },
{     616, 0.002397 },
{     624, 0.002711 },
{     630, 0.002496 },
{     660, 0.002812 },
{     693, 0.002949 },
{     715, 0.003571 },
{     720, 0.002783 },
{     728, 0.003060 },
{     770, 0.003392 },
{     780, 0.003553 },
{     792, 0.003198 },
{     819, 0.003726 },
{     840, 0.003234 },
{     858, 0.004354 },
{     880, 0.003800 },
{     910, 0.004304 },
{     924, 0.003975 },
{     936, 0.004123 },
{     990, 0.004517 },
{    1001, 0.005066 },
{    1008, 0.003902 },
{    1040, 0.004785 },
{    1092, 0.005017 },
{    1144, 0.005599 },
{    1155, 0.005380 },
{    1170, 0.005730 },
{    1232, 0.005323 },
{    1260, 0.005112 },
{    1287, 0.006658 },
{    1320, 0.005974 },
{    1365, 0.006781 },
{    1386, 0.006413 },
{    1430, 0.007622 },
{    1456, 0.006679 },
{    1540, 0.007032 },
{    1560, 0.007538 },
{    1584, 0.007126 },
{    1638, 0.007979 },
{    1680, 0.007225 },
{    1716, 0.008961 },
{    1820, 0.008818 },
{    1848, 0.008427 },
{    1872, 0.009004 },
{    1980, 0.009398 },
{    2002, 0.010830 },
{    2145, 0.012010 },
{    2184, 0.010586 },
{    2288, 0.012058 },
{    2310, 0.011673 },
{    2340, 0.011700 },
{    2520, 0.011062 },
{    2574, 0.014313 },
{    2640, 0.013021 },
{    2730, 0.014606 },
{    2772, 0.013216 },
{    2860, 0.015789 },
{    3003, 0.016988 },
{    3080, 0.014911 },
{    3120, 0.016393 },
{    3276, 0.016741 },
{    3432, 0.018821 },
{    3465, 0.018138 },
{    3640, 0.018892 },
{    3696, 0.018634 },
{    3960, 0.020216 },
{    4004, 0.022455 },
{    4095, 0.022523 },
{    4290, 0.026087 },
{    4368, 0.023474 },
{    4620, 0.024590 },
{    4680, 0.025641 },
{    5005, 0.030303 },
{    5040, 0.025253 },
{    5148, 0.030364 },
{    5460, 0.031250 },
{    5544, 0.029412 },
{    5720, 0.034404 },
{    6006, 0.037500 },
{    6160, 0.034091 },
{    6435, 0.040214 },
{    6552, 0.037221 },
{    6864, 0.042735 },
{    6930, 0.040214 },
{    7280, 0.042980 },
{    7920, 0.045872 },
{    8008, 0.049505 },
{    8190, 0.049834 },
{    8580, 0.055762 },
{    9009, 0.057034 },
{    9240, 0.054945 },
{    9360, 0.056818 },
{   10010, 0.066667 },
{   10296, 0.065502 },
{   10920, 0.068182 },
{   11088, 0.065217 },
{   11440, 0.075000 },
{   12012, 0.078534 },
{   12870, 0.087719 },
{   13104, 0.081081 },
{   13860, 0.084270 },
{   15015, 0.102740 },
{   16016, 0.106383 },
{   16380, 0.105634 },
{   17160, 0.119048 },
{   18018, 0.123967 },
{   18480, 0.119048 },
{   20020, 0.137615 },
{   20592, 0.140187 },
{   21840, 0.154639 },
{   24024, 0.168539 },
{   25740, 0.180723 },
{   27720, 0.180723 },
{   30030, 0.220588 },
{   32760, 0.241935 },
{   34320, 0.254237 },
{   36036, 0.254237 },
{   40040, 0.288462 },
{   45045, 0.357143 },
{   48048, 0.357143 },
{   51480, 0.384615 },
{   55440, 0.384615 },
{   60060, 0.454545 },
{   65520, 0.517241 },
{   72072, 0.576923 },
{   80080, 0.625000 },
{   90090, 0.833333 },
{  102960, 0.789474 },
{  120120, 1.153846 },
{  144144, 1.153846 },
{  180180, 1.875000 },
{  240240, 2.500000 },
{  360360, 3.750000 },
{  720720, 7.500000 },
};

/*****************************************************************************
Return optimal n between nmin and nmax for prime factor fft.
******************************************************************************
Input:
nmin		lower bound on returned value (see notes below)
nmax		desired (but not guaranteed) upper bound on returned value

Returned:	valid n for prime factor fft
******************************************************************************
Notes:
The returned n will be composed of mutually prime factors from
the set {2,3,4,5,7,8,9,11,13,16}.  Because n cannot exceed
720720 = 5*7*9*11*13*16, 720720 is returned if nmin exceeds 720720.
If nmin does not exceed 720720, then the returned n will not be 
less than nmin.  The optimal n is chosen to minimize the estimated
cost of performing the fft, while satisfying the constraint, if
possible, that n not exceed nmax.
******************************************************************************
Author:  Dave Hale, Colorado School of Mines, 06/13/89
Modified:  Dave Hale, Colorado School of Mines, 08/05/91
	For efficiency, use pre-computed table of valid n and costs.
*****************************************************************************/
int npfao (int nmin, int nmax)
{
	int i,j;
	for (i=0; i<NTAB-1 && nctab[i].n<nmin; ++i);
	for (j=i+1; j<NTAB-1 && nctab[j].n<=nmax; ++j)
		if (nctab[j].c<nctab[i].c) i = j;
	return nctab[i].n;
}

void fftolen_(int* nmin, int* nmax, int* npt)
{
   int i;
   *npt = npfao(*nmin, *nmax);
   i = *npt;
  	while( (*npt%2)==1 ){
  	   i++;
  	   *npt = npfao(i, *nmax);
  	}
}

#define P120 0.120536680
#define P142 0.142314838
#define P173 0.173648178
#define P222 0.222520934
#define P239 0.239315664
#define P281 0.281732557
#define P342 0.342020143
#define P354 0.354604887
#define P382 0.382683432
#define P415 0.415415013
#define P433 0.433883739
#define P464 0.464723172
#define P540 0.540640817
#define P559 0.559016994
#define P568 0.568064747
#define P587 0.587785252
#define P623 0.623489802
#define P642 0.642787610
#define P654 0.654860734
#define P663 0.663122658
#define P707 0.707106781
#define P748 0.748510748
#define P755 0.755749574
#define P766 0.766044443
#define P781 0.781831482
#define P822 0.822983866
#define P841 0.841253533
#define P866 0.866025404
#define P885 0.885456026
#define P900 0.900968868
#define P909 0.909631995
#define P923 0.923879533
#define P935 0.935016243
#define P939 0.939692621
#define P951 0.951056516
#define P959 0.959492974
#define P970 0.970941817
#define P974 0.974927912
#define P984 0.984807753
#define P989 0.989821442
#define P992 0.992708874
#define NFAX 10

/*****************************************************************************
Prime factor fft:  complex to complex transform, in place
******************************************************************************
Input:
isign		sign of isign is the sign of exponent in fourier kernel
n		length of transform (see notes below)
z		array[n] of complex numbers to be transformed in place

Output:
z		array[n] of complex numbers transformed
******************************************************************************
Notes:
n must be factorable into mutually prime factors taken 
from the set {2,3,4,5,7,8,9,11,13,16}.  in other words,
	n = 2**p * 3**q * 5**r * 7**s * 11**t * 13**u
where
	0 <= p <= 4,  0 <= q <= 2,  0 <= r,s,t,u <= 1
is required for pfa to yield meaningful results.  this
restriction implies that n is restricted to the range
	1 <= n <= 720720 (= 5*7*9*11*13*16)
*****************************************************************************/
void pfacc(int isign, int n, complex cz[])
{
	static int kfax[] = { 16,13,11,9,8,7,5,4,3,2 };
	register float *z=(float*)cz;
	register int j0,j1,j2,j3,j4,j5,j6,j7,j8,j9,j10,j11,j12,j13,j14,j15,jt;
	int nleft,jfax,ifac,jfac,jinc,jmax,ndiv,m,mm=0,mu=0,l;
	float t1r,t1i,t2r,t2i,t3r,t3i,t4r,t4i,t5r,t5i,
		t6r,t6i,t7r,t7i,t8r,t8i,t9r,t9i,t10r,t10i,
		t11r,t11i,t12r,t12i,t13r,t13i,t14r,t14i,t15r,t15i,
		t16r,t16i,t17r,t17i,t18r,t18i,t19r,t19i,t20r,t20i,
		t21r,t21i,t22r,t22i,t23r,t23i,t24r,t24i,t25r,t25i,
		t26r,t26i,t27r,t27i,t28r,t28i,t29r,t29i,t30r,t30i,
		t31r,t31i,t32r,t32i,t33r,t33i,t34r,t34i,t35r,t35i,
		t36r,t36i,t37r,t37i,t38r,t38i,t39r,t39i,t40r,t40i,
		t41r,t41i,t42r,t42i,
		y1r,y1i,y2r,y2i,y3r,y3i,y4r,y4i,y5r,y5i,
		y6r,y6i,y7r,y7i,y8r,y8i,y9r,y9i,y10r,y10i,
		y11r,y11i,y12r,y12i,y13r,y13i,y14r,y14i,y15r,y15i,
		c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12;

	/* keep track of n left after dividing by factors */
	nleft = n;

	/* begin loop over possible factors (from biggest to smallest) */
	for (jfax=0; jfax<NFAX; jfax++) {

		/* skip if not a mutually prime factor of n */
        ifac = kfax[jfax];
        ndiv = nleft/ifac;
        if (ndiv*ifac!=nleft) continue;
 
		/* update n left and determine n divided by factor */
        nleft = ndiv;
        m = n/ifac;
 
		/* determine rotation factor mu and stride mm */
        for (jfac=1; jfac<=ifac; jfac++) {
			mu = jfac;
			mm = jfac*m;
			if (mm%ifac==1) break;
		}
 
		/* adjust rotation factor for sign of transform */
        if (isign<0) mu = ifac-mu;
 
		/* compute stride, limit, and pointers */
        jinc = 2*mm;
		jmax = 2*n;
        j0 = 0;
        j1 = j0+jinc;

		/* if factor is 2 */
        if (ifac==2) {
			for (l=0; l<m; l++) {
				t1r = z[j0]-z[j1];
				t1i = z[j0+1]-z[j1+1];
				z[j0] = z[j0]+z[j1];
				z[j0+1] = z[j0+1]+z[j1+1];
				z[j1] = t1r;
				z[j1+1] = t1i;
				jt = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
        j2 = j1+jinc;
        if (j2>=jmax) j2 = j2-jmax;

		/* if factor is 3 */
        if (ifac==3) {
			if (mu==1)
				c1 = P866;
			else
				c1 = -P866;
			for (l=0; l<m; l++) {
				t1r = z[j1]+z[j2];
				t1i = z[j1+1]+z[j2+1];
				y1r = z[j0]-0.5*t1r;
				y1i = z[j0+1]-0.5*t1i;
				y2r = c1*(z[j1]-z[j2]);
				y2i = c1*(z[j1+1]-z[j2+1]);
				z[j0] = z[j0]+t1r;
				z[j0+1] = z[j0+1]+t1i;
				z[j1] = y1r-y2i;
				z[j1+1] = y1i+y2r;
				z[j2] = y1r+y2i;
				z[j2+1] = y1i-y2r;
				jt = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j3 = j2+jinc;
		if (j3>=jmax) j3 = j3-jmax;

		/* if factor is 4 */
		if (ifac==4) {
			if (mu==1)
				c1 = 1.0;
			else
				c1 = -1.0;
			for (l=0; l<m; l++) {
				t1r = z[j0]+z[j2];
				t1i = z[j0+1]+z[j2+1];
				t2r = z[j1]+z[j3];
				t2i = z[j1+1]+z[j3+1];
				y1r = z[j0]-z[j2];
				y1i = z[j0+1]-z[j2+1];
				y3r = c1*(z[j1]-z[j3]);
				y3i = c1*(z[j1+1]-z[j3+1]);
				z[j0] = t1r+t2r;
				z[j0+1] = t1i+t2i;
				z[j1] = y1r-y3i;
				z[j1+1] = y1i+y3r;
				z[j2] = t1r-t2r;
				z[j2+1] = t1i-t2i;
				z[j3] = y1r+y3i;
				z[j3+1] = y1i-y3r;
				jt = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j4 = j3+jinc;
		if (j4>=jmax) j4 = j4-jmax;

		/* if factor is 5 */
		if (ifac==5) {
			if (mu==1) {
				c1 = P559;
				c2 = P951;
				c3 = P587;
			} else if (mu==2) {
				c1 = -P559;
				c2 = P587;
				c3 = -P951;
			} else if (mu==3) {
				c1 = -P559;
				c2 = -P587;
				c3 = P951;
			} else { 
				c1 = P559;
				c2 = -P951;
				c3 = -P587;
			}
			for (l=0; l<m; l++) {
				t1r = z[j1]+z[j4];
				t1i = z[j1+1]+z[j4+1];
				t2r = z[j2]+z[j3];
				t2i = z[j2+1]+z[j3+1];
				t3r = z[j1]-z[j4];
				t3i = z[j1+1]-z[j4+1];
				t4r = z[j2]-z[j3];
				t4i = z[j2+1]-z[j3+1];
				t5r = t1r+t2r;
				t5i = t1i+t2i;
				t6r = c1*(t1r-t2r);
				t6i = c1*(t1i-t2i);
				t7r = z[j0]-0.25*t5r;
				t7i = z[j0+1]-0.25*t5i;
				y1r = t7r+t6r;
				y1i = t7i+t6i;
				y2r = t7r-t6r;
				y2i = t7i-t6i;
				y3r = c3*t3r-c2*t4r;
				y3i = c3*t3i-c2*t4i;
				y4r = c2*t3r+c3*t4r;
				y4i = c2*t3i+c3*t4i;
				z[j0] = z[j0]+t5r;
				z[j0+1] = z[j0+1]+t5i;
				z[j1] = y1r-y4i;
				z[j1+1] = y1i+y4r;
				z[j2] = y2r-y3i;
				z[j2+1] = y2i+y3r;
				z[j3] = y2r+y3i;
				z[j3+1] = y2i-y3r;
				z[j4] = y1r+y4i;
				z[j4+1] = y1i-y4r;
				jt = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j5 = j4+jinc;
		if (j5>=jmax) j5 = j5-jmax;
		j6 = j5+jinc;
		if (j6>=jmax) j6 = j6-jmax;

		/* if factor is 7 */
		if (ifac==7) {
			if (mu==1) {
				c1 = P623;
				c2 = -P222;
				c3 = -P900;
				c4 = P781;
				c5 = P974;
				c6 = P433;
			} else if (mu==2) {
				c1 = -P222;
				c2 = -P900;
				c3 = P623;
				c4 = P974;
				c5 = -P433;
				c6 = -P781;
			} else if (mu==3) {
				c1 = -P900;
				c2 = P623;
				c3 = -P222;
				c4 = P433;
				c5 = -P781;
				c6 = P974;
			} else if (mu==4) {
				c1 = -P900;
				c2 = P623;
				c3 = -P222;
				c4 = -P433;
				c5 = P781;
				c6 = -P974;
			} else if (mu==5) {
				c1 = -P222;
				c2 = -P900;
				c3 = P623;
				c4 = -P974;
				c5 = P433;
				c6 = P781;
			} else {
				c1 = P623;
				c2 = -P222;
				c3 = -P900;
				c4 = -P781;
				c5 = -P974;
				c6 = -P433;
			}
			for (l=0; l<m; l++) {
				t1r = z[j1]+z[j6];
				t1i = z[j1+1]+z[j6+1];
				t2r = z[j2]+z[j5];
				t2i = z[j2+1]+z[j5+1];
				t3r = z[j3]+z[j4];
				t3i = z[j3+1]+z[j4+1];
				t4r = z[j1]-z[j6];
				t4i = z[j1+1]-z[j6+1];
				t5r = z[j2]-z[j5];
				t5i = z[j2+1]-z[j5+1];
				t6r = z[j3]-z[j4];
				t6i = z[j3+1]-z[j4+1];
				t7r = z[j0]-0.5*t3r;
				t7i = z[j0+1]-0.5*t3i;
				t8r = t1r-t3r;
				t8i = t1i-t3i;
				t9r = t2r-t3r;
				t9i = t2i-t3i;
				y1r = t7r+c1*t8r+c2*t9r;
				y1i = t7i+c1*t8i+c2*t9i;
				y2r = t7r+c2*t8r+c3*t9r;
				y2i = t7i+c2*t8i+c3*t9i;
				y3r = t7r+c3*t8r+c1*t9r;
				y3i = t7i+c3*t8i+c1*t9i;
				y4r = c6*t4r-c4*t5r+c5*t6r;
				y4i = c6*t4i-c4*t5i+c5*t6i;
				y5r = c5*t4r-c6*t5r-c4*t6r;
				y5i = c5*t4i-c6*t5i-c4*t6i;
				y6r = c4*t4r+c5*t5r+c6*t6r;
				y6i = c4*t4i+c5*t5i+c6*t6i;
				z[j0] = z[j0]+t1r+t2r+t3r;
				z[j0+1] = z[j0+1]+t1i+t2i+t3i;
				z[j1] = y1r-y6i;
				z[j1+1] = y1i+y6r;
				z[j2] = y2r-y5i;
				z[j2+1] = y2i+y5r;
				z[j3] = y3r-y4i;
				z[j3+1] = y3i+y4r;
				z[j4] = y3r+y4i;
				z[j4+1] = y3i-y4r;
				z[j5] = y2r+y5i;
				z[j5+1] = y2i-y5r;
				z[j6] = y1r+y6i;
				z[j6+1] = y1i-y6r;
				jt = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j7 = j6+jinc;
		if (j7>=jmax) j7 = j7-jmax;

		/* if factor is 8 */
		if (ifac==8) {
			if (mu==1) {
				c1 = 1.0;
				c2 = P707;
			} else if (mu==3) {
				c1 = -1.0;
				c2 = -P707;
			} else if (mu==5) {
				c1 = 1.0;
				c2 = -P707;
			} else {
				c1 = -1.0;
				c2 = P707;
			}
			c3 = c1*c2;
			for (l=0; l<m; l++) {
				t1r = z[j0]+z[j4];
				t1i = z[j0+1]+z[j4+1];
				t2r = z[j0]-z[j4];
				t2i = z[j0+1]-z[j4+1];
				t3r = z[j1]+z[j5];
				t3i = z[j1+1]+z[j5+1];
				t4r = z[j1]-z[j5];
				t4i = z[j1+1]-z[j5+1];
				t5r = z[j2]+z[j6];
				t5i = z[j2+1]+z[j6+1];
				t6r = c1*(z[j2]-z[j6]);
				t6i = c1*(z[j2+1]-z[j6+1]);
				t7r = z[j3]+z[j7];
				t7i = z[j3+1]+z[j7+1];
				t8r = z[j3]-z[j7];
				t8i = z[j3+1]-z[j7+1];
				t9r = t1r+t5r;
				t9i = t1i+t5i;
				t10r = t3r+t7r;
				t10i = t3i+t7i;
				t11r = c2*(t4r-t8r);
				t11i = c2*(t4i-t8i);
				t12r = c3*(t4r+t8r);
				t12i = c3*(t4i+t8i);
				y1r = t2r+t11r;
				y1i = t2i+t11i;
				y2r = t1r-t5r;
				y2i = t1i-t5i;
				y3r = t2r-t11r;
				y3i = t2i-t11i;
				y5r = t12r-t6r;
				y5i = t12i-t6i;
				y6r = c1*(t3r-t7r);
				y6i = c1*(t3i-t7i);
				y7r = t12r+t6r;
				y7i = t12i+t6i;
				z[j0] = t9r+t10r;
				z[j0+1] = t9i+t10i;
				z[j1] = y1r-y7i;
				z[j1+1] = y1i+y7r;
				z[j2] = y2r-y6i;
				z[j2+1] = y2i+y6r;
				z[j3] = y3r-y5i;
				z[j3+1] = y3i+y5r;
				z[j4] = t9r-t10r;
				z[j4+1] = t9i-t10i;
				z[j5] = y3r+y5i;
				z[j5+1] = y3i-y5r;
				z[j6] = y2r+y6i;
				z[j6+1] = y2i-y6r;
				z[j7] = y1r+y7i;
				z[j7+1] = y1i-y7r;
				jt = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j8 = j7+jinc;
		if (j8>=jmax) j8 = j8-jmax;

		/* if factor is 9 */
		if (ifac==9) {
			if (mu==1) {
				c1 = P866;
				c2 = P766;
				c3 = P642;
				c4 = P173;
				c5 = P984;
			} else if (mu==2) {
				c1 = -P866;
				c2 = P173;
				c3 = P984;
				c4 = -P939;
				c5 = P342;
			} else if (mu==4) {
				c1 = P866;
				c2 = -P939;
				c3 = P342;
				c4 = P766;
				c5 = -P642;
			} else if (mu==5) {
				c1 = -P866;
				c2 = -P939;
				c3 = -P342;
				c4 = P766;
				c5 = P642;
			} else if (mu==7) {
				c1 = P866;
				c2 = P173;
				c3 = -P984;
				c4 = -P939;
				c5 = -P342;
			} else {
				c1 = -P866;
				c2 = P766;
				c3 = -P642;
				c4 = P173;
				c5 = -P984;
			}
			c6 = c1*c2;
			c7 = c1*c3;
			c8 = c1*c4;
			c9 = c1*c5;
			for (l=0; l<m; l++) {
				t1r = z[j3]+z[j6];
				t1i = z[j3+1]+z[j6+1];
				t2r = z[j0]-0.5*t1r;
				t2i = z[j0+1]-0.5*t1i;
				t3r = c1*(z[j3]-z[j6]);
				t3i = c1*(z[j3+1]-z[j6+1]);
				t4r = z[j0]+t1r;
				t4i = z[j0+1]+t1i;
				t5r = z[j4]+z[j7];
				t5i = z[j4+1]+z[j7+1];
				t6r = z[j1]-0.5*t5r;
				t6i = z[j1+1]-0.5*t5i;
				t7r = z[j4]-z[j7];
				t7i = z[j4+1]-z[j7+1];
				t8r = z[j1]+t5r;
				t8i = z[j1+1]+t5i;
				t9r = z[j2]+z[j5];
				t9i = z[j2+1]+z[j5+1];
				t10r = z[j8]-0.5*t9r;
				t10i = z[j8+1]-0.5*t9i;
				t11r = z[j2]-z[j5];
				t11i = z[j2+1]-z[j5+1];
				t12r = z[j8]+t9r;
				t12i = z[j8+1]+t9i;
				t13r = t8r+t12r;
				t13i = t8i+t12i;
				t14r = t6r+t10r;
				t14i = t6i+t10i;
				t15r = t6r-t10r;
				t15i = t6i-t10i;
				t16r = t7r+t11r;
				t16i = t7i+t11i;
				t17r = t7r-t11r;
				t17i = t7i-t11i;
				t18r = c2*t14r-c7*t17r;
				t18i = c2*t14i-c7*t17i;
				t19r = c4*t14r+c9*t17r;
				t19i = c4*t14i+c9*t17i;
				t20r = c3*t15r+c6*t16r;
				t20i = c3*t15i+c6*t16i;
				t21r = c5*t15r-c8*t16r;
				t21i = c5*t15i-c8*t16i;
				t22r = t18r+t19r;
				t22i = t18i+t19i;
				t23r = t20r-t21r;
				t23i = t20i-t21i;
				y1r = t2r+t18r;
				y1i = t2i+t18i;
				y2r = t2r+t19r;
				y2i = t2i+t19i;
				y3r = t4r-0.5*t13r;
				y3i = t4i-0.5*t13i;
				y4r = t2r-t22r;
				y4i = t2i-t22i;
				y5r = t3r-t23r;
				y5i = t3i-t23i;
				y6r = c1*(t8r-t12r);
				y6i = c1*(t8i-t12i);
				y7r = t21r-t3r;
				y7i = t21i-t3i;
				y8r = t3r+t20r;
				y8i = t3i+t20i;
				z[j0] = t4r+t13r;
				z[j0+1] = t4i+t13i;
				z[j1] = y1r-y8i;
				z[j1+1] = y1i+y8r;
				z[j2] = y2r-y7i;
				z[j2+1] = y2i+y7r;
				z[j3] = y3r-y6i;
				z[j3+1] = y3i+y6r;
				z[j4] = y4r-y5i;
				z[j4+1] = y4i+y5r;
				z[j5] = y4r+y5i;
				z[j5+1] = y4i-y5r;
				z[j6] = y3r+y6i;
				z[j6+1] = y3i-y6r;
				z[j7] = y2r+y7i;
				z[j7+1] = y2i-y7r;
				z[j8] = y1r+y8i;
				z[j8+1] = y1i-y8r;
				jt = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j9 = j8+jinc;
		if (j9>=jmax) j9 = j9-jmax;
		j10 = j9+jinc;
		if (j10>=jmax) j10 = j10-jmax;

		/* if factor is 11 */
		if (ifac==11) {
			if (mu==1) {
				c1 = P841;
				c2 = P415;
				c3 = -P142;
				c4 = -P654;
				c5 = -P959;
				c6 = P540;
				c7 = P909;
				c8 = P989;
				c9 = P755;
				c10 = P281;
			} else if (mu==2) {
				c1 = P415;
				c2 = -P654;
				c3 = -P959;
				c4 = -P142;
				c5 = P841;
				c6 = P909;
				c7 = P755;
				c8 = -P281;
				c9 = -P989;
				c10 = -P540;
			} else if (mu==3) {
				c1 = -P142;
				c2 = -P959;
				c3 = P415;
				c4 = P841;
				c5 = -P654;
				c6 = P989;
				c7 = -P281;
				c8 = -P909;
				c9 = P540;
				c10 = P755;
			} else if (mu==4) {
				c1 = -P654;
				c2 = -P142;
				c3 = P841;
				c4 = -P959;
				c5 = P415;
				c6 = P755;
				c7 = -P989;
				c8 = P540;
				c9 = P281;
				c10 = -P909;
			} else if (mu==5) {
				c1 = -P959;
				c2 = P841;
				c3 = -P654;
				c4 = P415;
				c5 = -P142;
				c6 = P281;
				c7 = -P540;
				c8 = P755;
				c9 = -P909;
				c10 = P989;
			} else if (mu==6) {
				c1 = -P959;
				c2 = P841;
				c3 = -P654;
				c4 = P415;
				c5 = -P142;
				c6 = -P281;
				c7 = P540;
				c8 = -P755;
				c9 = P909;
				c10 = -P989;
			} else if (mu==7) {
				c1 = -P654;
				c2 = -P142;
				c3 = P841;
				c4 = -P959;
				c5 = P415;
				c6 = -P755;
				c7 = P989;
				c8 = -P540;
				c9 = -P281;
				c10 = P909;
			} else if (mu==8) {
				c1 = -P142;
				c2 = -P959;
				c3 = P415;
				c4 = P841;
				c5 = -P654;
				c6 = -P989;
				c7 = P281;
				c8 = P909;
				c9 = -P540;
				c10 = -P755;
			} else if (mu==9) {
				c1 = P415;
				c2 = -P654;
				c3 = -P959;
				c4 = -P142;
				c5 = P841;
				c6 = -P909;
				c7 = -P755;
				c8 = P281;
				c9 = P989;
				c10 = P540;
			} else {
				c1 = P841;
				c2 = P415;
				c3 = -P142;
				c4 = -P654;
				c5 = -P959;
				c6 = -P540;
				c7 = -P909;
				c8 = -P989;
				c9 = -P755;
				c10 = -P281;
			}
			for (l=0; l<m; l++) {
				t1r = z[j1]+z[j10];
				t1i = z[j1+1]+z[j10+1];
				t2r = z[j2]+z[j9];
				t2i = z[j2+1]+z[j9+1];
				t3r = z[j3]+z[j8];
				t3i = z[j3+1]+z[j8+1];
				t4r = z[j4]+z[j7];
				t4i = z[j4+1]+z[j7+1];
				t5r = z[j5]+z[j6];
				t5i = z[j5+1]+z[j6+1];
				t6r = z[j1]-z[j10];
				t6i = z[j1+1]-z[j10+1];
				t7r = z[j2]-z[j9];
				t7i = z[j2+1]-z[j9+1];
				t8r = z[j3]-z[j8];
				t8i = z[j3+1]-z[j8+1];
				t9r = z[j4]-z[j7];
				t9i = z[j4+1]-z[j7+1];
				t10r = z[j5]-z[j6];
				t10i = z[j5+1]-z[j6+1];
				t11r = z[j0]-0.5*t5r;
				t11i = z[j0+1]-0.5*t5i;
				t12r = t1r-t5r;
				t12i = t1i-t5i;
				t13r = t2r-t5r;
				t13i = t2i-t5i;
				t14r = t3r-t5r;
				t14i = t3i-t5i;
				t15r = t4r-t5r;
				t15i = t4i-t5i;
				y1r = t11r+c1*t12r+c2*t13r+c3*t14r+c4*t15r;
				y1i = t11i+c1*t12i+c2*t13i+c3*t14i+c4*t15i;
				y2r = t11r+c2*t12r+c4*t13r+c5*t14r+c3*t15r;
				y2i = t11i+c2*t12i+c4*t13i+c5*t14i+c3*t15i;
				y3r = t11r+c3*t12r+c5*t13r+c2*t14r+c1*t15r;
				y3i = t11i+c3*t12i+c5*t13i+c2*t14i+c1*t15i;
				y4r = t11r+c4*t12r+c3*t13r+c1*t14r+c5*t15r;
				y4i = t11i+c4*t12i+c3*t13i+c1*t14i+c5*t15i;
				y5r = t11r+c5*t12r+c1*t13r+c4*t14r+c2*t15r;
				y5i = t11i+c5*t12i+c1*t13i+c4*t14i+c2*t15i;
				y6r = c10*t6r-c6*t7r+c9*t8r-c7*t9r+c8*t10r;
				y6i = c10*t6i-c6*t7i+c9*t8i-c7*t9i+c8*t10i;
				y7r = c9*t6r-c8*t7r+c6*t8r+c10*t9r-c7*t10r;
				y7i = c9*t6i-c8*t7i+c6*t8i+c10*t9i-c7*t10i;
				y8r = c8*t6r-c10*t7r-c7*t8r+c6*t9r+c9*t10r;
				y8i = c8*t6i-c10*t7i-c7*t8i+c6*t9i+c9*t10i;
				y9r = c7*t6r+c9*t7r-c10*t8r-c8*t9r-c6*t10r;
				y9i = c7*t6i+c9*t7i-c10*t8i-c8*t9i-c6*t10i;
				y10r = c6*t6r+c7*t7r+c8*t8r+c9*t9r+c10*t10r;
				y10i = c6*t6i+c7*t7i+c8*t8i+c9*t9i+c10*t10i;
				z[j0] = z[j0]+t1r+t2r+t3r+t4r+t5r;
				z[j0+1] = z[j0+1]+t1i+t2i+t3i+t4i+t5i;
				z[j1] = y1r-y10i;
				z[j1+1] = y1i+y10r;
				z[j2] = y2r-y9i;
				z[j2+1] = y2i+y9r;
				z[j3] = y3r-y8i;
				z[j3+1] = y3i+y8r;
				z[j4] = y4r-y7i;
				z[j4+1] = y4i+y7r;
				z[j5] = y5r-y6i;
				z[j5+1] = y5i+y6r;
				z[j6] = y5r+y6i;
				z[j6+1] = y5i-y6r;
				z[j7] = y4r+y7i;
				z[j7+1] = y4i-y7r;
				z[j8] = y3r+y8i;
				z[j8+1] = y3i-y8r;
				z[j9] = y2r+y9i;
				z[j9+1] = y2i-y9r;
				z[j10] = y1r+y10i;
				z[j10+1] = y1i-y10r;
				jt = j10+2;
				j10 = j9+2;
				j9 = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j11 = j10+jinc;
		if (j11>=jmax) j11 = j11-jmax;
		j12 = j11+jinc;
		if (j12>=jmax) j12 = j12-jmax;

		/* if factor is 13 */
		if (ifac==13) {
			if (mu==1) {
				c1 = P885;
				c2 = P568;
				c3 = P120;
				c4 = -P354;
				c5 = -P748;
				c6 = -P970;
				c7 = P464;
				c8 = P822;
				c9 = P992;
				c10 = P935;
				c11 = P663;
				c12 = P239;
			} else if (mu==2) {
				c1 = P568;
				c2 = -P354;
				c3 = -P970;
				c4 = -P748;
				c5 = P120;
				c6 = P885;
				c7 = P822;
				c8 = P935;
				c9 = P239;
				c10 = -P663;
				c11 = -P992;
				c12 = -P464;
			} else if (mu==3) {
				c1 = P120;
				c2 = -P970;
				c3 = -P354;
				c4 = P885;
				c5 = P568;
				c6 = -P748;
				c7 = P992;
				c8 = P239;
				c9 = -P935;
				c10 = -P464;
				c11 = P822;
				c12 = P663;
			} else if (mu==4) {
				c1 = -P354;
				c2 = -P748;
				c3 = P885;
				c4 = P120;
				c5 = -P970;
				c6 = P568;
				c7 = P935;
				c8 = -P663;
				c9 = -P464;
				c10 = P992;
				c11 = -P239;
				c12 = -P822;
			} else if (mu==5) {
				c1 = -P748;
				c2 = P120;
				c3 = P568;
				c4 = -P970;
				c5 = P885;
				c6 = -P354;
				c7 = P663;
				c8 = -P992;
				c9 = P822;
				c10 = -P239;
				c11 = -P464;
				c12 = P935;
			} else if (mu==6) {
				c1 = -P970;
				c2 = P885;
				c3 = -P748;
				c4 = P568;
				c5 = -P354;
				c6 = P120;
				c7 = P239;
				c8 = -P464;
				c9 = P663;
				c10 = -P822;
				c11 = P935;
				c12 = -P992;
			} else if (mu==7) {
				c1 = -P970;
				c2 = P885;
				c3 = -P748;
				c4 = P568;
				c5 = -P354;
				c6 = P120;
				c7 = -P239;
				c8 = P464;
				c9 = -P663;
				c10 = P822;
				c11 = -P935;
				c12 = P992;
			} else if (mu==8) {
				c1 = -P748;
				c2 = P120;
				c3 = P568;
				c4 = -P970;
				c5 = P885;
				c6 = -P354;
				c7 = -P663;
				c8 = P992;
				c9 = -P822;
				c10 = P239;
				c11 = P464;
				c12 = -P935;
			} else if (mu==9) {
				c1 = -P354;
				c2 = -P748;
				c3 = P885;
				c4 = P120;
				c5 = -P970;
				c6 = P568;
				c7 = -P935;
				c8 = P663;
				c9 = P464;
				c10 = -P992;
				c11 = P239;
				c12 = P822;
			} else if (mu==10) {
				c1 = P120;
				c2 = -P970;
				c3 = -P354;
				c4 = P885;
				c5 = P568;
				c6 = -P748;
				c7 = -P992;
				c8 = -P239;
				c9 = P935;
				c10 = P464;
				c11 = -P822;
				c12 = -P663;
			} else if (mu==11) {
				c1 = P568;
				c2 = -P354;
				c3 = -P970;
				c4 = -P748;
				c5 = P120;
				c6 = P885;
				c7 = -P822;
				c8 = -P935;
				c9 = -P239;
				c10 = P663;
				c11 = P992;
				c12 = P464;
			} else {
				c1 = P885;
				c2 = P568;
				c3 = P120;
				c4 = -P354;
				c5 = -P748;
				c6 = -P970;
				c7 = -P464;
				c8 = -P822;
				c9 = -P992;
				c10 = -P935;
				c11 = -P663;
				c12 = -P239;
			}
			for (l=0; l<m; l++) {
				t1r = z[j1]+z[j12];
				t1i = z[j1+1]+z[j12+1];
				t2r = z[j2]+z[j11];
				t2i = z[j2+1]+z[j11+1];
				t3r = z[j3]+z[j10];
				t3i = z[j3+1]+z[j10+1];
				t4r = z[j4]+z[j9];
				t4i = z[j4+1]+z[j9+1];
				t5r = z[j5]+z[j8];
				t5i = z[j5+1]+z[j8+1];
				t6r = z[j6]+z[j7];
				t6i = z[j6+1]+z[j7+1];
				t7r = z[j1]-z[j12];
				t7i = z[j1+1]-z[j12+1];
				t8r = z[j2]-z[j11];
				t8i = z[j2+1]-z[j11+1];
				t9r = z[j3]-z[j10];
				t9i = z[j3+1]-z[j10+1];
				t10r = z[j4]-z[j9];
				t10i = z[j4+1]-z[j9+1];
				t11r = z[j5]-z[j8];
				t11i = z[j5+1]-z[j8+1];
				t12r = z[j6]-z[j7];
				t12i = z[j6+1]-z[j7+1];
				t13r = z[j0]-0.5*t6r;
				t13i = z[j0+1]-0.5*t6i;
				t14r = t1r-t6r;
				t14i = t1i-t6i;
				t15r = t2r-t6r;
				t15i = t2i-t6i;
				t16r = t3r-t6r;
				t16i = t3i-t6i;
				t17r = t4r-t6r;
				t17i = t4i-t6i;
				t18r = t5r-t6r;
				t18i = t5i-t6i;
				y1r = t13r+c1*t14r+c2*t15r+c3*t16r+c4*t17r+c5*t18r;
				y1i = t13i+c1*t14i+c2*t15i+c3*t16i+c4*t17i+c5*t18i;
				y2r = t13r+c2*t14r+c4*t15r+c6*t16r+c5*t17r+c3*t18r;
				y2i = t13i+c2*t14i+c4*t15i+c6*t16i+c5*t17i+c3*t18i;
				y3r = t13r+c3*t14r+c6*t15r+c4*t16r+c1*t17r+c2*t18r;
				y3i = t13i+c3*t14i+c6*t15i+c4*t16i+c1*t17i+c2*t18i;
				y4r = t13r+c4*t14r+c5*t15r+c1*t16r+c3*t17r+c6*t18r;
				y4i = t13i+c4*t14i+c5*t15i+c1*t16i+c3*t17i+c6*t18i;
				y5r = t13r+c5*t14r+c3*t15r+c2*t16r+c6*t17r+c1*t18r;
				y5i = t13i+c5*t14i+c3*t15i+c2*t16i+c6*t17i+c1*t18i;
				y6r = t13r+c6*t14r+c1*t15r+c5*t16r+c2*t17r+c4*t18r;
				y6i = t13i+c6*t14i+c1*t15i+c5*t16i+c2*t17i+c4*t18i;
				y7r = c12*t7r-c7*t8r+c11*t9r-c8*t10r+c10*t11r-c9*t12r;
				y7i = c12*t7i-c7*t8i+c11*t9i-c8*t10i+c10*t11i-c9*t12i;
				y8r = c11*t7r-c9*t8r+c8*t9r-c12*t10r-c7*t11r+c10*t12r;
				y8i = c11*t7i-c9*t8i+c8*t9i-c12*t10i-c7*t11i+c10*t12i;
				y9r = c10*t7r-c11*t8r-c7*t9r+c9*t10r-c12*t11r-c8*t12r;
				y9i = c10*t7i-c11*t8i-c7*t9i+c9*t10i-c12*t11i-c8*t12i;
				y10r = c9*t7r+c12*t8r-c10*t9r-c7*t10r+c8*t11r+c11*t12r;
				y10i = c9*t7i+c12*t8i-c10*t9i-c7*t10i+c8*t11i+c11*t12i;
				y11r = c8*t7r+c10*t8r+c12*t9r-c11*t10r-c9*t11r-c7*t12r;
				y11i = c8*t7i+c10*t8i+c12*t9i-c11*t10i-c9*t11i-c7*t12i;
				y12r = c7*t7r+c8*t8r+c9*t9r+c10*t10r+c11*t11r+c12*t12r;
				y12i = c7*t7i+c8*t8i+c9*t9i+c10*t10i+c11*t11i+c12*t12i;
				z[j0] = z[j0]+t1r+t2r+t3r+t4r+t5r+t6r;
				z[j0+1] = z[j0+1]+t1i+t2i+t3i+t4i+t5i+t6i;
				z[j1] = y1r-y12i;
				z[j1+1] = y1i+y12r;
				z[j2] = y2r-y11i;
				z[j2+1] = y2i+y11r;
				z[j3] = y3r-y10i;
				z[j3+1] = y3i+y10r;
				z[j4] = y4r-y9i;
				z[j4+1] = y4i+y9r;
				z[j5] = y5r-y8i;
				z[j5+1] = y5i+y8r;
				z[j6] = y6r-y7i;
				z[j6+1] = y6i+y7r;
				z[j7] = y6r+y7i;
				z[j7+1] = y6i-y7r;
				z[j8] = y5r+y8i;
				z[j8+1] = y5i-y8r;
				z[j9] = y4r+y9i;
				z[j9+1] = y4i-y9r;
				z[j10] = y3r+y10i;
				z[j10+1] = y3i-y10r;
				z[j11] = y2r+y11i;
				z[j11+1] = y2i-y11r;
				z[j12] = y1r+y12i;
				z[j12+1] = y1i-y12r;
				jt = j12+2;
				j12 = j11+2;
				j11 = j10+2;
				j10 = j9+2;
				j9 = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
		j13 = j12+jinc;
		if (j13>=jmax) j13 = j13-jmax;
		j14 = j13+jinc;
		if (j14>=jmax) j14 = j14-jmax;
		j15 = j14+jinc;
		if (j15>=jmax) j15 = j15-jmax;

		/* if factor is 16 */
		if (ifac==16) {
			if (mu==1) {
				c1 = 1.0;
				c2 = P923;
				c3 = P382;
				c4 = P707;
			} else if (mu==3) {
				c1 = -1.0;
				c2 = P382;
				c3 = P923;
				c4 = -P707;
			} else if (mu==5) {
				c1 = 1.0;
				c2 = -P382;
				c3 = P923;
				c4 = -P707;
			} else if (mu==7) {
				c1 = -1.0;
				c2 = -P923;
				c3 = P382;
				c4 = P707;
			} else if (mu==9) {
				c1 = 1.0;
				c2 = -P923;
				c3 = -P382;
				c4 = P707;
			} else if (mu==11) {
				c1 = -1.0;
				c2 = -P382;
				c3 = -P923;
				c4 = -P707;
			} else if (mu==13) {
				c1 = 1.0;
				c2 = P382;
				c3 = -P923;
				c4 = -P707;
			} else {
				c1 = -1.0;
				c2 = P923;
				c3 = -P382;
				c4 = P707;
			}
			c5 = c1*c4;
			c6 = c1*c3;
			c7 = c1*c2;
			for (l=0; l<m; l++) {
				t1r = z[j0]+z[j8];
				t1i = z[j0+1]+z[j8+1];
				t2r = z[j4]+z[j12];
				t2i = z[j4+1]+z[j12+1];
				t3r = z[j0]-z[j8];
				t3i = z[j0+1]-z[j8+1];
				t4r = c1*(z[j4]-z[j12]);
				t4i = c1*(z[j4+1]-z[j12+1]);
				t5r = t1r+t2r;
				t5i = t1i+t2i;
				t6r = t1r-t2r;
				t6i = t1i-t2i;
				t7r = z[j1]+z[j9];
				t7i = z[j1+1]+z[j9+1];
				t8r = z[j5]+z[j13];
				t8i = z[j5+1]+z[j13+1];
				t9r = z[j1]-z[j9];
				t9i = z[j1+1]-z[j9+1];
				t10r = z[j5]-z[j13];
				t10i = z[j5+1]-z[j13+1];
				t11r = t7r+t8r;
				t11i = t7i+t8i;
				t12r = t7r-t8r;
				t12i = t7i-t8i;
				t13r = z[j2]+z[j10];
				t13i = z[j2+1]+z[j10+1];
				t14r = z[j6]+z[j14];
				t14i = z[j6+1]+z[j14+1];
				t15r = z[j2]-z[j10];
				t15i = z[j2+1]-z[j10+1];
				t16r = z[j6]-z[j14];
				t16i = z[j6+1]-z[j14+1];
				t17r = t13r+t14r;
				t17i = t13i+t14i;
				t18r = c4*(t15r-t16r);
				t18i = c4*(t15i-t16i);
				t19r = c5*(t15r+t16r);
				t19i = c5*(t15i+t16i);
				t20r = c1*(t13r-t14r);
				t20i = c1*(t13i-t14i);
				t21r = z[j3]+z[j11];
				t21i = z[j3+1]+z[j11+1];
				t22r = z[j7]+z[j15];
				t22i = z[j7+1]+z[j15+1];
				t23r = z[j3]-z[j11];
				t23i = z[j3+1]-z[j11+1];
				t24r = z[j7]-z[j15];
				t24i = z[j7+1]-z[j15+1];
				t25r = t21r+t22r;
				t25i = t21i+t22i;
				t26r = t21r-t22r;
				t26i = t21i-t22i;
				t27r = t9r+t24r;
				t27i = t9i+t24i;
				t28r = t10r+t23r;
				t28i = t10i+t23i;
				t29r = t9r-t24r;
				t29i = t9i-t24i;
				t30r = t10r-t23r;
				t30i = t10i-t23i;
				t31r = t5r+t17r;
				t31i = t5i+t17i;
				t32r = t11r+t25r;
				t32i = t11i+t25i;
				t33r = t3r+t18r;
				t33i = t3i+t18i;
				t34r = c2*t29r-c6*t30r;
				t34i = c2*t29i-c6*t30i;
				t35r = t3r-t18r;
				t35i = t3i-t18i;
				t36r = c7*t27r-c3*t28r;
				t36i = c7*t27i-c3*t28i;
				t37r = t4r+t19r;
				t37i = t4i+t19i;
				t38r = c3*t27r+c7*t28r;
				t38i = c3*t27i+c7*t28i;
				t39r = t4r-t19r;
				t39i = t4i-t19i;
				t40r = c6*t29r+c2*t30r;
				t40i = c6*t29i+c2*t30i;
				t41r = c4*(t12r-t26r);
				t41i = c4*(t12i-t26i);
				t42r = c5*(t12r+t26r);
				t42i = c5*(t12i+t26i);
				y1r = t33r+t34r;
				y1i = t33i+t34i;
				y2r = t6r+t41r;
				y2i = t6i+t41i;
				y3r = t35r+t40r;
				y3i = t35i+t40i;
				y4r = t5r-t17r;
				y4i = t5i-t17i;
				y5r = t35r-t40r;
				y5i = t35i-t40i;
				y6r = t6r-t41r;
				y6i = t6i-t41i;
				y7r = t33r-t34r;
				y7i = t33i-t34i;
				y9r = t38r-t37r;
				y9i = t38i-t37i;
				y10r = t42r-t20r;
				y10i = t42i-t20i;
				y11r = t36r+t39r;
				y11i = t36i+t39i;
				y12r = c1*(t11r-t25r);
				y12i = c1*(t11i-t25i);
				y13r = t36r-t39r;
				y13i = t36i-t39i;
				y14r = t42r+t20r;
				y14i = t42i+t20i;
				y15r = t38r+t37r;
				y15i = t38i+t37i;
				z[j0] = t31r+t32r;
				z[j0+1] = t31i+t32i;
				z[j1] = y1r-y15i;
				z[j1+1] = y1i+y15r;
				z[j2] = y2r-y14i;
				z[j2+1] = y2i+y14r;
				z[j3] = y3r-y13i;
				z[j3+1] = y3i+y13r;
				z[j4] = y4r-y12i;
				z[j4+1] = y4i+y12r;
				z[j5] = y5r-y11i;
				z[j5+1] = y5i+y11r;
				z[j6] = y6r-y10i;
				z[j6+1] = y6i+y10r;
				z[j7] = y7r-y9i;
				z[j7+1] = y7i+y9r;
				z[j8] = t31r-t32r;
				z[j8+1] = t31i-t32i;
				z[j9] = y7r+y9i;
				z[j9+1] = y7i-y9r;
				z[j10] = y6r+y10i;
				z[j10+1] = y6i-y10r;
				z[j11] = y5r+y11i;
				z[j11+1] = y5i-y11r;
				z[j12] = y4r+y12i;
				z[j12+1] = y4i-y12r;
				z[j13] = y3r+y13i;
				z[j13+1] = y3i-y13r;
				z[j14] = y2r+y14i;
				z[j14+1] = y2i-y14r;
				z[j15] = y1r+y15i;
				z[j15+1] = y1i-y15r;
				jt = j15+2;
				j15 = j14+2;
				j14 = j13+2;
				j13 = j12+2;
				j12 = j11+2;
				j11 = j10+2;
				j10 = j9+2;
				j9 = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j1+2;
				j1 = j0+2;
				j0 = jt;
			}
			continue;
		}
	}
}

/* Fortran Interface */
void cwpfft_(void *fz, int *n, int *isign)
{ register int i; 
  register float  scale;
  float  *fp  = (float *)fz; 
  complex *cz = (complex *)fz;

  pfacc(*isign, *n, cz);

  if( *isign < 0 ) {
    scale = 1.0/(float)(*n); 
    for(i=0; i < 2*(*n); i++) 
    {
       *fp *= scale; fp++;
    }
  }
}
