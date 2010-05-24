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



//  static void unpackVectorToTracesU4( __m128* const x, float* const y,
//                                      const int nDataPoints,
//                                      const int nTraces );

#ifndef C_UTILS_OPT_H
#define C_UTILS_OPT_H

#include<xmmintrin.h>

inline int roundUp4( const int n ) { const int fPerV=4; return
((n+fPerV-1)/fPerV)*fPerV; };

inline void copyVector4( const float* const s, float* const d, const int nx ) {

  for( int i=0; i<nx; i+=sizeof(__m128)/sizeof(float) )
    _mm_store_ps( d + i, _mm_load_ps( s + i ) );

}

void matrixToBlockedFloat4( const float* const s, float* const d,
                            const int nx, const int ny, const int dimY );
void blockedToMatrixFloat4( const float* const s, float* const d,
                            const int nx, const int ny, const int dimY );
void transposeBlockedComplex4( const float* const uRow,
                               float* const uCol,
                               const int dimY,
                               const int dimX,
                               const int nx,
                               const int ny );
void transposeBlockedFloat4( const float* const uRow,
                             float* const uCol,
                             const int dimY,
                             const int dimX,
                             const int nx,
                             const int ny );
#endif
