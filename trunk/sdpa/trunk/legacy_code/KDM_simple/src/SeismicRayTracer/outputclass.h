/***************************************************************************
                          outputclass.h  -  description
                             -------------------
    begin                : Wed Jan 25 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef OUTPUTCLASS_H
#define OUTPUTCLASS_H


/**
  *@author Dirk Merten
  */
#include "structures/angsig.h"
#include "receivergrid.h"
#include "utils/propmodel.h"
#include "structures/grid3d.h"
#include "include/types.h"
#include "structures/tracingjob.h"
#include "filehandler/SegYHeader.h"



#include <iostream>
#include <fstream>

class OutputClass {
public: 

  OutputClass();
  ~OutputClass();
  /** No descriptions */
  void PrintRcv(const ReceiverGrid& RcvGrid, const point3D<float>& X0, const point3D<float>& X1);
  void WriteRcv(const ReceiverGrid& RcvGrid, const int& SrcX, const int& SrcY);
  void WriteAngSig(const AngSig* Signals, const int& n, const int& idir);
  void WriteAngSig(const AngSig* Signals, const int& n, const int& SrcX, const int& SrcY, const int& SrcZ);
  void TouchAngSig(const int& SrcX, const int& SrcY, const int& SrcZ,
		   const int& NSrcY);
  void WriteAngSig(const AngSig* Signals, const int& n, const int& SrcX, const int& SrcY, const int& SrcZ,
		   const int& NSrcY);
  void WriteRcv(const ReceiverGrid& RcvGrid, const char* FName);
  void WriteRcvSegY(const ReceiverGrid& RcvGrid, const char* FName);
  void WriteDim(const grid3D& SrcGrid, const ReceiverGrid& RcvGrid);
  void PrintVelocityField(PropModel<VelGridPointBSpline>& VF);
  void PrintTriangulation(triMem& TList, rayMem& rArray, int n, int Tstep = -1);
  void PrintRay(ray3D* ray);
  void PrintJob(const TracingJob& Job);
  //int SetDirectory(const char* DirName, const char* PreFix);

// private member variables
 private:
/// Prefix of TTT files.
    char* PreFileName;

    
};

#endif
