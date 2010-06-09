/***************************************************************************
                          migtypes.h  -  description
                             -------------------
    begin                : Wed Apr 5 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#ifndef MIGTYPES_H
#define MIGTYPES_H
 
enum GATHER_MODE {ALL, COMMON_OFFSET, COMMON_CDP};

enum FILE_MODE {SEGY_BIGENDIAN = 0, 
//		SEGY_LITENDIAN = 1, deprecated 
		SU_LITENDIAN = 2,  
		RAW_FLOAT = 3, 
		SEGY_BIGENDIAN_SXFLOAT = 4, 
		SVF = 5,
		UNDEFINED_FILE_MODE = 99};

enum FILE_ORDER {CDP_GATHER, SHOT_GATHER, IRREGULAR};

enum IO_MODE {EACHNODE, IONODE_SOCKET, IONODE_VM};

#define QCOBS 15
#define QCOBS_V 0
#define QCOBS_DANG 1
#define QCOBS_NTOT 2
#define QCOBS_RANK 3
#define QCOBS_MEANRAY 4
#define QCOBS_ILDIP 5
#define QCOBS_XLDIP 6
#define QCOBS_VECM1 7
#define QCOBS_VECM2 8
#define QCOBS_VECM3 9
#define QCOBS_VTIEPSILON 10
#define QCOBS_VTIDELTA 11
#define QCOBS_DIRCNT 12

#endif
