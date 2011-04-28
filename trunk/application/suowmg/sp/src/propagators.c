#include "propagators.h"

memIsoFFD3 *alloc_iso_ffd3(int nx, int ny) {

  int max1d = IMAX(nx, ny);
  memIsoFFD3 *mem = (memIsoFFD3 *)malloc(sizeof(memIsoFFD3));

  mem->a1     = allocfloat1(nx*ny);
  mem->a2     = allocfloat1(nx*ny);
  mem->b1     = allocfloat1(nx*ny);
  mem->zav    = allocfloatcomplex1(nx*ny);
  mem->zbv    = allocfloatcomplex1(nx*ny);
  mem->zcv    = allocfloatcomplex1(nx*ny);
  mem->zdv    = allocfloatcomplex1(nx*ny);
  mem->zbnx1v = allocfloatcomplex1(max1d);
  mem->zbnxnv = allocfloatcomplex1(max1d);
  mem->zdnx1v = allocfloatcomplex1(max1d);
  mem->zdnxnv = allocfloatcomplex1(max1d);
  mem->zcc    = allocfloatcomplex1(max1d);
  mem->c1     = allocfloat1(max1d);
  mem->c2a    = allocfloat1(max1d);
  mem->c2b    = allocfloat1(max1d);
  mem->c2     = allocfloat1(max1d);
  mem->ssv    = allocfloat1(max1d);
  mem->c1i    = allocfloat1(max1d);
  mem->c2r    = allocfloat1(max1d);
  mem->c2i    = allocfloat1(max1d);
  mem->rnza   = allocfloat1(max1d);
  mem->rnzc   = allocfloat1(max1d);
  mem->zev    = allocfloatcomplex1(nx*ny);
  mem->zfv    = allocfloatcomplex1(max1d);
  mem->zfv2   = allocfloatcomplex1(max1d);

  return mem;
}


void iso_ps_ud(fftwf_complex *rec, fftwf_complex *src, int nx, int ny, float *kx2, float *ky2, float k0,
	       float dz, fftwf_plan r1, fftwf_plan s1, fftwf_plan r2, fftwf_plan s2) {
  
  float rt, rp;
  float k02;
  float scale;

  scale=1.0f/((float)(nx*ny));
        
  // FFT over (x,y) --> (Kx,Ky) domain
  fftwf_execute(r1);
  fftwf_execute(s1);

  // Propagation by phase-shift
  k02 = k0*k0;
  for ( int iy = 0; iy < ny; iy++) {
    for ( int ix = 0; ix < nx; ix++) {
      rt = k02 - kx2[ix] - ky2[iy];
      if ( rt >= 0.0f ) {
	rp = sqrtf(rt)*dz;
	rec[iy*nx+ix] *= (cosf(rp)+sinf(rp)*I) * scale;
	src[iy*nx+ix] *= (cosf(rp)-sinf(rp)*I) * scale;
      } else {
	rp = -sqrtf(-rt)*dz;
	rec[iy*nx+ix] *= expf(rp) * scale;
	src[iy*nx+ix] *= expf(rp) * scale;
      }
    }
  }

  // FFT over (Kxm,Kym) --> (xm,ym) domain
  fftwf_execute(r2);
  fftwf_execute(s2);
      
  return;
    

}


void ssf_ud( fftwf_complex *rec, fftwf_complex *src, int nx, int ny, float *vP2D, float vB, float dz, float w )
{
      
  float rt, k0dzr;
  
  // Split-step Fourier correction
  k0dzr = w * dz / vB;
  for ( int ix = 0; ix < nx*ny; ix++) {
    rt = (vB*vP2D[ix] - 1.0f)*k0dzr;
    if (fabsf(rt) > 0.00001f) {
      rec[ix] *= cosf(rt) + sinf(rt)*I;
      src[ix] *= cosf(rt) - sinf(rt)*I;
    }
  }
      
  return;
}


