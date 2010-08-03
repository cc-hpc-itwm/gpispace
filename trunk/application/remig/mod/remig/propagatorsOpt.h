//###################################################################
//
// Demigration Remigration optimized Propagators
//
//
// Dr. Martin Kuehn
// Fraunhofer Institut fuer Techno- und Wirtschaftsmathematik (ITWM)
// Fraunhofer-Platz 1
// D-67663 Kaiserslautern
//
//###################################################################



/**********************************************************************************
C               propagators.h
C Optimized propagators                                                        C
C                                                                              C
C WARNING: Modification from original propagators.f                            C
C                                                                              C
************************************************************************************/

#ifndef C_PROPAGATORS_OPT_H
#define C_PROPAGATORS_OPT_H


//------------------- po3d_ps_opt -----------------------------------
int po3d_ps_opt(
    float* const       u,        // complex u(nx, ny) frequency slice of wave field
    const int          nx,       //! number of grids in x-direction
    const int          ny,       //! number of grids in y-direction
    const float* const kx2,      // kx2(nx) square of wave-number array in x-direction
    const float* const ky2,      // ky2(ny) square of wave-number array in y-direction
    const float        k0,       //! reference wave-number
    const float        dz,       //! depth interval
    float* const       u1,       // temp space 4*roundUp4(nx) complex
    const int          iupd      //! up/down flag: -1=forward, +1=backword
 );

//------------------- po3_ps_calc_intervals -------------------------
void po3d_ps_calc_intervals(
    const int          nx,
    const int          ny,
    const float* const kx2,
    const float* const ky2,
    const float        k0,
    int* const         v
);

//------------------- po3D_ssf_opt ----------------------------------
int po3d_ssf_opt(
    float* const          u,   // complex   u(nx, ny)   ! frequency slice of wave field
    const int            nx,        //! number of grids in x-direction
    const int            ny,        //! number of grids in y-direction
    const float*  const kx2, //kx2(nx) wave-number array in x-direction
    const float*  const ky2, //ky2(ny) wave-number array in y-direction
    const float* const    v,  //v(nx, ny) depth slice of velocity
    const float          v0,       // reference velocity
    const float          dz,       //! depth interval
    const float           w,          // circular frequency
    float* const         u1,  // temp space 4*roundUp4(nx) complex
    const int          iupd     // up/down flag: -1=forward, +1=backword
    );

//------------------- po3D_gsp1_opt ----------------------------------
int po3d_gsp1_opt(
    float* const         u,     // u(nx, ny) frequency slice of wave field
    const int           nx,     // number of grids in x-direction
    const int           ny,     // number of grids in y-direction
    const float* const kx2,     // kx2(nx) wave-number array in x-direction
    const float* const ky2,     //real  ky2(ny) wave-number array in y-direction
    const float* const   v,     //v(nx, ny) depth slice of velocity
    const float         v0,     // reference velocity
    const float         dz,     // depth interval
    const float          w,     // circular frequency
    const float         dx,     // grid interval in x-direction
    const float         dy,     // grid interval in y-direction
    float* const        u1,     //u1(4*(maxy+3)) 1D work array
    float* const        v1,     //v1(maxy) 1D work array
    float* const        za,     //complex za(4*(maxy+2))  // 1D work array
    float* const        zb,     //complex zb(4*maxy)  // 1D work array
    float* const        zc,     //complex zc(4*(maxy+2))  // 1D work array
    float* const        zd,     //complex zd(4*maxy)  // 1D work array
    float* const        ze,     //complex ze(4*(maxy+1))  1D work array
    const int         iupd      //up/down flag: -1=forward, +1=backword
);

//------------------- po3d_gsp2_opt --------------------------------
int po3d_gsp2_opt(
    float* const         u,       //complex u(nx, ny) frequency slice of wave field
    const int           nx,       //number of grids in x-direction
    const int           ny,       //number of grids in y-direction
    const float* const kx2,       //kx2(nx) wave-number array in x-direction
    const float* const ky2,       //ky2(ny) wave-number array in y-direction
    const float* const   v,       //v(nx, ny) depth slice of velocity
    const float         v0,       //reference velocity
    const float         dz,       //depth interval
    const float          w,       //circular frequency
    const float         dx,       //grid interval in x-direction
    const float         dy,       //grid interval in y-direction
    float* const        u1,       //u1(4*(maxy+3)) 1D work array
    float* const        v1,       //v1(4*maxy) 1D work array
    float* const        za,       //za(4*(maxy+2)) 1D work array
    float* const        zb,       //zb(4*maxy) 1D work array
    float* const        zc,       //zc(4*(maxy+2)) 1D work array
    float* const        zd,       //zd(4*maxy) 1D work array
    float* const        ze,       //ze(4*(maxy+1)) 1D work array
    float* const        a1,       //real  a1(nx,ny) 2D coefficient array
    float* const        b1,       //real  b1(nx,ny) 2D coefficient array
    float* const        a2,       //a2(nx,ny) 2D coefficient array
    const int         iupd        //up/down flag: -1=forward, +1=backword
);

//------------------- po3d_gsp2_co_opt -----------------------------
int po3d_gsp2_co_opt(
    const float* const v,// v(nx,ny)
    const float       v0,
    const int         nx,
    const int         ny,
    float* const      a1,//real a1(nx,ny)
    float* const      b1,//real b1(nx,ny)
    float* const      a2 //real a2(nx,ny)
);

#endif
