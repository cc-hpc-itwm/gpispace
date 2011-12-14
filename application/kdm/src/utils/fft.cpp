/***************************************************************************
 *   Copyright (C) 2007 by Maxim Ilyasov   *
 *   ilyasov@itwm.fhg.de   *
 ***************************************************************************/

#include "fft.h"
#include <math.h>
#include <iostream>

using namespace std;

int getptz(int N)
{
	int c1,c2,c3,c4,c5,c6;
	c1 = 4;
	c2 = 2;
	c3 = c4 = c5 = c6 = 1;

	float dN = logf((float)N);
	float d2 = logf(2.0f);
	float d3 = logf(3.0f);
	float d5 = logf(5.0f);
	float d7 = logf(7.0f);
	float d11 = logf(11.0f);
	float d13 = logf(13.0f);
	float dcur = d2 *c1 + d3*c2 + d5*c3 + d7*c4 + d11*c5 + d13*c6;

	for(int p=0; p<=4; p++)
	{
		for(int q=0; q<=2; q++)
		{
			for(int r=0; r<=1; r++)
			{
				for(int s=0; s<=1; s++)
				{
					for(int t=0; t<=1; t++)
					{
						for(int u=0; u<=1; u++)
						{
							float val = p*d2+q*d3+r*d5+s*d7+t*d11+u*d13;
							if((dN<=val)&&(val<=dcur))
							{
								dcur = val;
								c1 = p;
								c2 = q;
								c3 = r;
								c4 = s;
								c5 = t;
								c6 = u;
							}
						}
					}
				}
			}
		}
	} 
	int dimFFT = 1;
	for(int p=0; p< c1; p++)
		dimFFT *= 2;

	for(int q=0; q< c2; q++)
		dimFFT *= 3;

	for(int r=0; r<c3; r++)
		dimFFT *= 5;

	for(int s=0; s< c4; s++)
		dimFFT *= 7;

	for(int t=0; t< c5; t++)
		dimFFT *= 11;

	for(int u=0; u< c6; u++)
		dimFFT *= 13;

	return dimFFT;
}