void iso_ffd3_ud( float complex *src, float complex *rec, float *v, memIsoFFD3 *mem,
		  float w, float v0, float dz, float dx, float dy, int nx, int ny, timeFFD3 *time ) {

  int ix, iy;
  unsigned long long t1, t2;

  // x-direction processing
  // Conj
  get_rtc_(&t1);
  for ( ix=0; ix<nx*ny; ix++ ) {
    src[ix]   = conjf(src[ix]);
  }
  get_rtc_(&t2); time->tConj1 += t2-t1; get_rtc_(&t1);

  // M1
  iso_ffd3_m1(w,v,v0,mem->zav,mem->zbv,mem->zcv,mem->zdv,
	      mem->c1, mem->c2a, mem->c2b, mem->c2, mem->ssv, mem->c1i,mem->c2r,mem->c2i,mem->rnza,mem->rnzc,
	      dz, nx, ny, dx, mem->a1, mem->b1, 1 );
  get_rtc_(&t2); time->tXM1 += t2-t1; get_rtc_(&t1);
  iso_ffd3_mx_boundaryX(w, v, mem->zav, mem->zbv, mem->zcv, mem->zdv,
			mem->zbnx1v, mem->zbnxnv, mem->zdnx1v, mem->zdnxnv,
			nx, ny, dx, 1 );
  get_rtc_(&t2); time->tXBound1 += t2-t1; get_rtc_(&t1);
  for ( iy=0; iy<ny; iy++ ) {
    trisolver_ud_1D(rec+(iy*nx), src+(iy*nx), mem->zav+(iy*nx),mem->zbv+(iy*nx),mem->zcv+(iy*nx),mem->zdv+(iy*nx),
		    mem->zbnx1v[iy],mem->zbnxnv[iy],mem->zdnx1v[iy],mem->zdnxnv[iy],
		    mem->zev, nx);
  }
  get_rtc_(&t2); time->tXTri1 += t2-t1; get_rtc_(&t1);

  // M2
  iso_ffd3_m2(w,v,v0,mem->zav,mem->zbv,mem->zcv,mem->zdv,
	      mem->c2, mem->ssv, mem->c1i, mem->c2i, mem->rnza, mem->rnzc,
	      dz, nx, ny, dx, mem->a2, 1  );
  get_rtc_(&t2); time->tXM2 += t2-t1; get_rtc_(&t1);
  iso_ffd3_mx_boundaryX(w, v, mem->zav, mem->zbv, mem->zcv, mem->zdv,
			mem->zbnx1v, mem->zbnxnv, mem->zdnx1v, mem->zdnxnv,
			nx, ny, dx, 1 );
  get_rtc_(&t2); time->tXBound2 += t2-t1; get_rtc_(&t1);
  for ( iy=0; iy<ny; iy++ ) {
    trisolver_ud_1D(rec+(iy*nx), src+(iy*nx), mem->zav+(iy*nx),mem->zbv+(iy*nx),mem->zcv+(iy*nx),mem->zdv+(iy*nx),
		    mem->zbnx1v[iy],mem->zbnxnv[iy],mem->zdnx1v[iy],mem->zdnxnv[iy],
		    mem->zev, nx);
  }
  get_rtc_(&t2); time->tXTri2 += t2-t1; get_rtc_(&t1);

  // y-direction processing
  // M1
  iso_ffd3_m1(w, v, v0, mem->zav, mem->zbv, mem->zcv, mem->zdv,
	      mem->c1, mem->c2a, mem->c2b, mem->c2, mem->ssv, mem->c1i, mem->c2r, mem->c2i, mem->rnza, mem->rnzc,
	      dz, nx, ny, dy, mem->a1, mem->b1, 1  );
  get_rtc_(&t2); time->tYM1 += t2-t1; get_rtc_(&t1);

  iso_ffd3_mx_boundaryY(w, v, mem->zav, mem->zbv, mem->zcv, mem->zdv,
			mem->zbnx1v, mem->zbnxnv, mem->zdnx1v, mem->zdnxnv,
			nx, ny, dy, 1 );
  get_rtc_(&t2); time->tYBound1 += t2-t1; get_rtc_(&t1);

  trisolver_ud_2D(rec, src, mem->zav, mem->zbv, mem->zcv, mem->zdv,
		  mem->zbnx1v, mem->zbnxnv, mem->zdnx1v, mem->zdnxnv,
		  mem->zev, mem->zfv, mem->zfv2, mem->zcc, ny, nx);
  get_rtc_(&t2); time->tYTri1 += t2-t1; get_rtc_(&t1);

  // M2
  iso_ffd3_m2(w, v, v0, mem->zav, mem->zbv, mem->zcv, mem->zdv,
	      mem->c2, mem->ssv, mem->c1i, mem->c2i, mem->rnza, mem->rnzc,
	      dz, nx, ny, dy, mem->a2, 1  );
  get_rtc_(&t2); time->tYM2 += t2-t1; get_rtc_(&t1);
  iso_ffd3_mx_boundaryY(w, v, mem->zav, mem->zbv, mem->zcv, mem->zdv,
			mem->zbnx1v, mem->zbnxnv, mem->zdnx1v, mem->zdnxnv,
			nx, ny, dy, 1 );
  get_rtc_(&t2); time->tYBound2 += t2-t1; get_rtc_(&t1);
  trisolver_ud_2D(rec, src, mem->zav, mem->zbv, mem->zcv, mem->zdv,
		  mem->zbnx1v, mem->zbnxnv, mem->zdnx1v, mem->zdnxnv,
		  mem->zev, mem->zfv, mem->zfv2, mem->zcc, ny, nx);
  get_rtc_(&t2); time->tYTri2 += t2-t1; get_rtc_(&t1);

  // Conj
  for ( ix=0; ix<nx*ny; ix++ ) {
    src[ix]   = conjf(src[ix]);
  }
  get_rtc_(&t2); time->tConj2 += t2-t1;

  return;
}


void iso_ffd3_co( float *v, float v0, int nx, int ny, float *a1, float*b1, float *a2 ) {

  int ix, iy;
  float p, pp, ppp, pppp, p2, p3;

  for ( iy=0; iy<ny; iy++ ) {
    for ( ix=0; ix<nx; ix++ ) {
      p = v0*v[iy*nx+ix];
      pp = p*p;
      ppp = pp*p;
      pppp = ppp*p;
      p2 = 1.0f+p+pp;
      p3 = p2+ppp+pppp;
      a1[iy*nx+ix] = 4.0f*p3/(p2*p2);
      b1[iy*nx+ix] = 2.0f*p3*p3/(p2*p2*p2);
      a2[iy*nx+ix] = 1.0f/(0.5f-0.25f*p2*p2/p3);
    }
  }

  return;
}


void iso_ffd3_m1( float wl, float *v, float v0,
		  float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
		  float *c1, float *c2a, float *c2b, float *c2, float *ssv, 
		  float *c1i, float *c2r, float *c2i, float *rnza, float *rnzc,
		  float dzm, int nx, int ny, float dx, float *a1, float *b1, int iupd ) {


  float eta, beta, eps, dz, xupd, den, alpha;
  float c2aa, c2bb;
  int ix, iy;

  //  Constants
  beta = 0.101f;
  eps = 3.1415926f;
  xupd = (float)iupd;
  dz = dzm * xupd;
  eta = eps * xupd;
  den = wl*wl+eta*eta;
  alpha = wl*dx*dx/dz;
  c2aa = eta/(dz*den);
  c2bb = wl/(dz*den);

  //
  //  calculate the tri-d elements
  //
  for ( iy=0; iy<ny; iy++ ) {
    for ( ix=0; ix<nx; ix++ ) {
      c1[ix]   = 1.0f/a1[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c2a[ix]  = c2aa*b1[iy*nx+ix]*c1[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c2b[ix]  = c2bb*b1[iy*nx+ix]*c1[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c2[ix]   = 0.5f*c1[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      ssv[ix]  = 1.0f - v0*v[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c1i[ix]  = alpha*v[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c2r[ix]  = c2a[ix]/v[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c2i[ix]  = c2b[ix]/v[iy*nx+ix]+c1i[ix]*beta;
    }
    for ( ix=0; ix<nx; ix++ ) {
      rnza[ix] = c2r[ix]+c2[ix]*ssv[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      rnzc[ix] = c2r[ix]-c2[ix]*ssv[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      zav[iy*nx+ix] = rnza[ix] + c2i[ix]*I;
    }
    for ( ix=0; ix<nx; ix++ ) {
      zcv[iy*nx+ix] = rnzc[ix] + c2i[ix]*I;
    }
    for ( ix=0; ix<nx; ix++ ) {
      zbv[iy*nx+ix] = c1i[ix]*I - 2.0f*zav[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      zdv[iy*nx+ix] = c1i[ix]*I - 2.0f*zcv[iy*nx+ix];
    }
  }

  return;
}


void iso_ffd3_m2( float wl, float *v, float v0,
		  float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
		  float *c2, float *ssv, float *c1i, float *c2i, float *rnza, float *rnzc,
		  float dzm, int nx, int ny, float dx, float *a2, int iupd ) {
      
  float beta, dz, xupd, alpha;
  int ix, iy;
     
  //  Constants
  beta = 0.101f;
  xupd = (float)iupd;
  dz = dzm * xupd;
  alpha = wl*dx*dx/dz;


  //  calculate the tri-d elements
  for ( iy=0; iy<ny; iy++ ) {
    for ( ix=0; ix<nx; ix++ ) {
      c2[ix]   = 0.5f/a2[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      ssv[ix]  = 1.0f - v0*v[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c1i[ix]  = alpha*v[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      c2i[ix]  = c1i[ix]*beta;
    }
    for ( ix=0; ix<nx; ix++ ) {
      rnza[ix] =  c2[ix]*ssv[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      rnzc[ix] = -c2[ix]*ssv[ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      zav[iy*nx+ix] = rnza[ix] + c2i[ix]*I;
    }
    for ( ix=0; ix<nx; ix++ ) {
      zcv[iy*nx+ix] = rnzc[ix] + c2i[ix]*I;
    }
    for ( ix=0; ix<nx; ix++ ) {
      zbv[iy*nx+ix] = c1i[ix]*I - 2.0f*zav[iy*nx+ix];
    }
    for ( ix=0; ix<nx; ix++ ) {
      zdv[iy*nx+ix] = c1i[ix]*I - 2.0f*zcv[iy*nx+ix];
    }
  }
      
  return;
}



void iso_ffd3_mx_boundaryX( float wl, float *v,
			    float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
			    float complex *zbnx1v, float complex *zbnxnv, float complex *zdnx1v, float complex *zdnxnv,
			    int nx, int ny, float dx, int iupd ) {
  
  float xupd, dx4, c1, c2;
  float complex zcc;
  int iy;

  //  Constants
  xupd = (float)iupd;
  dx4 = 0.25f * dx;
 
  //
  //  absorbing boundary conditions ********
  //
  for ( iy=0; iy<ny; iy++ ) {
    c1         = xupd * wl/v[iy*nx  ]*dx4;
    c2         = c1*c1;
    zcc        = (1.0f-c2 + 2.0f*c1*I)/(1.0f+c2);
    zbnx1v[iy] = zbv[iy*nx  ]+zcc*zav[iy*nx  ];
    zdnx1v[iy] = zdv[iy*nx  ]+zcc*zcv[iy*nx  ];
      
    c1         = xupd * wl/v[iy*nx+ny-1]*dx4;
    c2         = c1*c1;
    zcc        = (1.0f-c2 + 2.0f*c1*I)/(1.0f+c2);
    zbnxnv[iy] = zbv[iy*nx+ny-1]+zcc*zav[iy*nx+ny-1];
    zdnxnv[iy] = zdv[iy*nx+ny-1]+zcc*zcv[iy*nx+ny-1];
  }

  return;
}


void iso_ffd3_mx_boundaryY( float wl, float *v,
			    float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
			    float complex *zbnx1v, float complex *zbnxnv, float complex *zdnx1v, float complex *zdnxnv,
			    int nx, int ny, float dx, int iupd ) {
  
  float xupd, dx4, c1, c2;
  float complex zcc;
  int ix;

  //  Constants
  xupd = (float)iupd;
  dx4 = 0.25f * dx;
 
  //
  //  absorbing boundary conditions ********
  //
  for ( ix=0; ix<nx; ix++ ) {
    c1         = xupd * wl*v[ix]*dx4;
    c2         = c1*c1;
    zcc        = (1.0f-c2 + 2.0f*c1*I)/(1.0f+c2);
    zbnx1v[ix] = zbv[ix]+zcc*zav[ix];
    zdnx1v[ix] = zdv[ix]+zcc*zcv[ix];
  }
  
  for ( ix=0; ix<nx; ix++ ) {
    c1         = xupd * wl*v[(ny-1)*nx+ix]*dx4;
    c2         = c1*c1;
    zcc        = (1.0f-c2 + 2.0f*c1*I)/(1.0f+c2);
    zbnxnv[ix] = zbv[(ny-1)*nx+ix]+zcc*zav[(ny-1)*nx+ix];
    zdnxnv[ix] = zdv[(ny-1)*nx+ix]+zcc*zcv[(ny-1)*nx+ix];
  }
  
  return;
}



void trisolver_ud_2D( float complex *p, float complex *p2, 
		      float complex *za, float complex *zb, float complex *zaa, float complex *zbb,
		      float complex *zbnx1, float complex *zbnxn, float complex *zbbnx1, float complex *zbbnxn,
		      float complex *zev, float complex *zfv, float complex *zfv2, float complex *den,
		      int nx, int ny) {

  int ix, iy;
  
  // (A)  FORWARD
  //
  //     ix=0
  //
  for ( iy=0; iy<ny; iy++ ) {
    zev[iy] = -za[ny+iy]/zbnx1[iy];
  }
  for ( iy=0; iy<ny; iy++ ) {
    zfv [iy] = (zbbnx1[iy]*p [iy]+zaa[nx+iy]*p [nx+iy])/zbnx1[iy];
  }
  for ( iy=0; iy<ny; iy++ ) {
    zfv2[iy] = (zbbnx1[iy]*p2[iy]+zaa[ny+iy]*p2[nx+iy])/zbnx1[iy];
  }

  //
  //     ix=1
  //
  for ( iy=0; iy<ny; iy++ ) {
    den[iy]=1.0f/(zb[nx+iy]+za[iy]*zev[iy]);
  }
  for ( iy=0; iy<ny; iy++ ) {
    zev[nx+iy]=-za[2*nx+iy]*den[iy];
  }
  for ( iy=0; iy<ny; iy++ ) {
    p [iy]=((zaa[iy]*p [iy]+zbb[nx+iy]*p [nx+iy] + zaa[2*nx+iy]*p [2*nx+iy])-za[iy]*zfv [iy])*den[iy];
  }
  for ( iy=0; iy<ny; iy++ ) {
    p2[iy]=((zaa[iy]*p2[iy]+zbb[nx+iy]*p2[nx+iy] + zaa[2*nx+iy]*p2[2*nx+iy])-za[iy]*zfv2[iy])*den[iy];
  }
	

  //
  //     ix>1
  //
  int nx1 = nx-2;
  int ixm1, ixp1;
  for ( ix=2; ix<=nx1; ix++ ) {
    ixm1=ix-1;
    ixp1=ix+1;
    for ( iy=0; iy<ny; iy++ ) {
      den[iy]=1.0f/(zb[ix*ny+iy]+za[ixm1*ny+iy]*zev[ixm1*ny+iy]);
    }
    for ( iy=0; iy<ny; iy++ ) {
      zev[ix*ny+iy]=-za[ixp1*ny+iy]*den[iy];
    }
    for ( iy=0; iy<ny; iy++ ) {
      p [ixm1*ny+iy]=((zaa[ixm1*ny+iy]*p [ixm1*ny+iy]+zbb[ix*ny+iy]*p [ix*ny+iy]
		       +zaa[ixp1*ny+iy]*p [ixp1*ny+iy])-za[ixm1*ny+iy]*p [(ixm1-1)*ny+iy])*den[iy];
    }
    for ( iy=0; iy<ny; iy++ ) {
      p2[ixm1*ny+iy]=((zaa[ixm1*ny+iy]*p2[ixm1*ny+iy]+zbb[ix*ny+iy]*p2[ix*ny+iy]
		       +zaa[ixp1*ny+iy]*p2[ixp1*ny+iy])-za[ixm1*ny+iy]*p2[(ixm1-1)*ny+iy])*den[iy];
    }
  }

  //
  //     (B)  LAST ELEMENT      ix=nx-1
  //
  for ( iy=0; iy<ny; iy++ ) {
    p [(nx-1)*ny+iy]=( zbbnxn[iy]*p [(nx-1)*ny+iy]+zaa[nx1*ny+iy]*p [nx1*ny+iy]
		       -za[nx1*ny+iy]*p [(nx1-1)*ny+iy] )/(zbnxn[iy]+za[nx1*ny+iy]*zev[nx1*ny+iy]);
  }
  for ( iy=0; iy<ny; iy++ ) {
    p2[(nx-1)*ny+iy]=( zbbnxn[iy]*p2[(nx-1)*ny+iy]+zaa[nx1*ny+iy]*p2[nx1*ny+iy]
		       -za[nx1*ny+iy]*p2[(nx1-1)*ny+iy] )/(zbnxn[iy]+za[nx1*ny+iy]*zev[nx1*ny+iy]);
  }
  
  //
  //     (C)  BACKWARD
  //
  for ( ix=nx1; ix>=1; ix-- ) {
    ixm1=ix-1;
    ixp1=ix+1;
    for ( iy=0; iy<ny; iy++ ) {
      p [ix*ny+iy]=zev[ix*ny+iy]*p [ixp1*ny+iy]+p [ixm1*ny+iy];
    }
    for ( iy=0; iy<ny; iy++ ) {
      p2[ix*ny+iy]=zev[ix*ny+iy]*p2[ixp1*ny+iy]+p2[ixm1*ny+iy];
    }
  }

  //
  //     ix=0
  //
  for ( iy=0; iy<ny; iy++ ) {
    p [iy]=zev[iy]*p [ny+iy]+zfv [iy];
  }
  for ( iy=0; iy<ny; iy++ ) {
    p2[iy]=zev[iy]*p2[ny+iy]+zfv2[iy];
  }
  
  return;
  
}


void trisolver_ud_1D(  float complex *p, float complex *p2, 
		       float complex *za, float complex *zb, float complex *zaa, float complex *zbb,
		       float complex zbnx1, float complex zbnxn, float complex zbbnx1, float complex zbbnxn,
		       float complex *ze, int nx ) {

  int ix;
  float complex zf, zf2, den;


  // (A)  FORWARD
  //
  //     ix=0
  //
  ze[0] = -za[1]/zbnx1;
  zf    = (zbbnx1*p [0]+zaa[1]*p [1])/zbnx1;
  zf2   = (zbbnx1*p2[0]+zaa[1]*p2[1])/zbnx1;

  //
  //     ix=1
  //
  den=1.0f/(zb[1]+za[0]*ze[0]);
  ze[1]=-za[2]*den;
  p [0]=((zaa[0]*p [0]+zbb[1]*p [1] + zaa[2]*p [2])-za[0]*zf )*den;
  p2[0]=((zaa[0]*p2[0]+zbb[1]*p2[1] + zaa[2]*p2[2])-za[0]*zf2)*den;


  //
  //     ix>1
  //
  int nx1 = nx-2;
  int ixm1, ixp1;
  for ( ix=2; ix<=nx1; ix++ ) {
    ixm1=ix-1;
    ixp1=ix+1;
    den=1.0f/(zb[ix]+za[ixm1]*ze[ixm1]);
    ze[ix]=-za[ixp1]*den;
    p [ixm1]=((zaa[ixm1]*p [ixm1]+zbb[ix]*p [ix]
	       +   zaa[ixp1]*p [ixp1])-za[ixm1]*p [ixm1-1])*den;
    p2[ixm1]=((zaa[ixm1]*p2[ixm1]+zbb[ix]*p2[ix]
	       +   zaa[ixp1]*p2[ixp1])-za[ixm1]*p2[ixm1-1])*den;
  }

  //
  //     (B)  LAST ELEMENT      ix=nx-1
  //
  p [nx-1]=( zbbnxn*p [nx-1]+zaa[nx1]*p [nx1]
	     -za[nx1]*p [nx1-1] )/(zbnxn+za[nx1]*ze[nx1]);
  p2[nx-1]=( zbbnxn*p2[nx-1]+zaa[nx1]*p2[nx1]
	     -za[nx1]*p2[nx1-1] )/(zbnxn+za[nx1]*ze[nx1]);

  //
  //     (C)  BACKWARD
  //
  for ( ix=nx1; ix>=1; ix-- ) {
    ixm1=ix-1;
    ixp1=ix+1;
    p [ix]=ze[ix]*p [ixp1]+p [ixm1];
    p2[ix]=ze[ix]*p2[ixp1]+p2[ixm1];
  }

  //
  //     ix=0
  //
  p [0]=ze[0]*p [1]+zf ;
  p2[0]=ze[0]*p2[1]+zf2;

  return;
}



void getWavenumbers(int nx, float dx, float *kx2)
{
  int  nxhalf, ix;
  float dkx;

  nxhalf = nx/2;
  dkx=2.0f*(float)PI/((float)nx*dx);

  kx2[0]=0.0;

  for (ix=1; ix<=nxhalf; ix++) {
    kx2[ix]  = (ix*dkx) * (ix*dkx) ;
  }

  for (ix=nx-1; ix>nxhalf; ix--) {
    kx2[ix]=kx2[nx-ix];
  }
  
  //for (ix=0; ix<nx; ix++) fprintf(stderr, "%f ", kx2[ix]); fprintf(stderr, "\n");

  return;
}


void getTaperXY(float **taperXY, int nxf,int nyf,int nx,int ny, float alpha, int border) {

  int ix, iy;


  // Set everything to zero
  for ( ix = 0; ix < nxf*nyf; ix++ ) {
    taperXY[0][ix] = 0.0f;
  }
    
  // Set image domain to one
  for ( iy = 0; iy < ny; iy++ ) {
    for ( ix = 0; ix < nx; ix++ ) {
      taperXY[iy][ix] = 1.0f;
    }
  }

  // Taper edge of image domain
  for ( iy = 0; iy < border; iy++ ) {
    for ( ix = 0; ix < nx; ix++ ) {
      taperXY[iy][ix] = expf(-powf(alpha*(float)(iy-border),2.0f));
    }
  }
  for ( iy = ny-border-1; iy < ny; iy++ ) {
    for ( ix = 0; ix < nx; ix++ ) {
      taperXY[iy][ix] = expf(-powf(alpha*(float)(ny-border-1-iy),2.0f));
    }
  }

  for ( ix = 0; ix < border; ix++ ) {
    for ( iy = 0; iy < ny; iy++ ) {
      taperXY[iy][ix] *= expf(-powf(alpha*(float)(ix-border),2.0f));
    }
  }
  for ( ix = nx-border-1; ix < nx; ix++ ) {
    for ( iy = 0; iy < ny; iy++ ) {
      taperXY[iy][ix] *= expf(-powf(alpha*(float)(nx-border-1-ix),2.0f));
    }
  }

  return;

}