#ifdef __SSE__
void sse_fft(int isign, int n, __m128 cz[])
{
	static int kfax[] = { 16,13,11,9,8,7,5,4,3,2 };
	register __m128 *z = cz;
	register int j00,j01,j2,j3,j4,j5,j6,j7,j8,j9,j10,j11,j12,j13,j14,j15,jt;
	int nleft,jfax,ifac,jfac,jinc,jmax,ndiv,m,mm=0,mu=0,l;
	__m128 t1r,t1i,t2r,t2i,t3r,t3i,t4r,t4i,t5r,t5i,
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
	for (jfax=0; jfax<10; jfax++) {

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
        j00 = 0;
        j01 = j00+jinc;

		/* if factor is 2 */
        if (ifac==2) {
			for (l=0; l<m; l++) {
				t1r = _mm_sub_ps(z[j00],z[j01]);
				t1i = _mm_sub_ps(z[j00+1],z[j01+1]);
				z[j00] = _mm_add_ps( z[j00],z[j01]);
				z[j00+1] = _mm_add_ps(z[j00+1],z[j01+1]);
				z[j01] = t1r;
				z[j01+1] = t1i;
				jt = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
        j2 = j01+jinc;
        if (j2>=jmax) j2 = j2-jmax;

		/* if factor is 3 */
        if (ifac==3) {
			if (mu==1)
			{
				const float value = 0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
			}
			else
			{
    			        const float value = -0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
			}
			for (l=0; l<m; l++) {
                 		const float value = 0.5;
				const __m128 c05 = _mm_setr_ps(value,value,value,value);
				t1r= _mm_add_ps(z[j01],z[j2]);
				t1i= _mm_add_ps(z[j01+1],z[j2+1]);
				y1r= _mm_sub_ps(z[j00],_mm_mul_ps(c05,t1r));
				y1i=_mm_sub_ps(z[j00+1],_mm_mul_ps(c05,t1i));
				y2r= _mm_mul_ps(c1,_mm_sub_ps(z[j01],z[j2]));
				y2i= _mm_mul_ps(c1,_mm_sub_ps(z[j01+1],z[j2+1]));
				z[j00]= _mm_add_ps(z[j00],t1r);
				z[j00+1]= _mm_add_ps(z[j00+1],t1i);
				z[j01]= _mm_sub_ps(y1r,y2i);
				z[j01+1]=_mm_add_ps(y1i,y2r);
				z[j2]= _mm_add_ps(y1r,y2i);
				z[j2+1]= _mm_sub_ps(y1i,y2r);
				jt = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j3 = j2+jinc;
		if (j3>=jmax) j3 = j3-jmax;

		/* if factor is 4 */
		if (ifac==4) {
			if (mu==1)
			{
               		        const float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
			}
			else
			{
			        const float value = -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
			}
			for (l=0; l<m; l++) {
				t1r= _mm_add_ps(z[j00],z[j2]);
				t1i= _mm_add_ps(z[j00+1],z[j2+1]);
				t2r= _mm_add_ps(z[j01],z[j3]);
				t2i= _mm_add_ps(z[j01+1],z[j3+1]);
				y1r= _mm_sub_ps(z[j00],z[j2]);
				y1i= _mm_sub_ps(z[j00+1],z[j2+1]);
				y3r= _mm_mul_ps(c1, _mm_sub_ps(z[j01],z[j3]));
				y3i= _mm_mul_ps(c1, _mm_sub_ps(z[j01+1],z[j3+1]));
				z[j00]=_mm_add_ps(t1r,t2r);
				z[j00+1]= _mm_add_ps(t1i,t2i);
				z[j01]= _mm_sub_ps(y1r,y3i);
				z[j01+1]= _mm_add_ps(y1i,y3r);
				z[j2]= _mm_sub_ps(t1r,t2r);
				z[j2+1]= _mm_sub_ps(t1i,t2i);
				z[j3]= _mm_add_ps(y1r,y3i);
				z[j3+1]= _mm_sub_ps(y1i,y3r);
				jt = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j4 = j3+jinc;
		if (j4>=jmax) j4 = j4-jmax;

		/* if factor is 5 */
		if (ifac==5) {
			if (mu==1) {
				float value = 0.559016994;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.951056516;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.587785252;
				c3 = _mm_setr_ps(value,value,value,value);
			} else if (mu==2) {
				float value = -0.559016994;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.587785252;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.951056516;
				c3 = _mm_setr_ps(value,value,value,value);
			} else if (mu==3) {
				float value = -0.559016994;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.587785252;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.951056516;
				c3 = _mm_setr_ps(value,value,value,value);
			} else { 
			    	float value = 0.559016994;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.951056516;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.587785252;
				c3 = _mm_setr_ps(value,value,value,value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.25;
			        const __m128 c025 = _mm_setr_ps(value,value,value,value);
				t1r= _mm_add_ps(z[j01],z[j4]);
				t1i= _mm_add_ps(z[j01+1],z[j4+1]);
				t2r= _mm_add_ps(z[j2],z[j3]);
				t2i= _mm_add_ps(z[j2+1],z[j3+1]);
				t3r= _mm_sub_ps(z[j01],z[j4]);
				t3i= _mm_sub_ps(z[j01+1],z[j4+1]);
				t4r= _mm_sub_ps(z[j2],z[j3]);
				t4i= _mm_sub_ps(z[j2+1],z[j3+1]);
				t5r= _mm_add_ps(t1r,t2r);
				t5i= _mm_add_ps(t1i,t2i);
				t6r= _mm_mul_ps(c1,_mm_sub_ps(t1r,t2r));
				t6i= _mm_mul_ps(c1,_mm_sub_ps(t1i,t2i));
				t7r= _mm_sub_ps(z[j00], _mm_mul_ps(c025,t5r));
				t7i= _mm_sub_ps(z[j00+1], _mm_mul_ps(c025,t5i));
				y1r= _mm_add_ps(t7r,t6r);
				y1i= _mm_add_ps(t7i,t6i);
				y2r= _mm_sub_ps(t7r,t6r);
				y2i= _mm_sub_ps(t7i,t6i);
				y3r= _mm_sub_ps(_mm_mul_ps(c3,t3r),_mm_mul_ps(c2,t4r));
				y3i= _mm_sub_ps(_mm_mul_ps(c3,t3i),_mm_mul_ps(c2,t4i));
				y4r= _mm_add_ps(_mm_mul_ps(c2,t3r),_mm_mul_ps(c3,t4r));
				y4i= _mm_add_ps(_mm_mul_ps(c2,t3i),_mm_mul_ps(c3,t4i));
				z[j00]= _mm_add_ps(z[j00],t5r);
				z[j00+1]= _mm_add_ps(z[j00+1],t5i);
				z[j01]= _mm_sub_ps(y1r,y4i);
				z[j01+1]= _mm_add_ps(y1i,y4r);
				z[j2]= _mm_sub_ps(y2r,y3i);
				z[j2+1]= _mm_add_ps(y2i,y3r);
				z[j3]= _mm_add_ps(y2r,y3i);
				z[j3+1]= _mm_sub_ps(y2i,y3r);
				z[j4]= _mm_add_ps(y1r,y4i);
				z[j4+1]= _mm_sub_ps(y1i,y4r);
				jt = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 0.623489802;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.222520934;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.900968868;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.781831482;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.974927912;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.433883739;
				c6 = _mm_setr_ps(value,value,value,value);
			} else if (mu==2) {
				float value = -0.222520934;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.900968868;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.623489802;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.974927912;
				c4 =  _mm_setr_ps(value,value,value,value);
				value = -0.433883739;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.781831482;
				c6 = _mm_setr_ps(value,value,value,value);
			} else if (mu==3) {
				float value = -0.900968868;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.623489802;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.222520934;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.433883739;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.781831482;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.974927912;
				c6 = _mm_setr_ps(value,value,value,value);
			} else if (mu==4) {
				float value = -0.900968868;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.623489802;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.222520934;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.433883739;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.781831482;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.974927912;
				c6 = _mm_setr_ps(value,value,value,value);
			} else if (mu==5) {
				float value = -0.222520934;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.900968868;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.623489802;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.974927912;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.433883739;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.781831482;
				c6 = _mm_setr_ps(value,value,value,value);
			} else {
				float value = 0.623489802;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.222520934;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.900968868;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.781831482;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.974927912;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.433883739;
				c6 = _mm_setr_ps(value,value,value,value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const __m128 c05 = _mm_setr_ps(value,value,value,value);
				t1r= _mm_add_ps(z[j01],z[j6]);
				t1i= _mm_add_ps(z[j01+1],z[j6+1]);
				t2r= _mm_add_ps(z[j2],z[j5]);
				t2i= _mm_add_ps(z[j2+1],z[j5+1]);
				t3r= _mm_add_ps(z[j3],z[j4]);
				t3i= _mm_add_ps(z[j3+1],z[j4+1]);
				t4r= _mm_sub_ps(z[j01],z[j6]);
				t4i= _mm_sub_ps(z[j01+1],z[j6+1]);
				t5r= _mm_sub_ps(z[j2],z[j5]);
				t5i= _mm_sub_ps(z[j2+1],z[j5+1]);
				t6r= _mm_sub_ps(z[j3],z[j4]);
				t6i= _mm_sub_ps(z[j3+1],z[j4+1]);
				t7r= _mm_sub_ps(z[j00],_mm_mul_ps(c05,t3r));
				t7i= _mm_sub_ps(z[j00+1],_mm_mul_ps(c05,t3i));
				t8r= _mm_sub_ps(t1r,t3r);
				t8i= _mm_sub_ps(t1i,t3i);
				t9r= _mm_sub_ps(t2r,t3r);
				t9i= _mm_sub_ps(t2i,t3i);
				y1r= _mm_add_ps(_mm_add_ps(t7r,_mm_mul_ps(c1,t8r)),_mm_mul_ps(c2,t9r));
				y1i= _mm_add_ps(_mm_add_ps(t7i,_mm_mul_ps(c1,t8i)),_mm_mul_ps(c2,t9i));
				y2r= _mm_add_ps(_mm_add_ps(t7r,_mm_mul_ps(c2,t8r)),_mm_mul_ps(c3,t9r));
				y2i= _mm_add_ps(_mm_add_ps(t7i,_mm_mul_ps(c2,t8i)),_mm_mul_ps(c3,t9i));
				y3r= _mm_add_ps(_mm_add_ps(t7r,_mm_mul_ps(c3,t8r)),_mm_mul_ps(c1,t9r));
				y3i= _mm_add_ps(_mm_add_ps(t7i,_mm_mul_ps(c3,t8i)),_mm_mul_ps(c1,t9i));
				y4r = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c6,t4r),_mm_mul_ps(c4,t5r)),_mm_mul_ps(c5,t6r));
				y4i = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c6,t4i),_mm_mul_ps(c4,t5i)),_mm_mul_ps(c5,t6i));
				y5r = _mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c5,t4r),_mm_mul_ps(c6,t5r)),_mm_mul_ps(c4,t6r));
				y5i = _mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c5,t4i),_mm_mul_ps(c6,t5i)),_mm_mul_ps(c4,t6i));
				y6r= _mm_add_ps(_mm_add_ps(_mm_mul_ps(c4,t4r),_mm_mul_ps(c5,t5r)),_mm_mul_ps(c6,t6r));
				y6i= _mm_add_ps(_mm_add_ps(_mm_mul_ps(c4,t4i),_mm_mul_ps(c5,t5i)),_mm_mul_ps(c6,t6i));
				z[j00]= _mm_add_ps(z[j00],_mm_add_ps(t1r,_mm_add_ps(t2r,t3r)));
				z[j00+1]= _mm_add_ps(z[j00+1],_mm_add_ps(t1i,_mm_add_ps(t2i,t3i)));
				z[j01]= _mm_sub_ps(y1r,y6i);
				z[j01+1]= _mm_add_ps(y1i,y6r);
				z[j2]= _mm_sub_ps(y2r,y5i);
				z[j2+1]= _mm_add_ps(y2i,y5r);
				z[j3]= _mm_sub_ps(y3r,y4i);
				z[j3+1]= _mm_add_ps(y3i,y4r);
				z[j4]= _mm_add_ps(y3r,y4i);
				z[j4+1]= _mm_sub_ps(y3i,y4r);
				z[j5]= _mm_add_ps(y2r,y5i);
				z[j5+1]= _mm_sub_ps(y2i,y5r);
				z[j6]= _mm_add_ps(y1r,y6i);
				z[j6+1]=  _mm_sub_ps(y1i,y6r);
				jt = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j7 = j6+jinc;
		if (j7>=jmax) j7 = j7-jmax;

		/* if factor is 8 */
		if (ifac==8) {
			if (mu==1) {
			        float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.707106781;
				c2 = _mm_setr_ps(value,value,value,value);
			} else if (mu==3) {
				float value = -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.707106781;
				c2 = _mm_setr_ps(value,value,value,value);
			} else if (mu==5) {
				float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.707106781;
				c2 =  _mm_setr_ps(value,value,value,value);
			} else {
				float value = -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.707106781;
				c2 = _mm_setr_ps(value,value,value,value);
			}
			c3= _mm_mul_ps(c1,c2);
			for (l=0; l<m; l++) {
				t1r= _mm_add_ps(z[j00],z[j4]);
				t1i= _mm_add_ps(z[j00+1],z[j4+1]);
				t2r= _mm_sub_ps(z[j00],z[j4]);
				t2i= _mm_sub_ps(z[j00+1],z[j4+1]);
				t3r= _mm_add_ps(z[j01],z[j5]);
				t3i= _mm_add_ps(z[j01+1],z[j5+1]);
				t4r= _mm_sub_ps(z[j01],z[j5]);
				t4i= _mm_sub_ps(z[j01+1],z[j5+1]);
				t5r= _mm_add_ps(z[j2],z[j6]);
				t5i= _mm_add_ps(z[j2+1],z[j6+1]);
				t6r= _mm_mul_ps(c1,_mm_sub_ps(z[j2],z[j6]));
				t6i= _mm_mul_ps(c1,_mm_sub_ps(z[j2+1],z[j6+1]));
				t7r= _mm_add_ps(z[j3],z[j7]);
				t7i= _mm_add_ps(z[j3+1],z[j7+1]);
				t8r= _mm_sub_ps(z[j3],z[j7]);
				t8i= _mm_sub_ps(z[j3+1],z[j7+1]);
				t9r= _mm_add_ps(t1r,t5r);
				t9i= _mm_add_ps(t1i,t5i);
				t10r= _mm_add_ps(t3r,t7r);
				t10i= _mm_add_ps(t3i,t7i);
				t11r= _mm_mul_ps(c2,_mm_sub_ps(t4r,t8r));
				t11i= _mm_mul_ps(c2,_mm_sub_ps(t4i,t8i));
				t12r= _mm_mul_ps(c3,_mm_add_ps(t4r,t8r));
				t12i= _mm_mul_ps(c3,_mm_add_ps(t4i,t8i));
				y1r= _mm_add_ps(t2r,t11r);
				y1i= _mm_add_ps(t2i,t11i);
				y2r= _mm_sub_ps(t1r,t5r);
				y2i= _mm_sub_ps(t1i,t5i);
				y3r= _mm_sub_ps(t2r,t11r);
				y3i= _mm_sub_ps(t2i,t11i);
				y5r= _mm_sub_ps(t12r,t6r);
				y5i= _mm_sub_ps(t12i,t6i);
				y6r= _mm_mul_ps(c1,_mm_sub_ps(t3r,t7r));
				y6i= _mm_mul_ps(c1,_mm_sub_ps(t3i,t7i));
				y7r= _mm_add_ps(t12r,t6r);
				y7i= _mm_add_ps(t12i,t6i);
				z[j00]= _mm_add_ps(t9r,t10r);
				z[j00+1]= _mm_add_ps(t9i,t10i);
				z[j01]= _mm_sub_ps(y1r,y7i);
				z[j01+1]= _mm_add_ps(y1i,y7r);
				z[j2]= _mm_sub_ps(y2r,y6i);
				z[j2+1]= _mm_add_ps(y2i,y6r);
				z[j3]= _mm_sub_ps(y3r,y5i);
				z[j3+1]= _mm_add_ps(y3i,y5r);
				z[j4]= _mm_sub_ps(t9r,t10r);
				z[j4+1]= _mm_sub_ps(t9i,t10i);
				z[j5]= _mm_add_ps(y3r,y5i);
				z[j5+1]= _mm_sub_ps(y3i,y5r);
				z[j6]= _mm_add_ps(y2r,y6i);
				z[j6+1]= _mm_sub_ps(y2i,y6r);
				z[j7]= _mm_add_ps(y1r,y7i);
				z[j7+1]= _mm_sub_ps(y1i,y7r);
				jt = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j8 = j7+jinc;
		if (j8>=jmax) j8 = j8-jmax;

		/* if factor is 9 */
		if (ifac==9) {
			if (mu==1) {
				float value = 0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.766044443;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.642787610;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.173648178;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.984807753;
				c5 = _mm_setr_ps(value,value,value,value);
			} else if (mu==2) {
				float value = -0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.173648178;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.984807753;
				c3 =  _mm_setr_ps(value,value,value,value);
				value = -0.939692621;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.342020143;
				c5 = _mm_setr_ps(value,value,value,value);
			} else if (mu==4) {
				float value = 0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.939692621;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.342020143;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.766044443;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.642787610;
				c5 = _mm_setr_ps(value,value,value,value);
			} else if (mu==5) {
				float value = -0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.939692621;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.342020143;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.766044443;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.642787610;
				c5 = _mm_setr_ps(value,value,value,value);
			} else if (mu==7) {
				float value = 0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.173648178;
				c2 =  _mm_setr_ps(value,value,value,value);
				value = -0.984807753;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.939692621;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.342020143;
				c5 = _mm_setr_ps(value,value,value,value);
			} else {
				float value = -0.866025404;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.766044443;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.642787610;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.173648178;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.984807753;
				c5 = _mm_setr_ps(value,value,value,value);
			}
			c6= _mm_mul_ps(c1,c2);
			c7= _mm_mul_ps(c1,c3);
		        c8= _mm_mul_ps(c1,c4);
			c9= _mm_mul_ps(c1,c5);
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const __m128 c05 = _mm_setr_ps(value,value,value,value);
				t1r= _mm_add_ps(z[j3],z[j6]);
				t1i= _mm_add_ps(z[j3+1],z[j6+1]);
				t2r=_mm_sub_ps(z[j00],_mm_mul_ps(c05,t1r));
				t2i= _mm_sub_ps(z[j00+1],_mm_mul_ps(c05,t1i));
				t3r= _mm_mul_ps(c1,_mm_sub_ps(z[j3],z[j6]));
				t3i =  _mm_mul_ps(c1,_mm_sub_ps(z[j3+1],z[j6+1]));
				t4r = _mm_add_ps(z[j00],t1r);
				t4i = _mm_add_ps(z[j00+1],t1i);
				t5r = _mm_add_ps(z[j4],z[j7]);
				t5i = _mm_add_ps(z[j4+1],z[j7+1]);
				t6r = _mm_sub_ps(z[j01],_mm_mul_ps(c05,t5r));
				t6i = _mm_sub_ps(z[j01+1],_mm_mul_ps(c05,t5i));
				t7r = _mm_sub_ps(z[j4],z[j7]);
				t7i = _mm_sub_ps(z[j4+1],z[j7+1]);
				t8r = _mm_add_ps(z[j01],t5r);
				t8i = _mm_add_ps(z[j01+1],t5i);
				t9r = _mm_add_ps(z[j2],z[j5]);
				t9i = _mm_add_ps(z[j2+1],z[j5+1]);
				t10r = _mm_sub_ps(z[j8],_mm_mul_ps(c05,t9r));
				t10i = _mm_sub_ps(z[j8+1],_mm_mul_ps(c05,t9i));
				t11r = _mm_sub_ps(z[j2],z[j5]);
				t11i = _mm_sub_ps(z[j2+1],z[j5+1]);
				t12r = _mm_add_ps(z[j8],t9r);
				t12i = _mm_add_ps(z[j8+1],t9i);
				t13r = _mm_add_ps(t8r,t12r);
				t13i = _mm_add_ps(t8i,t12i);
				t14r = _mm_add_ps(t6r,t10r);
				t14i = _mm_add_ps(t6i,t10i);
				t15r = _mm_sub_ps(t6r,t10r);
				t15i = _mm_sub_ps(t6i,t10i);
				t16r = _mm_add_ps(t7r,t11r);
				t16i = _mm_add_ps(t7i,t11i);
				t17r = _mm_sub_ps(t7r,t11r);
				t17i = _mm_sub_ps(t7i,t11i);
				t18r = _mm_sub_ps(_mm_mul_ps(c2,t14r),_mm_mul_ps(c7,t17r));
				t18i = _mm_sub_ps(_mm_mul_ps(c2,t14i),_mm_mul_ps(c7,t17i));
				t19r = _mm_add_ps(_mm_mul_ps(c4,t14r),_mm_mul_ps(c9,t17r));
				t19i = _mm_add_ps(_mm_mul_ps(c4,t14i),_mm_mul_ps(c9,t17i));
				t20r = _mm_add_ps(_mm_mul_ps(c3,t15r),_mm_mul_ps(c6,t16r));
				t20i = _mm_add_ps(_mm_mul_ps(c3,t15i),_mm_mul_ps(c6,t16i));
				t21r = _mm_sub_ps(_mm_mul_ps(c5,t15r),_mm_mul_ps(c8,t16r));
				t21i = _mm_sub_ps(_mm_mul_ps(c5,t15i),_mm_mul_ps(c8,t16i));
				t22r = _mm_add_ps(t18r,t19r);
				t22i = _mm_add_ps(t18i,t19i);
				t23r = _mm_sub_ps(t20r,t21r);
				t23i = _mm_sub_ps(t20i,t21i);
				y1r = _mm_add_ps(t2r,t18r);
				y1i = _mm_add_ps(t2i,t18i);
				y2r = _mm_add_ps(t2r,t19r);
				y2i = _mm_add_ps(t2i,t19i);
				y3r = _mm_sub_ps(t4r,_mm_mul_ps(c05,t13r));
				y3i = _mm_sub_ps(t4i,_mm_mul_ps(c05,t13i));
				y4r = _mm_sub_ps(t2r,t22r);
				y4i = _mm_sub_ps(t2i,t22i);
				y5r = _mm_sub_ps(t3r,t23r);
				y5i = _mm_sub_ps(t3i,t23i);
				y6r = _mm_mul_ps(c1,_mm_sub_ps(t8r,t12r));
				y6i = _mm_mul_ps(c1,_mm_sub_ps(t8i,t12i));
				y7r = _mm_sub_ps(t21r,t3r);
				y7i = _mm_sub_ps(t21i,t3i);
				y8r = _mm_add_ps(t3r,t20r);
				y8i = _mm_add_ps(t3i,t20i);
				z[j00] = _mm_add_ps(t4r,t13r);
				z[j00+1] = _mm_add_ps(t4i,t13i);
				z[j01] = _mm_sub_ps(y1r,y8i);
				z[j01+1] = _mm_add_ps(y1i,y8r);
				z[j2] = _mm_sub_ps(y2r,y7i);
				z[j2+1] = _mm_add_ps(y2i,y7r);
				z[j3] = _mm_sub_ps(y3r,y6i);
				z[j3+1] = _mm_add_ps(y3i,y6r);
				z[j4] = _mm_sub_ps(y4r,y5i);
				z[j4+1] = _mm_add_ps(y4i,y5r);
				z[j5] = _mm_add_ps(y4r,y5i);
				z[j5+1] = _mm_sub_ps(y4i,y5r);
				z[j6] = _mm_add_ps(y3r,y6i);
				z[j6+1] = _mm_sub_ps(y3i,y6r);
				z[j7] = _mm_add_ps(y2r,y7i);
				z[j7+1] = _mm_sub_ps(y2i,y7r);
				z[j8] = _mm_add_ps(y1r,y8i);
				z[j8+1] = _mm_sub_ps(y1i,y8r);
				jt = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 0.841253533;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.540640817;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.909631995;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.989821442;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.755749574;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.281732557;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==2) {
				float value = 0.415415013;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.909631995;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.755749574;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.281732557;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.989821442;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.540640817;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==3) {
				float value = -0.142314838;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.989821442;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.281732557;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.909631995;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.540640817;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.755749574;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==4) {
				float value = -0.654860734;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.755749574;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.989821442;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.540640817;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.281732557;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.909631995;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==5) {
				float value = -0.959492974;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.281732557;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.540640817;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.755749574;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.909631995;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.989821442;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==6) {
				float value = -0.959492974;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.281732557;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.540640817;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.755749574;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.909631995;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.989821442;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==7) {
				float value = -0.654860734;
				c1 =  _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.755749574;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.989821442;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.540640817;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.281732557;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.909631995;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==8) {
				float value = -0.142314838;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.989821442;
				c6 =  _mm_setr_ps(value,value,value,value);
				value = 0.281732557;
				c7 =  _mm_setr_ps(value,value,value,value);
				value = 0.909631995;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.540640817;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.755749574;
				c10 = _mm_setr_ps(value,value,value,value);
			} else if (mu==9) {
				float value = 0.415415013;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.841253533;
				c5 =  _mm_setr_ps(value,value,value,value);
				value = -0.909631995;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.755749574;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.281732557;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.989821442;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.540640817;
				c10 = _mm_setr_ps(value,value,value,value);
			} else {
				float value = 0.841253533;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.415415013;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.142314838;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.654860734;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.959492974;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.540640817;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.909631995;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.989821442;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.755749574;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.281732557;
				c10 = _mm_setr_ps(value,value,value,value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const __m128 c05 = _mm_setr_ps(value,value,value,value);
				t1r = _mm_add_ps(z[j01],z[j10]);
				t1i = _mm_add_ps(z[j01+1],z[j10+1]);
				t2r = _mm_add_ps(z[j2],z[j9]);
				t2i = _mm_add_ps(z[j2+1],z[j9+1]);
				t3r = _mm_add_ps(z[j3],z[j8]);
				t3i = _mm_add_ps(z[j3+1],z[j8+1]);
				t4r = _mm_add_ps(z[j4],z[j7]);
				t4i = _mm_add_ps(z[j4+1],z[j7+1]);
				t5r = _mm_add_ps(z[j5],z[j6]);
				t5i = _mm_add_ps(z[j5+1],z[j6+1]);
				t6r = _mm_sub_ps(z[j01],z[j10]);
				t6i = _mm_sub_ps(z[j01+1],z[j10+1]);
				t7r = _mm_sub_ps(z[j2],z[j9]);
				t7i = _mm_sub_ps(z[j2+1],z[j9+1]);
				t8r = _mm_sub_ps(z[j3],z[j8]);
				t8i = _mm_sub_ps(z[j3+1],z[j8+1]);
				t9r = _mm_sub_ps(z[j4],z[j7]);
				t9i = _mm_sub_ps(z[j4+1],z[j7+1]);
				t10r = _mm_sub_ps(z[j5],z[j6]);
				t10i = _mm_sub_ps(z[j5+1],z[j6+1]);
				t11r = _mm_sub_ps(z[j00],_mm_mul_ps(c05,t5r));
				t11i = _mm_sub_ps(z[j00+1],_mm_mul_ps(c05,t5i));
				t12r = _mm_sub_ps(t1r,t5r);
				t12i = _mm_sub_ps(t1i,t5i);
				t13r = _mm_sub_ps(t2r,t5r);
				t13i = _mm_sub_ps(t2i,t5i);
				t14r = _mm_sub_ps(t3r,t5r);
				t14i = _mm_sub_ps(t3i,t5i);
				t15r = _mm_sub_ps(t4r,t5r);
				t15i = _mm_sub_ps(t4i,t5i);

				y1r = _mm_add_ps(t11r,_mm_add_ps(_mm_mul_ps(c1,t12r),_mm_add_ps(_mm_mul_ps(c2,t13r),_mm_add_ps(_mm_mul_ps(c3,t14r),_mm_mul_ps(c4,t15r)))));
				y1i = _mm_add_ps(t11i,_mm_add_ps(_mm_mul_ps(c1,t12i),_mm_add_ps(_mm_mul_ps(c2,t13i),_mm_add_ps(_mm_mul_ps(c3,t14i),_mm_mul_ps(c4,t15i)))));
				y2r = _mm_add_ps(t11r,_mm_add_ps(_mm_mul_ps(c2,t12r),_mm_add_ps(_mm_mul_ps(c4,t13r),_mm_add_ps(_mm_mul_ps(c5,t14r),_mm_mul_ps(c3,t15r)))));
				y2i = _mm_add_ps(t11i,_mm_add_ps(_mm_mul_ps(c2,t12i),_mm_add_ps(_mm_mul_ps(c4,t13i),_mm_add_ps(_mm_mul_ps(c5,t14i),_mm_mul_ps(c3,t15i)))));
				y3r = _mm_add_ps(t11r,_mm_add_ps(_mm_mul_ps(c3,t12r),_mm_add_ps(_mm_mul_ps(c5,t13r),_mm_add_ps(_mm_mul_ps(c2,t14r),_mm_mul_ps(c1,t15r)))));
				y3i = _mm_add_ps(t11i,_mm_add_ps(_mm_mul_ps(c3,t12i),_mm_add_ps(_mm_mul_ps(c5,t13i),_mm_add_ps(_mm_mul_ps(c2,t14i),_mm_mul_ps(c1,t15i)))));
				y4r = _mm_add_ps(t11r,_mm_add_ps(_mm_mul_ps(c4,t12r),_mm_add_ps(_mm_mul_ps(c3,t13r),_mm_add_ps(_mm_mul_ps(c1,t14r),_mm_mul_ps(c5,t15r)))));
				y4i = _mm_add_ps(t11i,_mm_add_ps(_mm_mul_ps(c4,t12i),_mm_add_ps(_mm_mul_ps(c3,t13i),_mm_add_ps(_mm_mul_ps(c1,t14i),_mm_mul_ps(c5,t15i)))));
				y5r = _mm_add_ps(t11r,_mm_add_ps(_mm_mul_ps(c5,t12r),_mm_add_ps(_mm_mul_ps(c1,t13r),_mm_add_ps(_mm_mul_ps(c4,t14r),_mm_mul_ps(c2,t15r)))));
				y5i = _mm_add_ps(t11i,_mm_add_ps(_mm_mul_ps(c5,t12i),_mm_add_ps(_mm_mul_ps(c1,t13i),_mm_add_ps(_mm_mul_ps(c4,t14i),_mm_mul_ps(c2,t15i)))));

				y6r = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c10,t6r),_mm_mul_ps(c6,t7r)),_mm_add_ps(_mm_sub_ps(_mm_mul_ps(c9,t8r),_mm_mul_ps(c7,t9r)),_mm_mul_ps(c8,t10r)));
				y6i = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c10,t6i),_mm_mul_ps(c6,t7i)),_mm_add_ps(_mm_sub_ps(_mm_mul_ps(c9,t8i),_mm_mul_ps(c7,t9i)),_mm_mul_ps(c8,t10i)));
				y7r = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c9,t6r),_mm_mul_ps(c8,t7r)),_mm_add_ps(_mm_mul_ps(c6,t8r),_mm_sub_ps(_mm_mul_ps(c10,t9r),_mm_mul_ps(c7,t10r))));
				y7i = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c9,t6i),_mm_mul_ps(c8,t7i)),_mm_add_ps(_mm_mul_ps(c6,t8i),_mm_sub_ps(_mm_mul_ps(c10,t9i),_mm_mul_ps(c7,t10i))));
				y8r = _mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c8,t6r),_mm_mul_ps(c10,t7r)),_mm_mul_ps(c7,t8r)),_mm_add_ps(_mm_mul_ps(c6,t9r),_mm_mul_ps(c9,t10r)));
				y8i = _mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c8,t6i),_mm_mul_ps(c10,t7i)),_mm_mul_ps(c7,t8i)),_mm_add_ps(_mm_mul_ps(c6,t9i),_mm_mul_ps(c9,t10i)));
				y9r = _mm_add_ps(_mm_mul_ps(c7,t6r),_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c9,t7r),_mm_mul_ps(c10,t8r)),_mm_mul_ps(c8,t9r)),_mm_mul_ps(c6,t10r)));
				y9i = _mm_add_ps(_mm_mul_ps(c7,t6i),_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c9,t7i),_mm_mul_ps(c10,t8i)),_mm_mul_ps(c8,t9i)),_mm_mul_ps(c6,t10i)));
				y10r = _mm_add_ps(_mm_mul_ps(c6,t6r),_mm_add_ps(_mm_mul_ps(c7,t7r),_mm_add_ps(_mm_mul_ps(c8,t8r),_mm_add_ps(_mm_mul_ps(c9,t9r),_mm_mul_ps(c10,t10r)))));
				y10i = _mm_add_ps(_mm_mul_ps(c6,t6i),_mm_add_ps(_mm_mul_ps(c7,t7i),_mm_add_ps(_mm_mul_ps(c8,t8i),_mm_add_ps(_mm_mul_ps(c9,t9i),_mm_mul_ps(c10,t10i)))));

				z[j00] = _mm_add_ps(z[j00],_mm_add_ps(t1r,_mm_add_ps(t2r,_mm_add_ps(t3r,_mm_add_ps(t4r,t5r)))));
				z[j00+1] = _mm_add_ps(z[j00+1],_mm_add_ps(t1i,_mm_add_ps(t2i,_mm_add_ps(t3i,_mm_add_ps(t4i,t5i)))));
				z[j01] = _mm_sub_ps(y1r,y10i);
				z[j01+1] = _mm_add_ps(y1i,y10r);
				z[j2] = _mm_sub_ps(y2r,y9i);
				z[j2+1] = _mm_add_ps(y2i,y9r);
				z[j3] = _mm_sub_ps(y3r,y8i);
				z[j3+1] = _mm_add_ps(y3i,y8r);
				z[j4] = _mm_sub_ps(y4r,y7i);
				z[j4+1] = _mm_add_ps(y4i,y7r);
				z[j5] = _mm_sub_ps(y5r,y6i);
				z[j5+1] = _mm_add_ps(y5i,y6r);
				z[j6] = _mm_add_ps(y5r,y6i);
				z[j6+1] = _mm_sub_ps(y5i,y6r);
				z[j7] = _mm_add_ps(y4r,y7i);
				z[j7+1] = _mm_sub_ps(y4i,y7r);
				z[j8] = _mm_add_ps(y3r,y8i);
				z[j8+1] = _mm_sub_ps(y3i,y8r);
				z[j9] = _mm_add_ps(y2r,y9i);
				z[j9+1] = _mm_sub_ps(y2i,y9r);
				z[j10] = _mm_add_ps(y1r,y10i);
				z[j10+1] = _mm_sub_ps(y1i,y10r);
				jt = j10+2;
				j10 = j9+2;
				j9 = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 0.885456026;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.464723172;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.822983866;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.992708874;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.935016243;
				c10 = _mm_setr_ps(value,value,value,value);
				value = 0.663122658;
				c11 = _mm_setr_ps(value,value,value,value);
				value = 0.239315664;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==2) {
				float value = 0.568064747;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.822983866;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.935016243;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.239315664;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.663122658;
				c10 = _mm_setr_ps(value,value,value,value);
				value = -0.992708874;
				c11 = _mm_setr_ps(value,value,value,value);
				value = -0.464723172;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==3) {
				float value = 0.120536680;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.992708874;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.239315664;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.935016243;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.464723172;
				c10 = _mm_setr_ps(value,value,value,value);
				value = 0.822983866;
				c11 = _mm_setr_ps(value,value,value,value);
				value = 0.663122658;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==4) {
				float value = -0.354604887;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.935016243;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.663122658;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.464723172;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.992708874;
				c10 = _mm_setr_ps(value,value,value,value);
				value = -0.239315664;
				c11 = _mm_setr_ps(value,value,value,value);
				value = -0.822983866;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==5) {
				float value = -0.748510748;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.663122658;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.992708874;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.822983866;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.239315664;
				c10 = _mm_setr_ps(value,value,value,value);
				value = -0.464723172;
				c11 = _mm_setr_ps(value,value,value,value);
				value = 0.935016243;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==6) {
				float value = -0.970941817;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c6 = _mm_setr_ps(value,value,value,value);
				value = 0.239315664;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.464723172;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.663122658;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.822983866;
				c10 = _mm_setr_ps(value,value,value,value);
				value = 0.935016243;
				c11 = _mm_setr_ps(value,value,value,value);
				value = -0.992708874;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==7) {
				float value = -0.970941817;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.239315664;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.464723172;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.663122658;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.822983866;
				c10 = _mm_setr_ps(value,value,value,value);
				value = -0.935016243;
				c11 = _mm_setr_ps(value,value,value,value);
				value = 0.992708874;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==8) {
				float value = -0.748510748;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.663122658;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.992708874;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.822983866;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.239315664;
				c10 = _mm_setr_ps(value,value,value,value);
				value = 0.464723172;
				c11 = _mm_setr_ps(value,value,value,value);
				value = -0.935016243;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==9) {
				float value = -0.354604887;
				c1 = _mm_setr_ps(value,value,value,value);
   			        value = -0.748510748;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.935016243;
				c7 = _mm_setr_ps(value,value,value,value);
				value = 0.663122658;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.464723172;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.992708874;
				c10 = _mm_setr_ps(value,value,value,value);
				value = 0.239315664;
				c11 = _mm_setr_ps(value,value,value,value);
				value = 0.822983866;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==10) {
				float value = 0.120536680;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.992708874;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.239315664;
				c8 = _mm_setr_ps(value,value,value,value);
				value = 0.935016243;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.464723172;
				c10 = _mm_setr_ps(value,value,value,value);
				value = -0.822983866;
				c11 = _mm_setr_ps(value,value,value,value);
				value = -0.663122658;
				c12 = _mm_setr_ps(value,value,value,value);
			} else if (mu==11) {
				float value = 0.568064747;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c4 = _mm_setr_ps(value,value,value,value);
				value = 0.120536680;
				c5 = _mm_setr_ps(value,value,value,value);
				value = 0.885456026;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.822983866;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.935016243;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.239315664;
				c9 = _mm_setr_ps(value,value,value,value);
				value = 0.663122658;
				c10 = _mm_setr_ps(value,value,value,value);
				value = 0.992708874;
				c11 = _mm_setr_ps(value,value,value,value);
				value = 0.464723172;
				c12 = _mm_setr_ps(value,value,value,value);
			} else {
				float value = 0.885456026;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.568064747;
				c2 = _mm_setr_ps(value,value,value,value);
			        value = 0.120536680;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.354604887;
				c4 = _mm_setr_ps(value,value,value,value);
				value = -0.748510748;
				c5 = _mm_setr_ps(value,value,value,value);
				value = -0.970941817;
				c6 = _mm_setr_ps(value,value,value,value);
				value = -0.464723172;
				c7 = _mm_setr_ps(value,value,value,value);
				value = -0.822983866;
				c8 = _mm_setr_ps(value,value,value,value);
				value = -0.992708874;
				c9 = _mm_setr_ps(value,value,value,value);
				value = -0.935016243;
				c10 = _mm_setr_ps(value,value,value,value);
				value = -0.663122658;
				c11 = _mm_setr_ps(value,value,value,value);
				value = -0.239315664;
				c12 = _mm_setr_ps(value,value,value,value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const __m128 c05 = _mm_setr_ps(value,value,value,value);
				t1r = _mm_add_ps(z[j01],z[j12]);
				t1i = _mm_add_ps(z[j01+1],z[j12+1]);
				t2r = _mm_add_ps(z[j2],z[j11]);
				t2i = _mm_add_ps(z[j2+1],z[j11+1]);
				t3r = _mm_add_ps(z[j3],z[j10]);
				t3i = _mm_add_ps(z[j3+1],z[j10+1]);
				t4r = _mm_add_ps(z[j4],z[j9]);
				t4i = _mm_add_ps(z[j4+1],z[j9+1]);
				t5r = _mm_add_ps(z[j5],z[j8]);
				t5i = _mm_add_ps(z[j5+1],z[j8+1]);
				t6r = _mm_add_ps(z[j6],z[j7]);
				t6i = _mm_add_ps(z[j6+1],z[j7+1]);
				t7r = _mm_sub_ps(z[j01],z[j12]);
				t7i = _mm_sub_ps(z[j01+1],z[j12+1]);
				t8r = _mm_sub_ps(z[j2],z[j11]);
				t8i = _mm_sub_ps(z[j2+1],z[j11+1]);
				t9r = _mm_sub_ps(z[j3],z[j10]);
				t9i = _mm_sub_ps(z[j3+1],z[j10+1]);
				t10r = _mm_sub_ps(z[j4],z[j9]);
				t10i = _mm_sub_ps(z[j4+1],z[j9+1]);
				t11r = _mm_sub_ps(z[j5],z[j8]);
				t11i = _mm_sub_ps(z[j5+1],z[j8+1]);
				t12r = _mm_sub_ps(z[j6],z[j7]);
				t12i = _mm_sub_ps(z[j6+1],z[j7+1]);
				t13r = _mm_sub_ps(z[j00],_mm_mul_ps(c05,t6r));
				t13i = _mm_sub_ps(z[j00+1],_mm_mul_ps(c05,t6i));
				t14r = _mm_sub_ps(t1r,t6r);
				t14i = _mm_sub_ps(t1i,t6i);
				t15r = _mm_sub_ps(t2r,t6r);
				t15i = _mm_sub_ps(t2i,t6i);
				t16r = _mm_sub_ps(t3r,t6r);
				t16i = _mm_sub_ps(t3i,t6i);
				t17r = _mm_sub_ps(t4r,t6r);
				t17i = _mm_sub_ps(t4i,t6i);
				t18r = _mm_sub_ps(t5r,t6r);
				t18i = _mm_sub_ps(t5i,t6i);
				y1r = _mm_add_ps(t13r,_mm_add_ps(_mm_mul_ps(c1,t14r),_mm_add_ps(_mm_mul_ps(c2,t15r),_mm_add_ps(_mm_mul_ps(c3,t16r),_mm_add_ps(_mm_mul_ps(c4,t17r),_mm_mul_ps(c5,t18r))))));
				y1i = _mm_add_ps(t13i,_mm_add_ps(_mm_mul_ps(c1,t14i),_mm_add_ps(_mm_mul_ps(c2,t15i),_mm_add_ps(_mm_mul_ps(c3,t16i),_mm_add_ps(_mm_mul_ps(c4,t17i),_mm_mul_ps(c5,t18i))))));
				y2r = _mm_add_ps(t13r,_mm_add_ps(_mm_mul_ps(c2,t14r),_mm_add_ps(_mm_mul_ps(c4,t15r),_mm_add_ps(_mm_mul_ps(c6,t16r),_mm_add_ps(_mm_mul_ps(c5,t17r),_mm_mul_ps(c3,t18r))))));
				y2i = _mm_add_ps(t13i,_mm_add_ps(_mm_mul_ps(c2,t14i),_mm_add_ps(_mm_mul_ps(c4,t15i),_mm_add_ps(_mm_mul_ps(c6,t16i),_mm_add_ps(_mm_mul_ps(c5,t17i),_mm_mul_ps(c3,t18i))))));
				y3r = _mm_add_ps(t13r,_mm_add_ps(_mm_mul_ps(c3,t14r),_mm_add_ps(_mm_mul_ps(c6,t15r),_mm_add_ps(_mm_mul_ps(c4,t16r),_mm_add_ps(_mm_mul_ps(c1,t17r),_mm_mul_ps(c2,t18r))))));
				y3i = _mm_add_ps(t13i,_mm_add_ps(_mm_mul_ps(c3,t14i),_mm_add_ps(_mm_mul_ps(c6,t15i),_mm_add_ps(_mm_mul_ps(c4,t16i),_mm_add_ps(_mm_mul_ps(c1,t17i),_mm_mul_ps(c2,t18i))))));
				y4r = _mm_add_ps(t13r,_mm_add_ps(_mm_mul_ps(c4,t14r),_mm_add_ps(_mm_mul_ps(c5,t15r),_mm_add_ps(_mm_mul_ps(c1,t16r),_mm_add_ps(_mm_mul_ps(c3,t17r),_mm_mul_ps(c6,t18r))))));
				y4i = _mm_add_ps(t13i,_mm_add_ps(_mm_mul_ps(c4,t14i),_mm_add_ps(_mm_mul_ps(c5,t15i),_mm_add_ps(_mm_mul_ps(c1,t16i),_mm_add_ps(_mm_mul_ps(c3,t17i),_mm_mul_ps(c6,t18i))))));
				y5r = _mm_add_ps(t13r,_mm_add_ps(_mm_mul_ps(c5,t14r),_mm_add_ps(_mm_mul_ps(c3,t15r),_mm_add_ps(_mm_mul_ps(c2,t16r),_mm_add_ps(_mm_mul_ps(c6,t17r),_mm_mul_ps(c1,t18r))))));
				y5i = _mm_add_ps(t13i,_mm_add_ps(_mm_mul_ps(c5,t14i),_mm_add_ps(_mm_mul_ps(c3,t15i),_mm_add_ps(_mm_mul_ps(c2,t16i),_mm_add_ps(_mm_mul_ps(c6,t17i),_mm_mul_ps(c1,t18i))))));
				y6r = _mm_add_ps(t13r,_mm_add_ps(_mm_mul_ps(c6,t14r),_mm_add_ps(_mm_mul_ps(c1,t15r),_mm_add_ps(_mm_mul_ps(c5,t16r),_mm_add_ps(_mm_mul_ps(c2,t17r),_mm_mul_ps(c4,t18r))))));
				y6i = _mm_add_ps(t13i,_mm_add_ps(_mm_mul_ps(c6,t14i),_mm_add_ps(_mm_mul_ps(c1,t15i),_mm_add_ps(_mm_mul_ps(c5,t16i),_mm_add_ps(_mm_mul_ps(c2,t17i),_mm_mul_ps(c4,t18i))))));

				y7r = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c12,t7r),_mm_mul_ps(c7,t8r)),_mm_add_ps(_mm_sub_ps(_mm_mul_ps(c11,t9r),_mm_mul_ps(c8,t10r)),_mm_sub_ps(_mm_mul_ps(c10,t11r),_mm_mul_ps(c9,t12r))));
				y7i = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c12,t7i),_mm_mul_ps(c7,t8i)),_mm_add_ps(_mm_sub_ps(_mm_mul_ps(c11,t9i),_mm_mul_ps(c8,t10i)),_mm_sub_ps(_mm_mul_ps(c10,t11i),_mm_mul_ps(c9,t12i))));
				y8r = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c11,t7r),_mm_mul_ps(c9,t8r)),_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c8,t9r),_mm_mul_ps(c12,t10r)),_mm_mul_ps(c7,t11r)),_mm_mul_ps(c10,t12r)));
				y8i = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(c11,t7i),_mm_mul_ps(c9,t8i)),_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c8,t9i),_mm_mul_ps(c12,t10i)),_mm_mul_ps(c7,t11i)),_mm_mul_ps(c10,t12i)));
				y9r = _mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c10,t7r),_mm_mul_ps(c11,t8r)),_mm_mul_ps(c7,t9r)),_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c9,t10r),_mm_mul_ps(c12,t11r)),_mm_mul_ps(c8,t12r)));
				y9i = _mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c10,t7i),_mm_mul_ps(c11,t8i)),_mm_mul_ps(c7,t9i)),_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c9,t10i),_mm_mul_ps(c12,t11i)),_mm_mul_ps(c8,t12i)));
				y10r = _mm_add_ps(_mm_mul_ps(c9,t7r),_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c12,t8r),_mm_mul_ps(c10,t9r)),_mm_mul_ps(c7,t10r)),_mm_add_ps(_mm_mul_ps(c8,t11r),_mm_mul_ps(c11,t12r))));
				y10i = _mm_add_ps(_mm_mul_ps(c9,t7i),_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c12,t8i),_mm_mul_ps(c10,t9i)),_mm_mul_ps(c7,t10i)),_mm_add_ps(_mm_mul_ps(c8,t11i),_mm_mul_ps(c11,t12i))));
				y11r = _mm_add_ps(_mm_mul_ps(c8,t7r),_mm_add_ps(_mm_mul_ps(c10,t8r),_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c12,t9r),_mm_mul_ps(c11,t10r)),_mm_mul_ps(c9,t11r)),_mm_mul_ps(c7,t12r))));
				y11i = _mm_add_ps(_mm_mul_ps(c8,t7i),_mm_add_ps(_mm_mul_ps(c10,t8i),_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(c12,t9i),_mm_mul_ps(c11,t10i)),_mm_mul_ps(c9,t11i)),_mm_mul_ps(c7,t12i))));
				y12r = _mm_add_ps(_mm_mul_ps(c7,t7r),_mm_add_ps(_mm_mul_ps(c8,t8r),_mm_add_ps(_mm_mul_ps(c9,t9r),_mm_add_ps(_mm_mul_ps(c10,t10r),_mm_add_ps(_mm_mul_ps(c11,t11r),_mm_mul_ps(c12,t12r))))));
				y12i = _mm_add_ps(_mm_mul_ps(c7,t7i),_mm_add_ps(_mm_mul_ps(c8,t8i),_mm_add_ps(_mm_mul_ps(c9,t9i),_mm_add_ps(_mm_mul_ps(c10,t10i),_mm_add_ps(_mm_mul_ps(c11,t11i),_mm_mul_ps(c12,t12i))))));
				z[j00] = _mm_add_ps(z[j00],_mm_add_ps(t1r,_mm_add_ps(t2r,_mm_add_ps(t3r,_mm_add_ps(t4r,_mm_add_ps(t5r,t6r))))));
				z[j00+1] = _mm_add_ps(z[j00+1],_mm_add_ps(t1i,_mm_add_ps(t2i,_mm_add_ps(t3i,_mm_add_ps(t4i,_mm_add_ps(t5i,t6i))))));
				z[j01] = _mm_sub_ps(y1r,y12i);
				z[j01+1] = _mm_add_ps(y1i,y12r);
				z[j2] = _mm_sub_ps(y2r,y11i);
				z[j2+1] = _mm_add_ps(y2i,y11r);
				z[j3] = _mm_sub_ps(y3r,y10i);
				z[j3+1] = _mm_add_ps(y3i,y10r);
				z[j4] = _mm_sub_ps(y4r,y9i);
				z[j4+1] = _mm_add_ps(y4i,y9r);
				z[j5] = _mm_sub_ps(y5r,y8i);
				z[j5+1] = _mm_add_ps(y5i,y8r);
				z[j6] = _mm_sub_ps(y6r,y7i);
				z[j6+1] = _mm_add_ps(y6i,y7r);
				z[j7] = _mm_add_ps(y6r,y7i);
				z[j7+1] = _mm_sub_ps(y6i,y7r);
				z[j8] = _mm_add_ps(y5r,y8i);
				z[j8+1] = _mm_sub_ps(y5i,y8r);
				z[j9] = _mm_add_ps(y4r,y9i);
				z[j9+1] = _mm_sub_ps(y4i,y9r);
				z[j10] = _mm_add_ps(y3r,y10i);
				z[j10+1] = _mm_sub_ps(y3i,y10r);
				z[j11] = _mm_add_ps(y2r,y11i);
				z[j11+1] = _mm_sub_ps(y2i,y11r);
				z[j12] = _mm_add_ps(y1r,y12i);
				z[j12+1] = _mm_sub_ps(y1i,y12r);
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value= 0.923879533;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.382683432;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else if (mu==3) {
				float value= -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.382683432;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.923879533;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else if (mu==5) {
				float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.382683432;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.923879533;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else if (mu==7) {
				float value = -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.923879533;
				c2 = _mm_setr_ps(value,value,value,value);
				value = 0.382683432;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else if (mu==9) {
				float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.923879533;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.382683432;
				c3 = _mm_setr_ps(value,value,value,value);
				value= 0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else if (mu==11) {
				float value = -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = -0.382683432;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.923879533;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else if (mu==13) {
				float value = 1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.382683432;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.923879533;
				c3 = _mm_setr_ps(value,value,value,value);
				value = -0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			} else {
				float value = -1.0;
				c1 = _mm_setr_ps(value,value,value,value);
				value = 0.923879533;
				c2 = _mm_setr_ps(value,value,value,value);
				value = -0.382683432;
				c3 = _mm_setr_ps(value,value,value,value);
				value = 0.707106781;
				c4 = _mm_setr_ps(value,value,value,value);
			}
			c5 = _mm_mul_ps(c1,c4);
			c6 = _mm_mul_ps(c1,c3);
			c7 = _mm_mul_ps(c1,c2);
			for (l=0; l<m; l++) {
				t1r = _mm_add_ps(z[j00],z[j8]);
				t1i = _mm_add_ps(z[j00+1],z[j8+1]);
				t2r = _mm_add_ps(z[j4],z[j12]);
				t2i = _mm_add_ps(z[j4+1],z[j12+1]);
				t3r = _mm_sub_ps(z[j00],z[j8]);
				t3i = _mm_sub_ps(z[j00+1],z[j8+1]);
				t4r = _mm_mul_ps(c1,_mm_sub_ps(z[j4],z[j12]));
				t4i = _mm_mul_ps(c1,_mm_sub_ps(z[j4+1],z[j12+1]));
				t5r = _mm_add_ps(t1r,t2r);
				t5i = _mm_add_ps(t1i,t2i);
				t6r = _mm_sub_ps(t1r,t2r);
				t6i = _mm_sub_ps(t1i,t2i);
				t7r = _mm_add_ps(z[j01],z[j9]);
				t7i = _mm_add_ps(z[j01+1],z[j9+1]);
				t8r = _mm_add_ps(z[j5],z[j13]);
				t8i = _mm_add_ps(z[j5+1],z[j13+1]);
				t9r = _mm_sub_ps(z[j01],z[j9]);
				t9i = _mm_sub_ps(z[j01+1],z[j9+1]);
				t10r = _mm_sub_ps(z[j5],z[j13]);
				t10i = _mm_sub_ps(z[j5+1],z[j13+1]);
				t11r = _mm_add_ps(t7r,t8r);
				t11i = _mm_add_ps(t7i,t8i);
				t12r = _mm_sub_ps(t7r,t8r);
				t12i = _mm_sub_ps(t7i,t8i);
				t13r = _mm_add_ps(z[j2],z[j10]);
				t13i = _mm_add_ps(z[j2+1],z[j10+1]);
				t14r = _mm_add_ps(z[j6],z[j14]);
				t14i = _mm_add_ps(z[j6+1],z[j14+1]);
				t15r = _mm_sub_ps(z[j2],z[j10]);
				t15i = _mm_sub_ps(z[j2+1],z[j10+1]);
				t16r = _mm_sub_ps(z[j6],z[j14]);
				t16i = _mm_sub_ps(z[j6+1],z[j14+1]);
				t17r = _mm_add_ps(t13r,t14r);
				t17i = _mm_add_ps(t13i,t14i);
				t18r = _mm_mul_ps(c4,_mm_sub_ps(t15r,t16r));
				t18i = _mm_mul_ps(c4,_mm_sub_ps(t15i,t16i));
				t19r = _mm_mul_ps(c5,_mm_add_ps(t15r,t16r));
				t19i = _mm_mul_ps(c5,_mm_add_ps(t15i,t16i));
				t20r = _mm_mul_ps(c1,_mm_sub_ps(t13r,t14r));
				t20i = _mm_mul_ps(c1,_mm_sub_ps(t13i,t14i));
				t21r = _mm_add_ps(z[j3],z[j11]);
				t21i = _mm_add_ps(z[j3+1],z[j11+1]);
				t22r = _mm_add_ps(z[j7],z[j15]);
				t22i = _mm_add_ps(z[j7+1],z[j15+1]);
				t23r = _mm_sub_ps(z[j3],z[j11]);
				t23i = _mm_sub_ps(z[j3+1],z[j11+1]);
				t24r = _mm_sub_ps(z[j7],z[j15]);
				t24i = _mm_sub_ps(z[j7+1],z[j15+1]);
				t25r = _mm_add_ps(t21r,t22r);
				t25i = _mm_add_ps(t21i,t22i);
				t26r = _mm_sub_ps(t21r,t22r);
				t26i = _mm_sub_ps(t21i,t22i);
				t27r = _mm_add_ps(t9r,t24r);
				t27i = _mm_add_ps(t9i,t24i);
				t28r = _mm_add_ps(t10r,t23r);
				t28i =_mm_add_ps(t10i,t23i);
				t29r = _mm_sub_ps(t9r,t24r);
				t29i = _mm_sub_ps(t9i,t24i);
				t30r = _mm_sub_ps(t10r,t23r);
				t30i = _mm_sub_ps(t10i,t23i);
				t31r = _mm_add_ps(t5r,t17r);
				t31i = _mm_add_ps(t5i,t17i);
				t32r = _mm_add_ps(t11r,t25r);
				t32i = _mm_add_ps(t11i,t25i);
				t33r = _mm_add_ps(t3r,t18r);
				t33i = _mm_add_ps(t3i,t18i);
				t34r = _mm_sub_ps(_mm_mul_ps(c2,t29r),_mm_mul_ps(c6,t30r));
				t34i = _mm_sub_ps(_mm_mul_ps(c2,t29i),_mm_mul_ps(c6,t30i));
				t35r = _mm_sub_ps(t3r,t18r);
				t35i = _mm_sub_ps(t3i,t18i);
				t36r = _mm_sub_ps(_mm_mul_ps(c7,t27r),_mm_mul_ps(c3,t28r));
				t36i = _mm_sub_ps(_mm_mul_ps(c7,t27i),_mm_mul_ps(c3,t28i));
				t37r = _mm_add_ps(t4r,t19r);
				t37i = _mm_add_ps(t4i,t19i);
				t38r = _mm_add_ps(_mm_mul_ps(c3,t27r),_mm_mul_ps(c7,t28r));
				t38i = _mm_add_ps(_mm_mul_ps(c3,t27i),_mm_mul_ps(c7,t28i));
				t39r = _mm_sub_ps(t4r,t19r);
				t39i = _mm_sub_ps(t4i,t19i);
				t40r = _mm_add_ps(_mm_mul_ps(c6,t29r),_mm_mul_ps(c2,t30r));
				t40i = _mm_add_ps(_mm_mul_ps(c6,t29i),_mm_mul_ps(c2,t30i));
				t41r = _mm_mul_ps(c4,_mm_sub_ps(t12r,t26r));
				t41i = _mm_mul_ps(c4,_mm_sub_ps(t12i,t26i));
				t42r = _mm_mul_ps(c5,_mm_add_ps(t12r,t26r));
				t42i = _mm_mul_ps(c5,_mm_add_ps(t12i,t26i));
				y1r = _mm_add_ps(t33r,t34r);
				y1i = _mm_add_ps(t33i,t34i);
				y2r = _mm_add_ps(t6r,t41r);
				y2i = _mm_add_ps(t6i,t41i);
				y3r = _mm_add_ps(t35r,t40r);
				y3i = _mm_add_ps(t35i,t40i);
				y4r = _mm_sub_ps(t5r,t17r);
				y4i = _mm_sub_ps(t5i,t17i);
				y5r = _mm_sub_ps(t35r,t40r);
				y5i = _mm_sub_ps(t35i,t40i);
				y6r = _mm_sub_ps(t6r,t41r);
				y6i = _mm_sub_ps(t6i,t41i);
				y7r = _mm_sub_ps(t33r,t34r);
				y7i = _mm_sub_ps(t33i,t34i);
				y9r = _mm_sub_ps(t38r,t37r);
				y9i = _mm_sub_ps(t38i,t37i);
				y10r = _mm_sub_ps(t42r,t20r);
				y10i = _mm_sub_ps(t42i,t20i);
				y11r = _mm_add_ps(t36r,t39r);
				y11i = _mm_add_ps(t36i,t39i);
				y12r = _mm_mul_ps(c1,_mm_sub_ps(t11r,t25r));
				y12i = _mm_mul_ps(c1,_mm_sub_ps(t11i,t25i));
				y13r = _mm_sub_ps(t36r,t39r);
				y13i = _mm_sub_ps(t36i,t39i);
				y14r = _mm_add_ps(t42r,t20r);
				y14i = _mm_add_ps(t42i,t20i);
				y15r = _mm_add_ps(t38r,t37r);
				y15i = _mm_add_ps(t38i,t37i);
				z[j00] = _mm_add_ps(t31r,t32r);
				z[j00+1] = _mm_add_ps(t31i,t32i);
				z[j01] = _mm_sub_ps(y1r,y15i);
				z[j01+1] = _mm_add_ps(y1i,y15r);
				z[j2] = _mm_sub_ps(y2r,y14i);
				z[j2+1] = _mm_add_ps(y2i,y14r);
				z[j3] = _mm_sub_ps(y3r,y13i);
				z[j3+1] = _mm_add_ps(y3i,y13r);
				z[j4] = _mm_sub_ps(y4r,y12i);
				z[j4+1] = _mm_add_ps(y4i,y12r);
				z[j5] = _mm_sub_ps(y5r,y11i);
				z[j5+1] = _mm_add_ps(y5i,y11r);
				z[j6] = _mm_sub_ps(y6r,y10i);
				z[j6+1] = _mm_add_ps(y6i,y10r);
				z[j7] = _mm_sub_ps(y7r,y9i);
				z[j7+1] = _mm_add_ps(y7i,y9r);
				z[j8] = _mm_sub_ps(t31r,t32r);
				z[j8+1] = _mm_sub_ps(t31i,t32i);
				z[j9] = _mm_add_ps(y7r,y9i);
				z[j9+1] = _mm_sub_ps(y7i,y9r);
				z[j10] = _mm_add_ps(y6r,y10i);
				z[j10+1] = _mm_sub_ps(y6i,y10r);
				z[j11] = _mm_add_ps(y5r,y11i);
				z[j11+1] = _mm_sub_ps(y5i,y11r);
				z[j12] = _mm_add_ps(y4r,y12i);
				z[j12+1] = _mm_sub_ps(y4i,y12r);
				z[j13] = _mm_add_ps(y3r,y13i);
				z[j13+1] = _mm_sub_ps(y3i,y13r);
				z[j14] = _mm_add_ps(y2r,y14i);
				z[j14+1] = _mm_sub_ps(y2i,y14r);
				z[j15] = _mm_add_ps(y1r,y15i);
				z[j15+1] = _mm_sub_ps(y1i,y15r);
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
	}
}
#endif

#ifdef __ALTIVEC__
void sse_fft(int isign, int n, vector float cz[])
{
	static int kfax[] = { 16,13,11,9,8,7,5,4,3,2 };
	register vector float *z = cz;
	register int j00,j01,j2,j3,j4,j5,j6,j7,j8,j9,j10,j11,j12,j13,j14,j15,jt;
	int nleft,jfax,ifac,jfac,jinc,jmax,ndiv,m,mm=0,mu=0,l;
	vector float t1r,t1i,t2r,t2i,t3r,t3i,t4r,t4i,t5r,t5i,
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
	for (jfax=0; jfax<10; jfax++) {

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
        j00 = 0;
        j01 = j00+jinc;

		/* if factor is 2 */
        if (ifac==2) {
			for (l=0; l<m; l++) {
				t1r = vec_sub(z[j00],z[j01]);
				t1i = vec_sub(z[j00+1],z[j01+1]);
				z[j00] = vec_add( z[j00],z[j01]);
				z[j00+1] = vec_add(z[j00+1],z[j01+1]);
				z[j01] = t1r;
				z[j01+1] = t1i;
				jt = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
        j2 = j01+jinc;
        if (j2>=jmax) j2 = j2-jmax;

		/* if factor is 3 */
        if (ifac==3) {
			if (mu==1)
			{
				const float value = 0.866025404;
				c1 = spu_splats(value);
			}
			else
			{
    			        const float value = -0.866025404;
				c1 = spu_splats(value);
			}
			for (l=0; l<m; l++) {
                 		const float value = 0.5;
				const vector float c05 = spu_splats(value);
				t1r= vec_add(z[j01],z[j2]);
				t1i= vec_add(z[j01+1],z[j2+1]);
				y1r= vec_sub(z[j00],spu_mul(c05,t1r));
				y1i=vec_sub(z[j00+1],spu_mul(c05,t1i));
				y2r= spu_mul(c1,vec_sub(z[j01],z[j2]));
				y2i= spu_mul(c1,vec_sub(z[j01+1],z[j2+1]));
				z[j00]= vec_add(z[j00],t1r);
				z[j00+1]= vec_add(z[j00+1],t1i);
				z[j01]= vec_sub(y1r,y2i);
				z[j01+1]=vec_add(y1i,y2r);
				z[j2]= vec_add(y1r,y2i);
				z[j2+1]= vec_sub(y1i,y2r);
				jt = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j3 = j2+jinc;
		if (j3>=jmax) j3 = j3-jmax;

		/* if factor is 4 */
		if (ifac==4) {
			if (mu==1)
			{
               		        const float value = 1.0;
				c1 = spu_splats(value);
			}
			else
			{
			        const float value = -1.0;
				c1 = spu_splats(value);
			}
			for (l=0; l<m; l++) {
				t1r= vec_add(z[j00],z[j2]);
				t1i= vec_add(z[j00+1],z[j2+1]);
				t2r= vec_add(z[j01],z[j3]);
				t2i= vec_add(z[j01+1],z[j3+1]);
				y1r= vec_sub(z[j00],z[j2]);
				y1i= vec_sub(z[j00+1],z[j2+1]);
				y3r= spu_mul(c1, vec_sub(z[j01],z[j3]));
				y3i= spu_mul(c1, vec_sub(z[j01+1],z[j3+1]));
				z[j00]=vec_add(t1r,t2r);
				z[j00+1]= vec_add(t1i,t2i);
				z[j01]= vec_sub(y1r,y3i);
				z[j01+1]= vec_add(y1i,y3r);
				z[j2]= vec_sub(t1r,t2r);
				z[j2+1]= vec_sub(t1i,t2i);
				z[j3]= vec_add(y1r,y3i);
				z[j3+1]= vec_sub(y1i,y3r);
				jt = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j4 = j3+jinc;
		if (j4>=jmax) j4 = j4-jmax;

		/* if factor is 5 */
		if (ifac==5) {
			if (mu==1) {
				float value = 0.559016994;
				c1 = spu_splats(value);
				value = 0.951056516;
				c2 = spu_splats(value);
				value = 0.587785252;
				c3 = spu_splats(value);
			} else if (mu==2) {
				float value = -0.559016994;
				c1 = spu_splats(value);
				value = 0.587785252;
				c2 = spu_splats(value);
				value = -0.951056516;
				c3 = spu_splats(value);
			} else if (mu==3) {
				float value = -0.559016994;
				c1 = spu_splats(value);
				value = -0.587785252;
				c2 = spu_splats(value);
				value = 0.951056516;
				c3 = spu_splats(value);
			} else { 
			    	float value = 0.559016994;
				c1 = spu_splats(value);
				value = -0.951056516;
				c2 = spu_splats(value);
				value = -0.587785252;
				c3 = spu_splats(value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.25;
			        const vector float c025 = spu_splats(value);
				t1r= vec_add(z[j01],z[j4]);
				t1i= vec_add(z[j01+1],z[j4+1]);
				t2r= vec_add(z[j2],z[j3]);
				t2i= vec_add(z[j2+1],z[j3+1]);
				t3r= vec_sub(z[j01],z[j4]);
				t3i= vec_sub(z[j01+1],z[j4+1]);
				t4r= vec_sub(z[j2],z[j3]);
				t4i= vec_sub(z[j2+1],z[j3+1]);
				t5r= vec_add(t1r,t2r);
				t5i= vec_add(t1i,t2i);
				t6r= spu_mul(c1,vec_sub(t1r,t2r));
				t6i= spu_mul(c1,vec_sub(t1i,t2i));
				t7r= vec_sub(z[j00], spu_mul(c025,t5r));
				t7i= vec_sub(z[j00+1], spu_mul(c025,t5i));
				y1r= vec_add(t7r,t6r);
				y1i= vec_add(t7i,t6i);
				y2r= vec_sub(t7r,t6r);
				y2i= vec_sub(t7i,t6i);
				y3r= vec_sub(spu_mul(c3,t3r),spu_mul(c2,t4r));
				y3i= vec_sub(spu_mul(c3,t3i),spu_mul(c2,t4i));
				y4r= vec_add(spu_mul(c2,t3r),spu_mul(c3,t4r));
				y4i= vec_add(spu_mul(c2,t3i),spu_mul(c3,t4i));
				z[j00]= vec_add(z[j00],t5r);
				z[j00+1]= vec_add(z[j00+1],t5i);
				z[j01]= vec_sub(y1r,y4i);
				z[j01+1]= vec_add(y1i,y4r);
				z[j2]= vec_sub(y2r,y3i);
				z[j2+1]= vec_add(y2i,y3r);
				z[j3]= vec_add(y2r,y3i);
				z[j3+1]= vec_sub(y2i,y3r);
				z[j4]= vec_add(y1r,y4i);
				z[j4+1]= vec_sub(y1i,y4r);
				jt = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 0.623489802;
				c1 = spu_splats(value);
				value = -0.222520934;
				c2 = spu_splats(value);
				value = -0.900968868;
				c3 = spu_splats(value);
				value = 0.781831482;
				c4 = spu_splats(value);
				value = 0.974927912;
				c5 = spu_splats(value);
				value = 0.433883739;
				c6 = spu_splats(value);
			} else if (mu==2) {
				float value = -0.222520934;
				c1 = spu_splats(value);
				value = -0.900968868;
				c2 = spu_splats(value);
				value = 0.623489802;
				c3 = spu_splats(value);
				value = 0.974927912;
				c4 =  spu_splats(value);
				value = -0.433883739;
				c5 = spu_splats(value);
				value = -0.781831482;
				c6 = spu_splats(value);
			} else if (mu==3) {
				float value = -0.900968868;
				c1 = spu_splats(value);
				value = 0.623489802;
				c2 = spu_splats(value);
				value = -0.222520934;
				c3 = spu_splats(value);
				value = 0.433883739;
				c4 = spu_splats(value);
				value = -0.781831482;
				c5 = spu_splats(value);
				value = 0.974927912;
				c6 = spu_splats(value);
			} else if (mu==4) {
				float value = -0.900968868;
				c1 = spu_splats(value);
				value = 0.623489802;
				c2 = spu_splats(value);
				value = -0.222520934;
				c3 = spu_splats(value);
				value = -0.433883739;
				c4 = spu_splats(value);
				value = 0.781831482;
				c5 = spu_splats(value);
				value = -0.974927912;
				c6 = spu_splats(value);
			} else if (mu==5) {
				float value = -0.222520934;
				c1 = spu_splats(value);
				value = -0.900968868;
				c2 = spu_splats(value);
				value = 0.623489802;
				c3 = spu_splats(value);
				value = -0.974927912;
				c4 = spu_splats(value);
				value = 0.433883739;
				c5 = spu_splats(value);
				value = 0.781831482;
				c6 = spu_splats(value);
			} else {
				float value = 0.623489802;
				c1 = spu_splats(value);
				value = -0.222520934;
				c2 = spu_splats(value);
				value = -0.900968868;
				c3 = spu_splats(value);
				value = -0.781831482;
				c4 = spu_splats(value);
				value = -0.974927912;
				c5 = spu_splats(value);
				value = -0.433883739;
				c6 = spu_splats(value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const vector float c05 = spu_splats(value);
				t1r= vec_add(z[j01],z[j6]);
				t1i= vec_add(z[j01+1],z[j6+1]);
				t2r= vec_add(z[j2],z[j5]);
				t2i= vec_add(z[j2+1],z[j5+1]);
				t3r= vec_add(z[j3],z[j4]);
				t3i= vec_add(z[j3+1],z[j4+1]);
				t4r= vec_sub(z[j01],z[j6]);
				t4i= vec_sub(z[j01+1],z[j6+1]);
				t5r= vec_sub(z[j2],z[j5]);
				t5i= vec_sub(z[j2+1],z[j5+1]);
				t6r= vec_sub(z[j3],z[j4]);
				t6i= vec_sub(z[j3+1],z[j4+1]);
				t7r= vec_sub(z[j00],spu_mul(c05,t3r));
				t7i= vec_sub(z[j00+1],spu_mul(c05,t3i));
				t8r= vec_sub(t1r,t3r);
				t8i= vec_sub(t1i,t3i);
				t9r= vec_sub(t2r,t3r);
				t9i= vec_sub(t2i,t3i);
				y1r= vec_add(vec_add(t7r,spu_mul(c1,t8r)),spu_mul(c2,t9r));
				y1i= vec_add(vec_add(t7i,spu_mul(c1,t8i)),spu_mul(c2,t9i));
				y2r= vec_add(vec_add(t7r,spu_mul(c2,t8r)),spu_mul(c3,t9r));
				y2i= vec_add(vec_add(t7i,spu_mul(c2,t8i)),spu_mul(c3,t9i));
				y3r= vec_add(vec_add(t7r,spu_mul(c3,t8r)),spu_mul(c1,t9r));
				y3i= vec_add(vec_add(t7i,spu_mul(c3,t8i)),spu_mul(c1,t9i));
				y4r = vec_add(vec_sub(spu_mul(c6,t4r),spu_mul(c4,t5r)),spu_mul(c5,t6r));
				y4i = vec_add(vec_sub(spu_mul(c6,t4i),spu_mul(c4,t5i)),spu_mul(c5,t6i));
				y5r = vec_sub(vec_sub(spu_mul(c5,t4r),spu_mul(c6,t5r)),spu_mul(c4,t6r));
				y5i = vec_sub(vec_sub(spu_mul(c5,t4i),spu_mul(c6,t5i)),spu_mul(c4,t6i));
				y6r= vec_add(vec_add(spu_mul(c4,t4r),spu_mul(c5,t5r)),spu_mul(c6,t6r));
				y6i= vec_add(vec_add(spu_mul(c4,t4i),spu_mul(c5,t5i)),spu_mul(c6,t6i));
				z[j00]= vec_add(z[j00],vec_add(t1r,vec_add(t2r,t3r)));
				z[j00+1]= vec_add(z[j00+1],vec_add(t1i,vec_add(t2i,t3i)));
				z[j01]= vec_sub(y1r,y6i);
				z[j01+1]= vec_add(y1i,y6r);
				z[j2]= vec_sub(y2r,y5i);
				z[j2+1]= vec_add(y2i,y5r);
				z[j3]= vec_sub(y3r,y4i);
				z[j3+1]= vec_add(y3i,y4r);
				z[j4]= vec_add(y3r,y4i);
				z[j4+1]= vec_sub(y3i,y4r);
				z[j5]= vec_add(y2r,y5i);
				z[j5+1]= vec_sub(y2i,y5r);
				z[j6]= vec_add(y1r,y6i);
				z[j6+1]=  vec_sub(y1i,y6r);
				jt = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j7 = j6+jinc;
		if (j7>=jmax) j7 = j7-jmax;

		/* if factor is 8 */
		if (ifac==8) {
			if (mu==1) {
			        float value = 1.0;
				c1 = spu_splats(value);
				value = 0.707106781;
				c2 = spu_splats(value);
			} else if (mu==3) {
				float value = -1.0;
				c1 = spu_splats(value);
				value = -0.707106781;
				c2 = spu_splats(value);
			} else if (mu==5) {
				float value = 1.0;
				c1 = spu_splats(value);
				value = -0.707106781;
				c2 =  spu_splats(value);
			} else {
				float value = -1.0;
				c1 = spu_splats(value);
				value = 0.707106781;
				c2 = spu_splats(value);
			}
			c3= spu_mul(c1,c2);
			for (l=0; l<m; l++) {
				t1r= vec_add(z[j00],z[j4]);
				t1i= vec_add(z[j00+1],z[j4+1]);
				t2r= vec_sub(z[j00],z[j4]);
				t2i= vec_sub(z[j00+1],z[j4+1]);
				t3r= vec_add(z[j01],z[j5]);
				t3i= vec_add(z[j01+1],z[j5+1]);
				t4r= vec_sub(z[j01],z[j5]);
				t4i= vec_sub(z[j01+1],z[j5+1]);
				t5r= vec_add(z[j2],z[j6]);
				t5i= vec_add(z[j2+1],z[j6+1]);
				t6r= spu_mul(c1,vec_sub(z[j2],z[j6]));
				t6i= spu_mul(c1,vec_sub(z[j2+1],z[j6+1]));
				t7r= vec_add(z[j3],z[j7]);
				t7i= vec_add(z[j3+1],z[j7+1]);
				t8r= vec_sub(z[j3],z[j7]);
				t8i= vec_sub(z[j3+1],z[j7+1]);
				t9r= vec_add(t1r,t5r);
				t9i= vec_add(t1i,t5i);
				t10r= vec_add(t3r,t7r);
				t10i= vec_add(t3i,t7i);
				t11r= spu_mul(c2,vec_sub(t4r,t8r));
				t11i= spu_mul(c2,vec_sub(t4i,t8i));
				t12r= spu_mul(c3,vec_add(t4r,t8r));
				t12i= spu_mul(c3,vec_add(t4i,t8i));
				y1r= vec_add(t2r,t11r);
				y1i= vec_add(t2i,t11i);
				y2r= vec_sub(t1r,t5r);
				y2i= vec_sub(t1i,t5i);
				y3r= vec_sub(t2r,t11r);
				y3i= vec_sub(t2i,t11i);
				y5r= vec_sub(t12r,t6r);
				y5i= vec_sub(t12i,t6i);
				y6r= spu_mul(c1,vec_sub(t3r,t7r));
				y6i= spu_mul(c1,vec_sub(t3i,t7i));
				y7r= vec_add(t12r,t6r);
				y7i= vec_add(t12i,t6i);
				z[j00]= vec_add(t9r,t10r);
				z[j00+1]= vec_add(t9i,t10i);
				z[j01]= vec_sub(y1r,y7i);
				z[j01+1]= vec_add(y1i,y7r);
				z[j2]= vec_sub(y2r,y6i);
				z[j2+1]= vec_add(y2i,y6r);
				z[j3]= vec_sub(y3r,y5i);
				z[j3+1]= vec_add(y3i,y5r);
				z[j4]= vec_sub(t9r,t10r);
				z[j4+1]= vec_sub(t9i,t10i);
				z[j5]= vec_add(y3r,y5i);
				z[j5+1]= vec_sub(y3i,y5r);
				z[j6]= vec_add(y2r,y6i);
				z[j6+1]= vec_sub(y2i,y6r);
				z[j7]= vec_add(y1r,y7i);
				z[j7+1]= vec_sub(y1i,y7r);
				jt = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j8 = j7+jinc;
		if (j8>=jmax) j8 = j8-jmax;

		/* if factor is 9 */
		if (ifac==9) {
			if (mu==1) {
				float value = 0.866025404;
				c1 = spu_splats(value);
				value = 0.766044443;
				c2 = spu_splats(value);
				value = 0.642787610;
				c3 = spu_splats(value);
				value = 0.173648178;
				c4 = spu_splats(value);
				value = 0.984807753;
				c5 = spu_splats(value);
			} else if (mu==2) {
				float value = -0.866025404;
				c1 = spu_splats(value);
				value = 0.173648178;
				c2 = spu_splats(value);
				value = 0.984807753;
				c3 =  spu_splats(value);
				value = -0.939692621;
				c4 = spu_splats(value);
				value = 0.342020143;
				c5 = spu_splats(value);
			} else if (mu==4) {
				float value = 0.866025404;
				c1 = spu_splats(value);
				value = -0.939692621;
				c2 = spu_splats(value);
				value = 0.342020143;
				c3 = spu_splats(value);
				value = 0.766044443;
				c4 = spu_splats(value);
				value = -0.642787610;
				c5 = spu_splats(value);
			} else if (mu==5) {
				float value = -0.866025404;
				c1 = spu_splats(value);
				value = -0.939692621;
				c2 = spu_splats(value);
				value = -0.342020143;
				c3 = spu_splats(value);
				value = 0.766044443;
				c4 = spu_splats(value);
				value = 0.642787610;
				c5 = spu_splats(value);
			} else if (mu==7) {
				float value = 0.866025404;
				c1 = spu_splats(value);
				value = 0.173648178;
				c2 =  spu_splats(value);
				value = -0.984807753;
				c3 = spu_splats(value);
				value = -0.939692621;
				c4 = spu_splats(value);
				value = -0.342020143;
				c5 = spu_splats(value);
			} else {
				float value = -0.866025404;
				c1 = spu_splats(value);
				value = 0.766044443;
				c2 = spu_splats(value);
				value = -0.642787610;
				c3 = spu_splats(value);
				value = 0.173648178;
				c4 = spu_splats(value);
				value = -0.984807753;
				c5 = spu_splats(value);
			}
			c6= spu_mul(c1,c2);
			c7= spu_mul(c1,c3);
		        c8= spu_mul(c1,c4);
			c9= spu_mul(c1,c5);
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const vector float c05 = spu_splats(value);
				t1r= vec_add(z[j3],z[j6]);
				t1i= vec_add(z[j3+1],z[j6+1]);
				t2r=vec_sub(z[j00],spu_mul(c05,t1r));
				t2i= vec_sub(z[j00+1],spu_mul(c05,t1i));
				t3r= spu_mul(c1,vec_sub(z[j3],z[j6]));
				t3i =  spu_mul(c1,vec_sub(z[j3+1],z[j6+1]));
				t4r = vec_add(z[j00],t1r);
				t4i = vec_add(z[j00+1],t1i);
				t5r = vec_add(z[j4],z[j7]);
				t5i = vec_add(z[j4+1],z[j7+1]);
				t6r = vec_sub(z[j01],spu_mul(c05,t5r));
				t6i = vec_sub(z[j01+1],spu_mul(c05,t5i));
				t7r = vec_sub(z[j4],z[j7]);
				t7i = vec_sub(z[j4+1],z[j7+1]);
				t8r = vec_add(z[j01],t5r);
				t8i = vec_add(z[j01+1],t5i);
				t9r = vec_add(z[j2],z[j5]);
				t9i = vec_add(z[j2+1],z[j5+1]);
				t10r = vec_sub(z[j8],spu_mul(c05,t9r));
				t10i = vec_sub(z[j8+1],spu_mul(c05,t9i));
				t11r = vec_sub(z[j2],z[j5]);
				t11i = vec_sub(z[j2+1],z[j5+1]);
				t12r = vec_add(z[j8],t9r);
				t12i = vec_add(z[j8+1],t9i);
				t13r = vec_add(t8r,t12r);
				t13i = vec_add(t8i,t12i);
				t14r = vec_add(t6r,t10r);
				t14i = vec_add(t6i,t10i);
				t15r = vec_sub(t6r,t10r);
				t15i = vec_sub(t6i,t10i);
				t16r = vec_add(t7r,t11r);
				t16i = vec_add(t7i,t11i);
				t17r = vec_sub(t7r,t11r);
				t17i = vec_sub(t7i,t11i);
				t18r = vec_sub(spu_mul(c2,t14r),spu_mul(c7,t17r));
				t18i = vec_sub(spu_mul(c2,t14i),spu_mul(c7,t17i));
				t19r = vec_add(spu_mul(c4,t14r),spu_mul(c9,t17r));
				t19i = vec_add(spu_mul(c4,t14i),spu_mul(c9,t17i));
				t20r = vec_add(spu_mul(c3,t15r),spu_mul(c6,t16r));
				t20i = vec_add(spu_mul(c3,t15i),spu_mul(c6,t16i));
				t21r = vec_sub(spu_mul(c5,t15r),spu_mul(c8,t16r));
				t21i = vec_sub(spu_mul(c5,t15i),spu_mul(c8,t16i));
				t22r = vec_add(t18r,t19r);
				t22i = vec_add(t18i,t19i);
				t23r = vec_sub(t20r,t21r);
				t23i = vec_sub(t20i,t21i);
				y1r = vec_add(t2r,t18r);
				y1i = vec_add(t2i,t18i);
				y2r = vec_add(t2r,t19r);
				y2i = vec_add(t2i,t19i);
				y3r = vec_sub(t4r,spu_mul(c05,t13r));
				y3i = vec_sub(t4i,spu_mul(c05,t13i));
				y4r = vec_sub(t2r,t22r);
				y4i = vec_sub(t2i,t22i);
				y5r = vec_sub(t3r,t23r);
				y5i = vec_sub(t3i,t23i);
				y6r = spu_mul(c1,vec_sub(t8r,t12r));
				y6i = spu_mul(c1,vec_sub(t8i,t12i));
				y7r = vec_sub(t21r,t3r);
				y7i = vec_sub(t21i,t3i);
				y8r = vec_add(t3r,t20r);
				y8i = vec_add(t3i,t20i);
				z[j00] = vec_add(t4r,t13r);
				z[j00+1] = vec_add(t4i,t13i);
				z[j01] = vec_sub(y1r,y8i);
				z[j01+1] = vec_add(y1i,y8r);
				z[j2] = vec_sub(y2r,y7i);
				z[j2+1] = vec_add(y2i,y7r);
				z[j3] = vec_sub(y3r,y6i);
				z[j3+1] = vec_add(y3i,y6r);
				z[j4] = vec_sub(y4r,y5i);
				z[j4+1] = vec_add(y4i,y5r);
				z[j5] = vec_add(y4r,y5i);
				z[j5+1] = vec_sub(y4i,y5r);
				z[j6] = vec_add(y3r,y6i);
				z[j6+1] = vec_sub(y3i,y6r);
				z[j7] = vec_add(y2r,y7i);
				z[j7+1] = vec_sub(y2i,y7r);
				z[j8] = vec_add(y1r,y8i);
				z[j8+1] = vec_sub(y1i,y8r);
				jt = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 0.841253533;
				c1 = spu_splats(value);
				value = 0.415415013;
				c2 = spu_splats(value);
				value = -0.142314838;
				c3 = spu_splats(value);
				value = -0.654860734;
				c4 = spu_splats(value);
				value = -0.959492974;
				c5 = spu_splats(value);
				value = 0.540640817;
				c6 = spu_splats(value);
				value = 0.909631995;
				c7 = spu_splats(value);
				value = 0.989821442;
				c8 = spu_splats(value);
				value = 0.755749574;
				c9 = spu_splats(value);
				value = 0.281732557;
				c10 = spu_splats(value);
			} else if (mu==2) {
				float value = 0.415415013;
				c1 = spu_splats(value);
				value = -0.654860734;
				c2 = spu_splats(value);
				value = -0.959492974;
				c3 = spu_splats(value);
				value = -0.142314838;
				c4 = spu_splats(value);
				value = 0.841253533;
				c5 = spu_splats(value);
				value = 0.909631995;
				c6 = spu_splats(value);
				value = 0.755749574;
				c7 = spu_splats(value);
				value = -0.281732557;
				c8 = spu_splats(value);
				value = -0.989821442;
				c9 = spu_splats(value);
				value = -0.540640817;
				c10 = spu_splats(value);
			} else if (mu==3) {
				float value = -0.142314838;
				c1 = spu_splats(value);
				value = -0.959492974;
				c2 = spu_splats(value);
				value = 0.415415013;
				c3 = spu_splats(value);
				value = 0.841253533;
				c4 = spu_splats(value);
				value = -0.654860734;
				c5 = spu_splats(value);
				value = 0.989821442;
				c6 = spu_splats(value);
				value = -0.281732557;
				c7 = spu_splats(value);
				value = -0.909631995;
				c8 = spu_splats(value);
				value = 0.540640817;
				c9 = spu_splats(value);
				value = 0.755749574;
				c10 = spu_splats(value);
			} else if (mu==4) {
				float value = -0.654860734;
				c1 = spu_splats(value);
				value = -0.142314838;
				c2 = spu_splats(value);
				value = 0.841253533;
				c3 = spu_splats(value);
				value = -0.959492974;
				c4 = spu_splats(value);
				value = 0.415415013;
				c5 = spu_splats(value);
				value = 0.755749574;
				c6 = spu_splats(value);
				value = -0.989821442;
				c7 = spu_splats(value);
				value = 0.540640817;
				c8 = spu_splats(value);
				value = 0.281732557;
				c9 = spu_splats(value);
				value = -0.909631995;
				c10 = spu_splats(value);
			} else if (mu==5) {
				float value = -0.959492974;
				c1 = spu_splats(value);
				value = 0.841253533;
				c2 = spu_splats(value);
				value = -0.654860734;
				c3 = spu_splats(value);
				value = 0.415415013;
				c4 = spu_splats(value);
				value = -0.142314838;
				c5 = spu_splats(value);
				value = 0.281732557;
				c6 = spu_splats(value);
				value = -0.540640817;
				c7 = spu_splats(value);
				value = 0.755749574;
				c8 = spu_splats(value);
				value = -0.909631995;
				c9 = spu_splats(value);
				value = 0.989821442;
				c10 = spu_splats(value);
			} else if (mu==6) {
				float value = -0.959492974;
				c1 = spu_splats(value);
				value = 0.841253533;
				c2 = spu_splats(value);
				value = -0.654860734;
				c3 = spu_splats(value);
				value = 0.415415013;
				c4 = spu_splats(value);
				value = -0.142314838;
				c5 = spu_splats(value);
				value = -0.281732557;
				c6 = spu_splats(value);
				value = 0.540640817;
				c7 = spu_splats(value);
				value = -0.755749574;
				c8 = spu_splats(value);
				value = 0.909631995;
				c9 = spu_splats(value);
				value = -0.989821442;
				c10 = spu_splats(value);
			} else if (mu==7) {
				float value = -0.654860734;
				c1 =  spu_splats(value);
				value = -0.142314838;
				c2 = spu_splats(value);
				value = 0.841253533;
				c3 = spu_splats(value);
				value = -0.959492974;
				c4 = spu_splats(value);
				value = 0.415415013;
				c5 = spu_splats(value);
				value = -0.755749574;
				c6 = spu_splats(value);
				value = 0.989821442;
				c7 = spu_splats(value);
				value = -0.540640817;
				c8 = spu_splats(value);
				value = -0.281732557;
				c9 = spu_splats(value);
				value = 0.909631995;
				c10 = spu_splats(value);
			} else if (mu==8) {
				float value = -0.142314838;
				c1 = spu_splats(value);
				value = -0.959492974;
				c2 = spu_splats(value);
				value = 0.415415013;
				c3 = spu_splats(value);
				value = 0.841253533;
				c4 = spu_splats(value);
				value = -0.654860734;
				c5 = spu_splats(value);
				value = -0.989821442;
				c6 =  spu_splats(value);
				value = 0.281732557;
				c7 =  spu_splats(value);
				value = 0.909631995;
				c8 = spu_splats(value);
				value = -0.540640817;
				c9 = spu_splats(value);
				value = -0.755749574;
				c10 = spu_splats(value);
			} else if (mu==9) {
				float value = 0.415415013;
				c1 = spu_splats(value);
				value = -0.654860734;
				c2 = spu_splats(value);
				value = -0.959492974;
				c3 = spu_splats(value);
				value = -0.142314838;
				c4 = spu_splats(value);
				value = 0.841253533;
				c5 =  spu_splats(value);
				value = -0.909631995;
				c6 = spu_splats(value);
				value = -0.755749574;
				c7 = spu_splats(value);
				value = 0.281732557;
				c8 = spu_splats(value);
				value = 0.989821442;
				c9 = spu_splats(value);
				value = 0.540640817;
				c10 = spu_splats(value);
			} else {
				float value = 0.841253533;
				c1 = spu_splats(value);
				value = 0.415415013;
				c2 = spu_splats(value);
				value = -0.142314838;
				c3 = spu_splats(value);
				value = -0.654860734;
				c4 = spu_splats(value);
				value = -0.959492974;
				c5 = spu_splats(value);
				value = -0.540640817;
				c6 = spu_splats(value);
				value = -0.909631995;
				c7 = spu_splats(value);
				value = -0.989821442;
				c8 = spu_splats(value);
				value = -0.755749574;
				c9 = spu_splats(value);
				value = -0.281732557;
				c10 = spu_splats(value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const vector float c05 = spu_splats(value);
				t1r = vec_add(z[j01],z[j10]);
				t1i = vec_add(z[j01+1],z[j10+1]);
				t2r = vec_add(z[j2],z[j9]);
				t2i = vec_add(z[j2+1],z[j9+1]);
				t3r = vec_add(z[j3],z[j8]);
				t3i = vec_add(z[j3+1],z[j8+1]);
				t4r = vec_add(z[j4],z[j7]);
				t4i = vec_add(z[j4+1],z[j7+1]);
				t5r = vec_add(z[j5],z[j6]);
				t5i = vec_add(z[j5+1],z[j6+1]);
				t6r = vec_sub(z[j01],z[j10]);
				t6i = vec_sub(z[j01+1],z[j10+1]);
				t7r = vec_sub(z[j2],z[j9]);
				t7i = vec_sub(z[j2+1],z[j9+1]);
				t8r = vec_sub(z[j3],z[j8]);
				t8i = vec_sub(z[j3+1],z[j8+1]);
				t9r = vec_sub(z[j4],z[j7]);
				t9i = vec_sub(z[j4+1],z[j7+1]);
				t10r = vec_sub(z[j5],z[j6]);
				t10i = vec_sub(z[j5+1],z[j6+1]);
				t11r = vec_sub(z[j00],spu_mul(c05,t5r));
				t11i = vec_sub(z[j00+1],spu_mul(c05,t5i));
				t12r = vec_sub(t1r,t5r);
				t12i = vec_sub(t1i,t5i);
				t13r = vec_sub(t2r,t5r);
				t13i = vec_sub(t2i,t5i);
				t14r = vec_sub(t3r,t5r);
				t14i = vec_sub(t3i,t5i);
				t15r = vec_sub(t4r,t5r);
				t15i = vec_sub(t4i,t5i);

				y1r = vec_add(t11r,vec_add(spu_mul(c1,t12r),vec_add(spu_mul(c2,t13r),vec_add(spu_mul(c3,t14r),spu_mul(c4,t15r)))));
				y1i = vec_add(t11i,vec_add(spu_mul(c1,t12i),vec_add(spu_mul(c2,t13i),vec_add(spu_mul(c3,t14i),spu_mul(c4,t15i)))));
				y2r = vec_add(t11r,vec_add(spu_mul(c2,t12r),vec_add(spu_mul(c4,t13r),vec_add(spu_mul(c5,t14r),spu_mul(c3,t15r)))));
				y2i = vec_add(t11i,vec_add(spu_mul(c2,t12i),vec_add(spu_mul(c4,t13i),vec_add(spu_mul(c5,t14i),spu_mul(c3,t15i)))));
				y3r = vec_add(t11r,vec_add(spu_mul(c3,t12r),vec_add(spu_mul(c5,t13r),vec_add(spu_mul(c2,t14r),spu_mul(c1,t15r)))));
				y3i = vec_add(t11i,vec_add(spu_mul(c3,t12i),vec_add(spu_mul(c5,t13i),vec_add(spu_mul(c2,t14i),spu_mul(c1,t15i)))));
				y4r = vec_add(t11r,vec_add(spu_mul(c4,t12r),vec_add(spu_mul(c3,t13r),vec_add(spu_mul(c1,t14r),spu_mul(c5,t15r)))));
				y4i = vec_add(t11i,vec_add(spu_mul(c4,t12i),vec_add(spu_mul(c3,t13i),vec_add(spu_mul(c1,t14i),spu_mul(c5,t15i)))));
				y5r = vec_add(t11r,vec_add(spu_mul(c5,t12r),vec_add(spu_mul(c1,t13r),vec_add(spu_mul(c4,t14r),spu_mul(c2,t15r)))));
				y5i = vec_add(t11i,vec_add(spu_mul(c5,t12i),vec_add(spu_mul(c1,t13i),vec_add(spu_mul(c4,t14i),spu_mul(c2,t15i)))));

				y6r = vec_add(vec_sub(spu_mul(c10,t6r),spu_mul(c6,t7r)),vec_add(vec_sub(spu_mul(c9,t8r),spu_mul(c7,t9r)),spu_mul(c8,t10r)));
				y6i = vec_add(vec_sub(spu_mul(c10,t6i),spu_mul(c6,t7i)),vec_add(vec_sub(spu_mul(c9,t8i),spu_mul(c7,t9i)),spu_mul(c8,t10i)));
				y7r = vec_add(vec_sub(spu_mul(c9,t6r),spu_mul(c8,t7r)),vec_add(spu_mul(c6,t8r),vec_sub(spu_mul(c10,t9r),spu_mul(c7,t10r))));
				y7i = vec_add(vec_sub(spu_mul(c9,t6i),spu_mul(c8,t7i)),vec_add(spu_mul(c6,t8i),vec_sub(spu_mul(c10,t9i),spu_mul(c7,t10i))));
				y8r = vec_add(vec_sub(vec_sub(spu_mul(c8,t6r),spu_mul(c10,t7r)),spu_mul(c7,t8r)),vec_add(spu_mul(c6,t9r),spu_mul(c9,t10r)));
				y8i = vec_add(vec_sub(vec_sub(spu_mul(c8,t6i),spu_mul(c10,t7i)),spu_mul(c7,t8i)),vec_add(spu_mul(c6,t9i),spu_mul(c9,t10i)));
				y9r = vec_add(spu_mul(c7,t6r),vec_sub(vec_sub(vec_sub(spu_mul(c9,t7r),spu_mul(c10,t8r)),spu_mul(c8,t9r)),spu_mul(c6,t10r)));
				y9i = vec_add(spu_mul(c7,t6i),vec_sub(vec_sub(vec_sub(spu_mul(c9,t7i),spu_mul(c10,t8i)),spu_mul(c8,t9i)),spu_mul(c6,t10i)));
				y10r = vec_add(spu_mul(c6,t6r),vec_add(spu_mul(c7,t7r),vec_add(spu_mul(c8,t8r),vec_add(spu_mul(c9,t9r),spu_mul(c10,t10r)))));
				y10i = vec_add(spu_mul(c6,t6i),vec_add(spu_mul(c7,t7i),vec_add(spu_mul(c8,t8i),vec_add(spu_mul(c9,t9i),spu_mul(c10,t10i)))));

				z[j00] = vec_add(z[j00],vec_add(t1r,vec_add(t2r,vec_add(t3r,vec_add(t4r,t5r)))));
				z[j00+1] = vec_add(z[j00+1],vec_add(t1i,vec_add(t2i,vec_add(t3i,vec_add(t4i,t5i)))));
				z[j01] = vec_sub(y1r,y10i);
				z[j01+1] = vec_add(y1i,y10r);
				z[j2] = vec_sub(y2r,y9i);
				z[j2+1] = vec_add(y2i,y9r);
				z[j3] = vec_sub(y3r,y8i);
				z[j3+1] = vec_add(y3i,y8r);
				z[j4] = vec_sub(y4r,y7i);
				z[j4+1] = vec_add(y4i,y7r);
				z[j5] = vec_sub(y5r,y6i);
				z[j5+1] = vec_add(y5i,y6r);
				z[j6] = vec_add(y5r,y6i);
				z[j6+1] = vec_sub(y5i,y6r);
				z[j7] = vec_add(y4r,y7i);
				z[j7+1] = vec_sub(y4i,y7r);
				z[j8] = vec_add(y3r,y8i);
				z[j8+1] = vec_sub(y3i,y8r);
				z[j9] = vec_add(y2r,y9i);
				z[j9+1] = vec_sub(y2i,y9r);
				z[j10] = vec_add(y1r,y10i);
				z[j10+1] = vec_sub(y1i,y10r);
				jt = j10+2;
				j10 = j9+2;
				j9 = j8+2;
				j8 = j7+2;
				j7 = j6+2;
				j6 = j5+2;
				j5 = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 0.885456026;
				c1 = spu_splats(value);
				value = 0.568064747;
				c2 = spu_splats(value);
				value = 0.120536680;
				c3 = spu_splats(value);
				value = -0.354604887;
				c4 = spu_splats(value);
				value = -0.748510748;
				c5 = spu_splats(value);
				value = -0.970941817;
				c6 = spu_splats(value);
				value = 0.464723172;
				c7 = spu_splats(value);
				value = 0.822983866;
				c8 = spu_splats(value);
				value = 0.992708874;
				c9 = spu_splats(value);
				value = 0.935016243;
				c10 = spu_splats(value);
				value = 0.663122658;
				c11 = spu_splats(value);
				value = 0.239315664;
				c12 = spu_splats(value);
			} else if (mu==2) {
				float value = 0.568064747;
				c1 = spu_splats(value);
				value = -0.354604887;
				c2 = spu_splats(value);
				value = -0.970941817;
				c3 = spu_splats(value);
				value = -0.748510748;
				c4 = spu_splats(value);
				value = 0.120536680;
				c5 = spu_splats(value);
				value = 0.885456026;
				c6 = spu_splats(value);
				value = 0.822983866;
				c7 = spu_splats(value);
				value = 0.935016243;
				c8 = spu_splats(value);
				value = 0.239315664;
				c9 = spu_splats(value);
				value = -0.663122658;
				c10 = spu_splats(value);
				value = -0.992708874;
				c11 = spu_splats(value);
				value = -0.464723172;
				c12 = spu_splats(value);
			} else if (mu==3) {
				float value = 0.120536680;
				c1 = spu_splats(value);
				value = -0.970941817;
				c2 = spu_splats(value);
				value = -0.354604887;
				c3 = spu_splats(value);
				value = 0.885456026;
				c4 = spu_splats(value);
				value = 0.568064747;
				c5 = spu_splats(value);
				value = -0.748510748;
				c6 = spu_splats(value);
				value = 0.992708874;
				c7 = spu_splats(value);
				value = 0.239315664;
				c8 = spu_splats(value);
				value = -0.935016243;
				c9 = spu_splats(value);
				value = -0.464723172;
				c10 = spu_splats(value);
				value = 0.822983866;
				c11 = spu_splats(value);
				value = 0.663122658;
				c12 = spu_splats(value);
			} else if (mu==4) {
				float value = -0.354604887;
				c1 = spu_splats(value);
				value = -0.748510748;
				c2 = spu_splats(value);
				value = 0.885456026;
				c3 = spu_splats(value);
				value = 0.120536680;
				c4 = spu_splats(value);
				value = -0.970941817;
				c5 = spu_splats(value);
				value = 0.568064747;
				c6 = spu_splats(value);
				value = 0.935016243;
				c7 = spu_splats(value);
				value = -0.663122658;
				c8 = spu_splats(value);
				value = -0.464723172;
				c9 = spu_splats(value);
				value = 0.992708874;
				c10 = spu_splats(value);
				value = -0.239315664;
				c11 = spu_splats(value);
				value = -0.822983866;
				c12 = spu_splats(value);
			} else if (mu==5) {
				float value = -0.748510748;
				c1 = spu_splats(value);
				value = 0.120536680;
				c2 = spu_splats(value);
				value = 0.568064747;
				c3 = spu_splats(value);
				value = -0.970941817;
				c4 = spu_splats(value);
				value = 0.885456026;
				c5 = spu_splats(value);
				value = -0.354604887;
				c6 = spu_splats(value);
				value = 0.663122658;
				c7 = spu_splats(value);
				value = -0.992708874;
				c8 = spu_splats(value);
				value = 0.822983866;
				c9 = spu_splats(value);
				value = -0.239315664;
				c10 = spu_splats(value);
				value = -0.464723172;
				c11 = spu_splats(value);
				value = 0.935016243;
				c12 = spu_splats(value);
			} else if (mu==6) {
				float value = -0.970941817;
				c1 = spu_splats(value);
				value = 0.885456026;
				c2 = spu_splats(value);
				value = -0.748510748;
				c3 = spu_splats(value);
				value = 0.568064747;
				c4 = spu_splats(value);
				value = -0.354604887;
				c5 = spu_splats(value);
				value = 0.120536680;
				c6 = spu_splats(value);
				value = 0.239315664;
				c7 = spu_splats(value);
				value = -0.464723172;
				c8 = spu_splats(value);
				value = 0.663122658;
				c9 = spu_splats(value);
				value = -0.822983866;
				c10 = spu_splats(value);
				value = 0.935016243;
				c11 = spu_splats(value);
				value = -0.992708874;
				c12 = spu_splats(value);
			} else if (mu==7) {
				float value = -0.970941817;
				c1 = spu_splats(value);
				value = 0.885456026;
				c2 = spu_splats(value);
				value = -0.748510748;
				c3 = spu_splats(value);
				value = 0.568064747;
				c4 = spu_splats(value);
				value = -0.354604887;
				c5 = spu_splats(value);
				value = 0.120536680;
				c6 = spu_splats(value);
				value = -0.239315664;
				c7 = spu_splats(value);
				value = 0.464723172;
				c8 = spu_splats(value);
				value = -0.663122658;
				c9 = spu_splats(value);
				value = 0.822983866;
				c10 = spu_splats(value);
				value = -0.935016243;
				c11 = spu_splats(value);
				value = 0.992708874;
				c12 = spu_splats(value);
			} else if (mu==8) {
				float value = -0.748510748;
				c1 = spu_splats(value);
				value = 0.120536680;
				c2 = spu_splats(value);
				value = 0.568064747;
				c3 = spu_splats(value);
				value = -0.970941817;
				c4 = spu_splats(value);
				value = 0.885456026;
				c5 = spu_splats(value);
				value = -0.354604887;
				c6 = spu_splats(value);
				value = -0.663122658;
				c7 = spu_splats(value);
				value = 0.992708874;
				c8 = spu_splats(value);
				value = -0.822983866;
				c9 = spu_splats(value);
				value = 0.239315664;
				c10 = spu_splats(value);
				value = 0.464723172;
				c11 = spu_splats(value);
				value = -0.935016243;
				c12 = spu_splats(value);
			} else if (mu==9) {
				float value = -0.354604887;
				c1 = spu_splats(value);
   			        value = -0.748510748;
				c2 = spu_splats(value);
				value = 0.885456026;
				c3 = spu_splats(value);
				value = 0.120536680;
				c4 = spu_splats(value);
				value = -0.970941817;
				c5 = spu_splats(value);
				value = 0.568064747;
				c6 = spu_splats(value);
				value = -0.935016243;
				c7 = spu_splats(value);
				value = 0.663122658;
				c8 = spu_splats(value);
				value = 0.464723172;
				c9 = spu_splats(value);
				value = -0.992708874;
				c10 = spu_splats(value);
				value = 0.239315664;
				c11 = spu_splats(value);
				value = 0.822983866;
				c12 = spu_splats(value);
			} else if (mu==10) {
				float value = 0.120536680;
				c1 = spu_splats(value);
				value = -0.970941817;
				c2 = spu_splats(value);
				value = -0.354604887;
				c3 = spu_splats(value);
				value = 0.885456026;
				c4 = spu_splats(value);
				value = 0.568064747;
				c5 = spu_splats(value);
				value = -0.748510748;
				c6 = spu_splats(value);
				value = -0.992708874;
				c7 = spu_splats(value);
				value = -0.239315664;
				c8 = spu_splats(value);
				value = 0.935016243;
				c9 = spu_splats(value);
				value = 0.464723172;
				c10 = spu_splats(value);
				value = -0.822983866;
				c11 = spu_splats(value);
				value = -0.663122658;
				c12 = spu_splats(value);
			} else if (mu==11) {
				float value = 0.568064747;
				c1 = spu_splats(value);
				value = -0.354604887;
				c2 = spu_splats(value);
				value = -0.970941817;
				c3 = spu_splats(value);
				value = -0.748510748;
				c4 = spu_splats(value);
				value = 0.120536680;
				c5 = spu_splats(value);
				value = 0.885456026;
				c6 = spu_splats(value);
				value = -0.822983866;
				c7 = spu_splats(value);
				value = -0.935016243;
				c8 = spu_splats(value);
				value = -0.239315664;
				c9 = spu_splats(value);
				value = 0.663122658;
				c10 = spu_splats(value);
				value = 0.992708874;
				c11 = spu_splats(value);
				value = 0.464723172;
				c12 = spu_splats(value);
			} else {
				float value = 0.885456026;
				c1 = spu_splats(value);
				value = 0.568064747;
				c2 = spu_splats(value);
			        value = 0.120536680;
				c3 = spu_splats(value);
				value = -0.354604887;
				c4 = spu_splats(value);
				value = -0.748510748;
				c5 = spu_splats(value);
				value = -0.970941817;
				c6 = spu_splats(value);
				value = -0.464723172;
				c7 = spu_splats(value);
				value = -0.822983866;
				c8 = spu_splats(value);
				value = -0.992708874;
				c9 = spu_splats(value);
				value = -0.935016243;
				c10 = spu_splats(value);
				value = -0.663122658;
				c11 = spu_splats(value);
				value = -0.239315664;
				c12 = spu_splats(value);
			}
			for (l=0; l<m; l++) {
			        const float value = 0.5;
				const vector float c05 = spu_splats(value);
				t1r = vec_add(z[j01],z[j12]);
				t1i = vec_add(z[j01+1],z[j12+1]);
				t2r = vec_add(z[j2],z[j11]);
				t2i = vec_add(z[j2+1],z[j11+1]);
				t3r = vec_add(z[j3],z[j10]);
				t3i = vec_add(z[j3+1],z[j10+1]);
				t4r = vec_add(z[j4],z[j9]);
				t4i = vec_add(z[j4+1],z[j9+1]);
				t5r = vec_add(z[j5],z[j8]);
				t5i = vec_add(z[j5+1],z[j8+1]);
				t6r = vec_add(z[j6],z[j7]);
				t6i = vec_add(z[j6+1],z[j7+1]);
				t7r = vec_sub(z[j01],z[j12]);
				t7i = vec_sub(z[j01+1],z[j12+1]);
				t8r = vec_sub(z[j2],z[j11]);
				t8i = vec_sub(z[j2+1],z[j11+1]);
				t9r = vec_sub(z[j3],z[j10]);
				t9i = vec_sub(z[j3+1],z[j10+1]);
				t10r = vec_sub(z[j4],z[j9]);
				t10i = vec_sub(z[j4+1],z[j9+1]);
				t11r = vec_sub(z[j5],z[j8]);
				t11i = vec_sub(z[j5+1],z[j8+1]);
				t12r = vec_sub(z[j6],z[j7]);
				t12i = vec_sub(z[j6+1],z[j7+1]);
				t13r = vec_sub(z[j00],spu_mul(c05,t6r));
				t13i = vec_sub(z[j00+1],spu_mul(c05,t6i));
				t14r = vec_sub(t1r,t6r);
				t14i = vec_sub(t1i,t6i);
				t15r = vec_sub(t2r,t6r);
				t15i = vec_sub(t2i,t6i);
				t16r = vec_sub(t3r,t6r);
				t16i = vec_sub(t3i,t6i);
				t17r = vec_sub(t4r,t6r);
				t17i = vec_sub(t4i,t6i);
				t18r = vec_sub(t5r,t6r);
				t18i = vec_sub(t5i,t6i);
				y1r = vec_add(t13r,vec_add(spu_mul(c1,t14r),vec_add(spu_mul(c2,t15r),vec_add(spu_mul(c3,t16r),vec_add(spu_mul(c4,t17r),spu_mul(c5,t18r))))));
				y1i = vec_add(t13i,vec_add(spu_mul(c1,t14i),vec_add(spu_mul(c2,t15i),vec_add(spu_mul(c3,t16i),vec_add(spu_mul(c4,t17i),spu_mul(c5,t18i))))));
				y2r = vec_add(t13r,vec_add(spu_mul(c2,t14r),vec_add(spu_mul(c4,t15r),vec_add(spu_mul(c6,t16r),vec_add(spu_mul(c5,t17r),spu_mul(c3,t18r))))));
				y2i = vec_add(t13i,vec_add(spu_mul(c2,t14i),vec_add(spu_mul(c4,t15i),vec_add(spu_mul(c6,t16i),vec_add(spu_mul(c5,t17i),spu_mul(c3,t18i))))));
				y3r = vec_add(t13r,vec_add(spu_mul(c3,t14r),vec_add(spu_mul(c6,t15r),vec_add(spu_mul(c4,t16r),vec_add(spu_mul(c1,t17r),spu_mul(c2,t18r))))));
				y3i = vec_add(t13i,vec_add(spu_mul(c3,t14i),vec_add(spu_mul(c6,t15i),vec_add(spu_mul(c4,t16i),vec_add(spu_mul(c1,t17i),spu_mul(c2,t18i))))));
				y4r = vec_add(t13r,vec_add(spu_mul(c4,t14r),vec_add(spu_mul(c5,t15r),vec_add(spu_mul(c1,t16r),vec_add(spu_mul(c3,t17r),spu_mul(c6,t18r))))));
				y4i = vec_add(t13i,vec_add(spu_mul(c4,t14i),vec_add(spu_mul(c5,t15i),vec_add(spu_mul(c1,t16i),vec_add(spu_mul(c3,t17i),spu_mul(c6,t18i))))));
				y5r = vec_add(t13r,vec_add(spu_mul(c5,t14r),vec_add(spu_mul(c3,t15r),vec_add(spu_mul(c2,t16r),vec_add(spu_mul(c6,t17r),spu_mul(c1,t18r))))));
				y5i = vec_add(t13i,vec_add(spu_mul(c5,t14i),vec_add(spu_mul(c3,t15i),vec_add(spu_mul(c2,t16i),vec_add(spu_mul(c6,t17i),spu_mul(c1,t18i))))));
				y6r = vec_add(t13r,vec_add(spu_mul(c6,t14r),vec_add(spu_mul(c1,t15r),vec_add(spu_mul(c5,t16r),vec_add(spu_mul(c2,t17r),spu_mul(c4,t18r))))));
				y6i = vec_add(t13i,vec_add(spu_mul(c6,t14i),vec_add(spu_mul(c1,t15i),vec_add(spu_mul(c5,t16i),vec_add(spu_mul(c2,t17i),spu_mul(c4,t18i))))));

				y7r = vec_add(vec_sub(spu_mul(c12,t7r),spu_mul(c7,t8r)),vec_add(vec_sub(spu_mul(c11,t9r),spu_mul(c8,t10r)),vec_sub(spu_mul(c10,t11r),spu_mul(c9,t12r))));
				y7i = vec_add(vec_sub(spu_mul(c12,t7i),spu_mul(c7,t8i)),vec_add(vec_sub(spu_mul(c11,t9i),spu_mul(c8,t10i)),vec_sub(spu_mul(c10,t11i),spu_mul(c9,t12i))));
				y8r = vec_add(vec_sub(spu_mul(c11,t7r),spu_mul(c9,t8r)),vec_add(vec_sub(vec_sub(spu_mul(c8,t9r),spu_mul(c12,t10r)),spu_mul(c7,t11r)),spu_mul(c10,t12r)));
				y8i = vec_add(vec_sub(spu_mul(c11,t7i),spu_mul(c9,t8i)),vec_add(vec_sub(vec_sub(spu_mul(c8,t9i),spu_mul(c12,t10i)),spu_mul(c7,t11i)),spu_mul(c10,t12i)));
				y9r = vec_add(vec_sub(vec_sub(spu_mul(c10,t7r),spu_mul(c11,t8r)),spu_mul(c7,t9r)),vec_sub(vec_sub(spu_mul(c9,t10r),spu_mul(c12,t11r)),spu_mul(c8,t12r)));
				y9i = vec_add(vec_sub(vec_sub(spu_mul(c10,t7i),spu_mul(c11,t8i)),spu_mul(c7,t9i)),vec_sub(vec_sub(spu_mul(c9,t10i),spu_mul(c12,t11i)),spu_mul(c8,t12i)));
				y10r = vec_add(spu_mul(c9,t7r),vec_add(vec_sub(vec_sub(spu_mul(c12,t8r),spu_mul(c10,t9r)),spu_mul(c7,t10r)),vec_add(spu_mul(c8,t11r),spu_mul(c11,t12r))));
				y10i = vec_add(spu_mul(c9,t7i),vec_add(vec_sub(vec_sub(spu_mul(c12,t8i),spu_mul(c10,t9i)),spu_mul(c7,t10i)),vec_add(spu_mul(c8,t11i),spu_mul(c11,t12i))));
				y11r = vec_add(spu_mul(c8,t7r),vec_add(spu_mul(c10,t8r),vec_sub(vec_sub(vec_sub(spu_mul(c12,t9r),spu_mul(c11,t10r)),spu_mul(c9,t11r)),spu_mul(c7,t12r))));
				y11i = vec_add(spu_mul(c8,t7i),vec_add(spu_mul(c10,t8i),vec_sub(vec_sub(vec_sub(spu_mul(c12,t9i),spu_mul(c11,t10i)),spu_mul(c9,t11i)),spu_mul(c7,t12i))));
				y12r = vec_add(spu_mul(c7,t7r),vec_add(spu_mul(c8,t8r),vec_add(spu_mul(c9,t9r),vec_add(spu_mul(c10,t10r),vec_add(spu_mul(c11,t11r),spu_mul(c12,t12r))))));
				y12i = vec_add(spu_mul(c7,t7i),vec_add(spu_mul(c8,t8i),vec_add(spu_mul(c9,t9i),vec_add(spu_mul(c10,t10i),vec_add(spu_mul(c11,t11i),spu_mul(c12,t12i))))));
				z[j00] = vec_add(z[j00],vec_add(t1r,vec_add(t2r,vec_add(t3r,vec_add(t4r,vec_add(t5r,t6r))))));
				z[j00+1] = vec_add(z[j00+1],vec_add(t1i,vec_add(t2i,vec_add(t3i,vec_add(t4i,vec_add(t5i,t6i))))));
				z[j01] = vec_sub(y1r,y12i);
				z[j01+1] = vec_add(y1i,y12r);
				z[j2] = vec_sub(y2r,y11i);
				z[j2+1] = vec_add(y2i,y11r);
				z[j3] = vec_sub(y3r,y10i);
				z[j3+1] = vec_add(y3i,y10r);
				z[j4] = vec_sub(y4r,y9i);
				z[j4+1] = vec_add(y4i,y9r);
				z[j5] = vec_sub(y5r,y8i);
				z[j5+1] = vec_add(y5i,y8r);
				z[j6] = vec_sub(y6r,y7i);
				z[j6+1] = vec_add(y6i,y7r);
				z[j7] = vec_add(y6r,y7i);
				z[j7+1] = vec_sub(y6i,y7r);
				z[j8] = vec_add(y5r,y8i);
				z[j8+1] = vec_sub(y5i,y8r);
				z[j9] = vec_add(y4r,y9i);
				z[j9+1] = vec_sub(y4i,y9r);
				z[j10] = vec_add(y3r,y10i);
				z[j10+1] = vec_sub(y3i,y10r);
				z[j11] = vec_add(y2r,y11i);
				z[j11+1] = vec_sub(y2i,y11r);
				z[j12] = vec_add(y1r,y12i);
				z[j12+1] = vec_sub(y1i,y12r);
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				float value = 1.0;
				c1 = spu_splats(value);
				value= 0.923879533;
				c2 = spu_splats(value);
				value = 0.382683432;
				c3 = spu_splats(value);
				value = 0.707106781;
				c4 = spu_splats(value);
			} else if (mu==3) {
				float value= -1.0;
				c1 = spu_splats(value);
				value = 0.382683432;
				c2 = spu_splats(value);
				value = 0.923879533;
				c3 = spu_splats(value);
				value = -0.707106781;
				c4 = spu_splats(value);
			} else if (mu==5) {
				float value = 1.0;
				c1 = spu_splats(value);
				value = -0.382683432;
				c2 = spu_splats(value);
				value = 0.923879533;
				c3 = spu_splats(value);
				value = -0.707106781;
				c4 = spu_splats(value);
			} else if (mu==7) {
				float value = -1.0;
				c1 = spu_splats(value);
				value = -0.923879533;
				c2 = spu_splats(value);
				value = 0.382683432;
				c3 = spu_splats(value);
				value = 0.707106781;
				c4 = spu_splats(value);
			} else if (mu==9) {
				float value = 1.0;
				c1 = spu_splats(value);
				value = -0.923879533;
				c2 = spu_splats(value);
				value = -0.382683432;
				c3 = spu_splats(value);
				value= 0.707106781;
				c4 = spu_splats(value);
			} else if (mu==11) {
				float value = -1.0;
				c1 = spu_splats(value);
				value = -0.382683432;
				c2 = spu_splats(value);
				value = -0.923879533;
				c3 = spu_splats(value);
				value = -0.707106781;
				c4 = spu_splats(value);
			} else if (mu==13) {
				float value = 1.0;
				c1 = spu_splats(value);
				value = 0.382683432;
				c2 = spu_splats(value);
				value = -0.923879533;
				c3 = spu_splats(value);
				value = -0.707106781;
				c4 = spu_splats(value);
			} else {
				float value = -1.0;
				c1 = spu_splats(value);
				value = 0.923879533;
				c2 = spu_splats(value);
				value = -0.382683432;
				c3 = spu_splats(value);
				value = 0.707106781;
				c4 = spu_splats(value);
			}
			c5 = spu_mul(c1,c4);
			c6 = spu_mul(c1,c3);
			c7 = spu_mul(c1,c2);
			for (l=0; l<m; l++) {
				t1r = vec_add(z[j00],z[j8]);
				t1i = vec_add(z[j00+1],z[j8+1]);
				t2r = vec_add(z[j4],z[j12]);
				t2i = vec_add(z[j4+1],z[j12+1]);
				t3r = vec_sub(z[j00],z[j8]);
				t3i = vec_sub(z[j00+1],z[j8+1]);
				t4r = spu_mul(c1,vec_sub(z[j4],z[j12]));
				t4i = spu_mul(c1,vec_sub(z[j4+1],z[j12+1]));
				t5r = vec_add(t1r,t2r);
				t5i = vec_add(t1i,t2i);
				t6r = vec_sub(t1r,t2r);
				t6i = vec_sub(t1i,t2i);
				t7r = vec_add(z[j01],z[j9]);
				t7i = vec_add(z[j01+1],z[j9+1]);
				t8r = vec_add(z[j5],z[j13]);
				t8i = vec_add(z[j5+1],z[j13+1]);
				t9r = vec_sub(z[j01],z[j9]);
				t9i = vec_sub(z[j01+1],z[j9+1]);
				t10r = vec_sub(z[j5],z[j13]);
				t10i = vec_sub(z[j5+1],z[j13+1]);
				t11r = vec_add(t7r,t8r);
				t11i = vec_add(t7i,t8i);
				t12r = vec_sub(t7r,t8r);
				t12i = vec_sub(t7i,t8i);
				t13r = vec_add(z[j2],z[j10]);
				t13i = vec_add(z[j2+1],z[j10+1]);
				t14r = vec_add(z[j6],z[j14]);
				t14i = vec_add(z[j6+1],z[j14+1]);
				t15r = vec_sub(z[j2],z[j10]);
				t15i = vec_sub(z[j2+1],z[j10+1]);
				t16r = vec_sub(z[j6],z[j14]);
				t16i = vec_sub(z[j6+1],z[j14+1]);
				t17r = vec_add(t13r,t14r);
				t17i = vec_add(t13i,t14i);
				t18r = spu_mul(c4,vec_sub(t15r,t16r));
				t18i = spu_mul(c4,vec_sub(t15i,t16i));
				t19r = spu_mul(c5,vec_add(t15r,t16r));
				t19i = spu_mul(c5,vec_add(t15i,t16i));
				t20r = spu_mul(c1,vec_sub(t13r,t14r));
				t20i = spu_mul(c1,vec_sub(t13i,t14i));
				t21r = vec_add(z[j3],z[j11]);
				t21i = vec_add(z[j3+1],z[j11+1]);
				t22r = vec_add(z[j7],z[j15]);
				t22i = vec_add(z[j7+1],z[j15+1]);
				t23r = vec_sub(z[j3],z[j11]);
				t23i = vec_sub(z[j3+1],z[j11+1]);
				t24r = vec_sub(z[j7],z[j15]);
				t24i = vec_sub(z[j7+1],z[j15+1]);
				t25r = vec_add(t21r,t22r);
				t25i = vec_add(t21i,t22i);
				t26r = vec_sub(t21r,t22r);
				t26i = vec_sub(t21i,t22i);
				t27r = vec_add(t9r,t24r);
				t27i = vec_add(t9i,t24i);
				t28r = vec_add(t10r,t23r);
				t28i =vec_add(t10i,t23i);
				t29r = vec_sub(t9r,t24r);
				t29i = vec_sub(t9i,t24i);
				t30r = vec_sub(t10r,t23r);
				t30i = vec_sub(t10i,t23i);
				t31r = vec_add(t5r,t17r);
				t31i = vec_add(t5i,t17i);
				t32r = vec_add(t11r,t25r);
				t32i = vec_add(t11i,t25i);
				t33r = vec_add(t3r,t18r);
				t33i = vec_add(t3i,t18i);
				t34r = vec_sub(spu_mul(c2,t29r),spu_mul(c6,t30r));
				t34i = vec_sub(spu_mul(c2,t29i),spu_mul(c6,t30i));
				t35r = vec_sub(t3r,t18r);
				t35i = vec_sub(t3i,t18i);
				t36r = vec_sub(spu_mul(c7,t27r),spu_mul(c3,t28r));
				t36i = vec_sub(spu_mul(c7,t27i),spu_mul(c3,t28i));
				t37r = vec_add(t4r,t19r);
				t37i = vec_add(t4i,t19i);
				t38r = vec_add(spu_mul(c3,t27r),spu_mul(c7,t28r));
				t38i = vec_add(spu_mul(c3,t27i),spu_mul(c7,t28i));
				t39r = vec_sub(t4r,t19r);
				t39i = vec_sub(t4i,t19i);
				t40r = vec_add(spu_mul(c6,t29r),spu_mul(c2,t30r));
				t40i = vec_add(spu_mul(c6,t29i),spu_mul(c2,t30i));
				t41r = spu_mul(c4,vec_sub(t12r,t26r));
				t41i = spu_mul(c4,vec_sub(t12i,t26i));
				t42r = spu_mul(c5,vec_add(t12r,t26r));
				t42i = spu_mul(c5,vec_add(t12i,t26i));
				y1r = vec_add(t33r,t34r);
				y1i = vec_add(t33i,t34i);
				y2r = vec_add(t6r,t41r);
				y2i = vec_add(t6i,t41i);
				y3r = vec_add(t35r,t40r);
				y3i = vec_add(t35i,t40i);
				y4r = vec_sub(t5r,t17r);
				y4i = vec_sub(t5i,t17i);
				y5r = vec_sub(t35r,t40r);
				y5i = vec_sub(t35i,t40i);
				y6r = vec_sub(t6r,t41r);
				y6i = vec_sub(t6i,t41i);
				y7r = vec_sub(t33r,t34r);
				y7i = vec_sub(t33i,t34i);
				y9r = vec_sub(t38r,t37r);
				y9i = vec_sub(t38i,t37i);
				y10r = vec_sub(t42r,t20r);
				y10i = vec_sub(t42i,t20i);
				y11r = vec_add(t36r,t39r);
				y11i = vec_add(t36i,t39i);
				y12r = spu_mul(c1,vec_sub(t11r,t25r));
				y12i = spu_mul(c1,vec_sub(t11i,t25i));
				y13r = vec_sub(t36r,t39r);
				y13i = vec_sub(t36i,t39i);
				y14r = vec_add(t42r,t20r);
				y14i = vec_add(t42i,t20i);
				y15r = vec_add(t38r,t37r);
				y15i = vec_add(t38i,t37i);
				z[j00] = vec_add(t31r,t32r);
				z[j00+1] = vec_add(t31i,t32i);
				z[j01] = vec_sub(y1r,y15i);
				z[j01+1] = vec_add(y1i,y15r);
				z[j2] = vec_sub(y2r,y14i);
				z[j2+1] = vec_add(y2i,y14r);
				z[j3] = vec_sub(y3r,y13i);
				z[j3+1] = vec_add(y3i,y13r);
				z[j4] = vec_sub(y4r,y12i);
				z[j4+1] = vec_add(y4i,y12r);
				z[j5] = vec_sub(y5r,y11i);
				z[j5+1] = vec_add(y5i,y11r);
				z[j6] = vec_sub(y6r,y10i);
				z[j6+1] = vec_add(y6i,y10r);
				z[j7] = vec_sub(y7r,y9i);
				z[j7+1] = vec_add(y7i,y9r);
				z[j8] = vec_sub(t31r,t32r);
				z[j8+1] = vec_sub(t31i,t32i);
				z[j9] = vec_add(y7r,y9i);
				z[j9+1] = vec_sub(y7i,y9r);
				z[j10] = vec_add(y6r,y10i);
				z[j10+1] = vec_sub(y6i,y10r);
				z[j11] = vec_add(y5r,y11i);
				z[j11+1] = vec_sub(y5i,y11r);
				z[j12] = vec_add(y4r,y12i);
				z[j12+1] = vec_sub(y4i,y12r);
				z[j13] = vec_add(y3r,y13i);
				z[j13+1] = vec_sub(y3i,y13r);
				z[j14] = vec_add(y2r,y14i);
				z[j14+1] = vec_sub(y2i,y14r);
				z[j15] = vec_add(y1r,y15i);
				z[j15+1] = vec_sub(y1i,y15r);
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
	}
}
#endif

void fft(int isign, int n, float cz[])
{
	static int kfax[] = { 16,13,11,9,8,7,5,4,3,2 };
	register float *z=(float*)cz;
	register int j00,j01,j2,j3,j4,j5,j6,j7,j8,j9,j10,j11,j12,j13,j14,j15,jt;
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
	for (jfax=0; jfax<10; jfax++) {

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
        j00 = 0;
        j01 = j00+jinc;

		/* if factor is 2 */
        if (ifac==2) {
			for (l=0; l<m; l++) {
				t1r = z[j00]-z[j01];
				t1i = z[j00+1]-z[j01+1];
				z[j00] = z[j00]+z[j01];
				z[j00+1] = z[j00+1]+z[j01+1];
				z[j01] = t1r;
				z[j01+1] = t1i;
				jt = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
        j2 = j01+jinc;
        if (j2>=jmax) j2 = j2-jmax;

		/* if factor is 3 */
        if (ifac==3) {
			if (mu==1)
				c1 = 0.866025404;
			else
				c1 = -0.866025404;
			for (l=0; l<m; l++) {
				t1r = z[j01]+z[j2];
				t1i = z[j01+1]+z[j2+1];
				y1r = z[j00]-0.5*t1r;
				y1i = z[j00+1]-0.5*t1i;
				y2r = c1*(z[j01]-z[j2]);
				y2i = c1*(z[j01+1]-z[j2+1]);
				z[j00] = z[j00]+t1r;
				z[j00+1] = z[j00+1]+t1i;
				z[j01] = y1r-y2i;
				z[j01+1] = y1i+y2r;
				z[j2] = y1r+y2i;
				z[j2+1] = y1i-y2r;
				jt = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				t1r = z[j00]+z[j2];
				t1i = z[j00+1]+z[j2+1];
				t2r = z[j01]+z[j3];
				t2i = z[j01+1]+z[j3+1];
				y1r = z[j00]-z[j2];
				y1i = z[j00+1]-z[j2+1];
				y3r = c1*(z[j01]-z[j3]);
				y3i = c1*(z[j01+1]-z[j3+1]);
				z[j00] = t1r+t2r;
				z[j00+1] = t1i+t2i;
				z[j01] = y1r-y3i;
				z[j01+1] = y1i+y3r;
				z[j2] = t1r-t2r;
				z[j2+1] = t1i-t2i;
				z[j3] = y1r+y3i;
				z[j3+1] = y1i-y3r;
				jt = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j4 = j3+jinc;
		if (j4>=jmax) j4 = j4-jmax;

		/* if factor is 5 */
		if (ifac==5) {
			if (mu==1) {
				c1 = 0.559016994;
				c2 = 0.951056516;
				c3 = 0.587785252;
			} else if (mu==2) {
				c1 = -0.559016994;
				c2 = 0.587785252;
				c3 = -0.951056516;
			} else if (mu==3) {
				c1 = -0.559016994;
				c2 = -0.587785252;
				c3 = 0.951056516;
			} else { 
				c1 = 0.559016994;
				c2 = -0.951056516;
				c3 = -0.587785252;
			}
			for (l=0; l<m; l++) {
				t1r = z[j01]+z[j4];
				t1i = z[j01+1]+z[j4+1];
				t2r = z[j2]+z[j3];
				t2i = z[j2+1]+z[j3+1];
				t3r = z[j01]-z[j4];
				t3i = z[j01+1]-z[j4+1];
				t4r = z[j2]-z[j3];
				t4i = z[j2+1]-z[j3+1];
				t5r = t1r+t2r;
				t5i = t1i+t2i;
				t6r = c1*(t1r-t2r);
				t6i = c1*(t1i-t2i);
				t7r = z[j00]-0.25*t5r;
				t7i = z[j00+1]-0.25*t5i;
				y1r = t7r+t6r;
				y1i = t7i+t6i;
				y2r = t7r-t6r;
				y2i = t7i-t6i;
				y3r = c3*t3r-c2*t4r;
				y3i = c3*t3i-c2*t4i;
				y4r = c2*t3r+c3*t4r;
				y4i = c2*t3i+c3*t4i;
				z[j00] = z[j00]+t5r;
				z[j00+1] = z[j00+1]+t5i;
				z[j01] = y1r-y4i;
				z[j01+1] = y1i+y4r;
				z[j2] = y2r-y3i;
				z[j2+1] = y2i+y3r;
				z[j3] = y2r+y3i;
				z[j3+1] = y2i-y3r;
				z[j4] = y1r+y4i;
				z[j4+1] = y1i-y4r;
				jt = j4+2;
				j4 = j3+2;
				j3 = j2+2;
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				c1 = 0.623489802;
				c2 = -0.222520934;
				c3 = -0.900968868;
				c4 = 0.781831482;
				c5 = 0.974927912;
				c6 = 0.433883739;
			} else if (mu==2) {
				c1 = -0.222520934;
				c2 = -0.900968868;
				c3 = 0.623489802;
				c4 = 0.974927912;
				c5 = -0.433883739;
				c6 = -0.781831482;
			} else if (mu==3) {
				c1 = -0.900968868;
				c2 = 0.623489802;
				c3 = -0.222520934;
				c4 = 0.433883739;
				c5 = -0.781831482;
				c6 = 0.974927912;
			} else if (mu==4) {
				c1 = -0.900968868;
				c2 = 0.623489802;
				c3 = -0.222520934;
				c4 = -0.433883739;
				c5 = 0.781831482;
				c6 = -0.974927912;
			} else if (mu==5) {
				c1 = -0.222520934;
				c2 = -0.900968868;
				c3 = 0.623489802;
				c4 = -0.974927912;
				c5 = 0.433883739;
				c6 = 0.781831482;
			} else {
				c1 = 0.623489802;
				c2 = -0.222520934;
				c3 = -0.900968868;
				c4 = -0.781831482;
				c5 = -0.974927912;
				c6 = -0.433883739;
			}
			for (l=0; l<m; l++) {
				t1r = z[j01]+z[j6];
				t1i = z[j01+1]+z[j6+1];
				t2r = z[j2]+z[j5];
				t2i = z[j2+1]+z[j5+1];
				t3r = z[j3]+z[j4];
				t3i = z[j3+1]+z[j4+1];
				t4r = z[j01]-z[j6];
				t4i = z[j01+1]-z[j6+1];
				t5r = z[j2]-z[j5];
				t5i = z[j2+1]-z[j5+1];
				t6r = z[j3]-z[j4];
				t6i = z[j3+1]-z[j4+1];
				t7r = z[j00]-0.5*t3r;
				t7i = z[j00+1]-0.5*t3i;
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
				z[j00] = z[j00]+t1r+t2r+t3r;
				z[j00+1] = z[j00+1]+t1i+t2i+t3i;
				z[j01] = y1r-y6i;
				z[j01+1] = y1i+y6r;
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j7 = j6+jinc;
		if (j7>=jmax) j7 = j7-jmax;

		/* if factor is 8 */
		if (ifac==8) {
			if (mu==1) {
				c1 = 1.0;
				c2 = 0.707106781;
			} else if (mu==3) {
				c1 = -1.0;
				c2 = -0.707106781;
			} else if (mu==5) {
				c1 = 1.0;
				c2 = -0.707106781;
			} else {
				c1 = -1.0;
				c2 = 0.707106781;
			}
			c3 = c1*c2;
			for (l=0; l<m; l++) {
				t1r = z[j00]+z[j4];
				t1i = z[j00+1]+z[j4+1];
				t2r = z[j00]-z[j4];
				t2i = z[j00+1]-z[j4+1];
				t3r = z[j01]+z[j5];
				t3i = z[j01+1]+z[j5+1];
				t4r = z[j01]-z[j5];
				t4i = z[j01+1]-z[j5+1];
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
				z[j00] = t9r+t10r;
				z[j00+1] = t9i+t10i;
				z[j01] = y1r-y7i;
				z[j01+1] = y1i+y7r;
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
		j8 = j7+jinc;
		if (j8>=jmax) j8 = j8-jmax;

		/* if factor is 9 */
		if (ifac==9) {
			if (mu==1) {
				c1 = 0.866025404;
				c2 = 0.766044443;
				c3 = 0.642787610;
				c4 = 0.173648178;
				c5 = 0.984807753;
			} else if (mu==2) {
				c1 = -0.866025404;
				c2 = 0.173648178;
				c3 = 0.984807753;
				c4 = -0.939692621;
				c5 = 0.342020143;
			} else if (mu==4) {
				c1 = 0.866025404;
				c2 = -0.939692621;
				c3 = 0.342020143;
				c4 = 0.766044443;
				c5 = -0.642787610;
			} else if (mu==5) {
				c1 = -0.866025404;
				c2 = -0.939692621;
				c3 = -0.342020143;
				c4 = 0.766044443;
				c5 = 0.642787610;
			} else if (mu==7) {
				c1 = 0.866025404;
				c2 = 0.173648178;
				c3 = -0.984807753;
				c4 = -0.939692621;
				c5 = -0.342020143;
			} else {
				c1 = -0.866025404;
				c2 = 0.766044443;
				c3 = -0.642787610;
				c4 = 0.173648178;
				c5 = -0.984807753;
			}
			c6 = c1*c2;
			c7 = c1*c3;
			c8 = c1*c4;
			c9 = c1*c5;
			for (l=0; l<m; l++) {
				t1r = z[j3]+z[j6];
				t1i = z[j3+1]+z[j6+1];
				t2r = z[j00]-0.5*t1r;
				t2i = z[j00+1]-0.5*t1i;
				t3r = c1*(z[j3]-z[j6]);
				t3i = c1*(z[j3+1]-z[j6+1]);
				t4r = z[j00]+t1r;
				t4i = z[j00+1]+t1i;
				t5r = z[j4]+z[j7];
				t5i = z[j4+1]+z[j7+1];
				t6r = z[j01]-0.5*t5r;
				t6i = z[j01+1]-0.5*t5i;
				t7r = z[j4]-z[j7];
				t7i = z[j4+1]-z[j7+1];
				t8r = z[j01]+t5r;
				t8i = z[j01+1]+t5i;
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
				z[j00] = t4r+t13r;
				z[j00+1] = t4i+t13i;
				z[j01] = y1r-y8i;
				z[j01+1] = y1i+y8r;
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				c1 = 0.841253533;
				c2 = 0.415415013;
				c3 = -0.142314838;
				c4 = -0.654860734;
				c5 = -0.959492974;
				c6 = 0.540640817;
				c7 = 0.909631995;
				c8 = 0.989821442;
				c9 = 0.755749574;
				c10 = 0.281732557;
			} else if (mu==2) {
				c1 = 0.415415013;
				c2 = -0.654860734;
				c3 = -0.959492974;
				c4 = -0.142314838;
				c5 = 0.841253533;
				c6 = 0.909631995;
				c7 = 0.755749574;
				c8 = -0.281732557;
				c9 = -0.989821442;
				c10 = -0.540640817;
			} else if (mu==3) {
				c1 = -0.142314838;
				c2 = -0.959492974;
				c3 = 0.415415013;
				c4 = 0.841253533;
				c5 = -0.654860734;
				c6 = 0.989821442;
				c7 = -0.281732557;
				c8 = -0.909631995;
				c9 = 0.540640817;
				c10 = 0.755749574;
			} else if (mu==4) {
				c1 = -0.654860734;
				c2 = -0.142314838;
				c3 = 0.841253533;
				c4 = -0.959492974;
				c5 = 0.415415013;
				c6 = 0.755749574;
				c7 = -0.989821442;
				c8 = 0.540640817;
				c9 = 0.281732557;
				c10 = -0.909631995;
			} else if (mu==5) {
				c1 = -0.959492974;
				c2 = 0.841253533;
				c3 = -0.654860734;
				c4 = 0.415415013;
				c5 = -0.142314838;
				c6 = 0.281732557;
				c7 = -0.540640817;
				c8 = 0.755749574;
				c9 = -0.909631995;
				c10 = 0.989821442;
			} else if (mu==6) {
				c1 = -0.959492974;
				c2 = 0.841253533;
				c3 = -0.654860734;
				c4 = 0.415415013;
				c5 = -0.142314838;
				c6 = -0.281732557;
				c7 = 0.540640817;
				c8 = -0.755749574;
				c9 = 0.909631995;
				c10 = -0.989821442;
			} else if (mu==7) {
				c1 = -0.654860734;
				c2 = -0.142314838;
				c3 = 0.841253533;
				c4 = -0.959492974;
				c5 = 0.415415013;
				c6 = -0.755749574;
				c7 = 0.989821442;
				c8 = -0.540640817;
				c9 = -0.281732557;
				c10 = 0.909631995;
			} else if (mu==8) {
				c1 = -0.142314838;
				c2 = -0.959492974;
				c3 = 0.415415013;
				c4 = 0.841253533;
				c5 = -0.654860734;
				c6 = -0.989821442;
				c7 = 0.281732557;
				c8 = 0.909631995;
				c9 = -0.540640817;
				c10 = -0.755749574;
			} else if (mu==9) {
				c1 = 0.415415013;
				c2 = -0.654860734;
				c3 = -0.959492974;
				c4 = -0.142314838;
				c5 = 0.841253533;
				c6 = -0.909631995;
				c7 = -0.755749574;
				c8 = 0.281732557;
				c9 = 0.989821442;
				c10 = 0.540640817;
			} else {
				c1 = 0.841253533;
				c2 = 0.415415013;
				c3 = -0.142314838;
				c4 = -0.654860734;
				c5 = -0.959492974;
				c6 = -0.540640817;
				c7 = -0.909631995;
				c8 = -0.989821442;
				c9 = -0.755749574;
				c10 = -0.281732557;
			}
			for (l=0; l<m; l++) {
				t1r = z[j01]+z[j10];
				t1i = z[j01+1]+z[j10+1];
				t2r = z[j2]+z[j9];
				t2i = z[j2+1]+z[j9+1];
				t3r = z[j3]+z[j8];
				t3i = z[j3+1]+z[j8+1];
				t4r = z[j4]+z[j7];
				t4i = z[j4+1]+z[j7+1];
				t5r = z[j5]+z[j6];
				t5i = z[j5+1]+z[j6+1];
				t6r = z[j01]-z[j10];
				t6i = z[j01+1]-z[j10+1];
				t7r = z[j2]-z[j9];
				t7i = z[j2+1]-z[j9+1];
				t8r = z[j3]-z[j8];
				t8i = z[j3+1]-z[j8+1];
				t9r = z[j4]-z[j7];
				t9i = z[j4+1]-z[j7+1];
				t10r = z[j5]-z[j6];
				t10i = z[j5+1]-z[j6+1];
				t11r = z[j00]-0.5*t5r;
				t11i = z[j00+1]-0.5*t5i;
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
				z[j00] = z[j00]+t1r+t2r+t3r+t4r+t5r;
				z[j00+1] = z[j00+1]+t1i+t2i+t3i+t4i+t5i;
				z[j01] = y1r-y10i;
				z[j01+1] = y1i+y10r;
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				c1 = 0.885456026;
				c2 = 0.568064747;
				c3 = 0.120536680;
				c4 = -0.354604887;
				c5 = -0.748510748;
				c6 = -0.970941817;
				c7 = 0.464723172;
				c8 = 0.822983866;
				c9 = 0.992708874;
				c10 = 0.935016243;
				c11 = 0.663122658;
				c12 = 0.239315664;
			} else if (mu==2) {
				c1 = 0.568064747;
				c2 = -0.354604887;
				c3 = -0.970941817;
				c4 = -0.748510748;
				c5 = 0.120536680;
				c6 = 0.885456026;
				c7 = 0.822983866;
				c8 = 0.935016243;
				c9 = 0.239315664;
				c10 = -0.663122658;
				c11 = -0.992708874;
				c12 = -0.464723172;
			} else if (mu==3) {
				c1 = 0.120536680;
				c2 = -0.970941817;
				c3 = -0.354604887;
				c4 = 0.885456026;
				c5 = 0.568064747;
				c6 = -0.748510748;
				c7 = 0.992708874;
				c8 = 0.239315664;
				c9 = -0.935016243;
				c10 = -0.464723172;
				c11 = 0.822983866;
				c12 = 0.663122658;
			} else if (mu==4) {
				c1 = -0.354604887;
				c2 = -0.748510748;
				c3 = 0.885456026;
				c4 = 0.120536680;
				c5 = -0.970941817;
				c6 = 0.568064747;
				c7 = 0.935016243;
				c8 = -0.663122658;
				c9 = -0.464723172;
				c10 = 0.992708874;
				c11 = -0.239315664;
				c12 = -0.822983866;
			} else if (mu==5) {
				c1 = -0.748510748;
				c2 = 0.120536680;
				c3 = 0.568064747;
				c4 = -0.970941817;
				c5 = 0.885456026;
				c6 = -0.354604887;
				c7 = 0.663122658;
				c8 = -0.992708874;
				c9 = 0.822983866;
				c10 = -0.239315664;
				c11 = -0.464723172;
				c12 = 0.935016243;
			} else if (mu==6) {
				c1 = -0.970941817;
				c2 = 0.885456026;
				c3 = -0.748510748;
				c4 = 0.568064747;
				c5 = -0.354604887;
				c6 = 0.120536680;
				c7 = 0.239315664;
				c8 = -0.464723172;
				c9 = 0.663122658;
				c10 = -0.822983866;
				c11 = 0.935016243;
				c12 = -0.992708874;
			} else if (mu==7) {
				c1 = -0.970941817;
				c2 = 0.885456026;
				c3 = -0.748510748;
				c4 = 0.568064747;
				c5 = -0.354604887;
				c6 = 0.120536680;
				c7 = -0.239315664;
				c8 = 0.464723172;
				c9 = -0.663122658;
				c10 = 0.822983866;
				c11 = -0.935016243;
				c12 = 0.992708874;
			} else if (mu==8) {
				c1 = -0.748510748;
				c2 = 0.120536680;
				c3 = 0.568064747;
				c4 = -0.970941817;
				c5 = 0.885456026;
				c6 = -0.354604887;
				c7 = -0.663122658;
				c8 = 0.992708874;
				c9 = -0.822983866;
				c10 = 0.239315664;
				c11 = 0.464723172;
				c12 = -0.935016243;
			} else if (mu==9) {
				c1 = -0.354604887;
				c2 = -0.748510748;
				c3 = 0.885456026;
				c4 = 0.120536680;
				c5 = -0.970941817;
				c6 = 0.568064747;
				c7 = -0.935016243;
				c8 = 0.663122658;
				c9 = 0.464723172;
				c10 = -0.992708874;
				c11 = 0.239315664;
				c12 = 0.822983866;
			} else if (mu==10) {
				c1 = 0.120536680;
				c2 = -0.970941817;
				c3 = -0.354604887;
				c4 = 0.885456026;
				c5 = 0.568064747;
				c6 = -0.748510748;
				c7 = -0.992708874;
				c8 = -0.239315664;
				c9 = 0.935016243;
				c10 = 0.464723172;
				c11 = -0.822983866;
				c12 = -0.663122658;
			} else if (mu==11) {
				c1 = 0.568064747;
				c2 = -0.354604887;
				c3 = -0.970941817;
				c4 = -0.748510748;
				c5 = 0.120536680;
				c6 = 0.885456026;
				c7 = -0.822983866;
				c8 = -0.935016243;
				c9 = -0.239315664;
				c10 = 0.663122658;
				c11 = 0.992708874;
				c12 = 0.464723172;
			} else {
				c1 = 0.885456026;
				c2 = 0.568064747;
				c3 = 0.120536680;
				c4 = -0.354604887;
				c5 = -0.748510748;
				c6 = -0.970941817;
				c7 = -0.464723172;
				c8 = -0.822983866;
				c9 = -0.992708874;
				c10 = -0.935016243;
				c11 = -0.663122658;
				c12 = -0.239315664;
			}
			for (l=0; l<m; l++) {
				t1r = z[j01]+z[j12];
				t1i = z[j01+1]+z[j12+1];
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
				t7r = z[j01]-z[j12];
				t7i = z[j01+1]-z[j12+1];
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
				t13r = z[j00]-0.5*t6r;
				t13i = z[j00+1]-0.5*t6i;
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
				z[j00] = z[j00]+t1r+t2r+t3r+t4r+t5r+t6r;
				z[j00+1] = z[j00+1]+t1i+t2i+t3i+t4i+t5i+t6i;
				z[j01] = y1r-y12i;
				z[j01+1] = y1i+y12r;
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
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
				c2 = 0.923879533;
				c3 = 0.382683432;
				c4 = 0.707106781;
			} else if (mu==3) {
				c1 = -1.0;
				c2 = 0.382683432;
				c3 = 0.923879533;
				c4 = -0.707106781;
			} else if (mu==5) {
				c1 = 1.0;
				c2 = -0.382683432;
				c3 = 0.923879533;
				c4 = -0.707106781;
			} else if (mu==7) {
				c1 = -1.0;
				c2 = -0.923879533;
				c3 = 0.382683432;
				c4 = 0.707106781;
			} else if (mu==9) {
				c1 = 1.0;
				c2 = -0.923879533;
				c3 = -0.382683432;
				c4 = 0.707106781;
			} else if (mu==11) {
				c1 = -1.0;
				c2 = -0.382683432;
				c3 = -0.923879533;
				c4 = -0.707106781;
			} else if (mu==13) {
				c1 = 1.0;
				c2 = 0.382683432;
				c3 = -0.923879533;
				c4 = -0.707106781;
			} else {
				c1 = -1.0;
				c2 = 0.923879533;
				c3 = -0.382683432;
				c4 = 0.707106781;
			}
			c5 = c1*c4;
			c6 = c1*c3;
			c7 = c1*c2;
			for (l=0; l<m; l++) {
				t1r = z[j00]+z[j8];
				t1i = z[j00+1]+z[j8+1];
				t2r = z[j4]+z[j12];
				t2i = z[j4+1]+z[j12+1];
				t3r = z[j00]-z[j8];
				t3i = z[j00+1]-z[j8+1];
				t4r = c1*(z[j4]-z[j12]);
				t4i = c1*(z[j4+1]-z[j12+1]);
				t5r = t1r+t2r;
				t5i = t1i+t2i;
				t6r = t1r-t2r;
				t6i = t1i-t2i;
				t7r = z[j01]+z[j9];
				t7i = z[j01+1]+z[j9+1];
				t8r = z[j5]+z[j13];
				t8i = z[j5+1]+z[j13+1];
				t9r = z[j01]-z[j9];
				t9i = z[j01+1]-z[j9+1];
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
				z[j00] = t31r+t32r;
				z[j00+1] = t31i+t32i;
				z[j01] = y1r-y15i;
				z[j01+1] = y1i+y15r;
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
				j2 = j01+2;
				j01 = j00+2;
				j00 = jt;
			}
			continue;
		}
	}
}
