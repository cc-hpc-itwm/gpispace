/***************************************************************************
                          wavefronttracer.h  -  description
                             -------------------
    begin                : Thu Apr 6 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-14
                changed to template with parameter TracingOperator_T

       merten | 2010-01-12
                static member function for threaded start-up of run added

       merten | 2009-11-05
                base class TracerBase added;

      micheld | 2009-08-07
		parameter controlling rays per source changed from g_RAY_MIN to g_InitAngle
		all calls of WFPtSrc now use "initial angle discretization" instead of "minimal ray number"
***************************************************************************/


#ifndef WAVEFRONTTRACER_H
#define WAVEFRONTTRACER_H

#define DIMENSION 100

/**
 The WaveFrontTrace class provides the main API for the Wave Front Tracer.  

 *@author Dirk Merten
 *@copyright ITWM
 */
#include "include/types.h"
#include "include/defs.h"
#include "structures/vti_velocity.h"
#include "utils/progressbar.h"
#include "utils/timer.h"
#include "memory/memman3d_malloc.h"
#include "memory/memman3d_vector.h"
#include "memory/memman3d_stlvector.h"
#include "memory/memman3d_bucket.h"
#include "filehandler/ttfilehandler.h"
#include "ray3d.h"
#include "triray3d.h"
#include "wfptsrc.h"
#include "boundaryoperator.h"
#include "boundarytubeoperator.h"
#include "refinementoperator.h"
#include "reflectionoperator.h"
#include "observeroperator.h"
#include "receivergrid.h"
#include "receiver.h"
#include "structures/tracingjob.h"

#include "checkclass.h"
#include "outputclass.h"
#include "tracerbase.h"

#ifdef PARALLEL_VM
#include "utils/VM_ParallelEnvironment.h"
#endif
#ifdef PARALLEL_DUMMY
#include "3D-GRT/Dummy_ParallelEnvironment.h"
#endif



#include <iostream>
#include <stdlib.h>
#include <unistd.h>



//#define DEBUG_CHECK
//#define OUTPUT_TRIANG
//#define TIMING

template<class TracingOperator_T> class WaveFrontTracer : public TracerBase<TracingOperator_T> {

  // helper structure for the threaded start of WaveFrontTracer::run
  typedef struct {
    WaveFrontTracer<TracingOperator_T>* Object;
    TracingJob* Job;
    bool active;
    ProgressBar* progress;
    int rank;  
    int tid;  
  } thread_parm_t;

 private:
  // provide access to velocity representation of TracingOperator_T
  // defined here and not in base class due to compiler problems
  // with inerited typedefs
  typedef typename TracingOperator_T::VREPR_T VREPR_T;

// public methods
 public: 
  /// Initialize the WaveFrontTracer with the global variables of the TracingJob.
  WaveFrontTracer(const TracingJob& Job, int& ierr):TracerBase<TracingOperator_T>(Job, ierr){};
  WaveFrontTracer(){};

  ~WaveFrontTracer(){};

  void SetDirectory(const char* DirName, const char* PreFix) { TTFH.SetDirectory(DirName, PreFix); };

  void execute(const TracingJob& Job, ParallelEnvironment* PE, int& ierr);
  //! Run the WaveFrontTracer on a Source and Receiver grid defined in the TracingJob.
  /*! Run the WaveFrontTracer on a Source and Receiver grid defined in the TracingJob. */
  void run(const TracingJob& Job, ProgressBar* progress, const int PRank = 0, const int ith = 0);
  /// Run the WaveFrontTracer for one source, store it if necessary, and write traveltime tables immediately into a given memory position.
  void run_one(const TracingJob& Job, const int iSrc, const int jSrc, const int kSrc, bool store2file, char* mem_ptr, const int PRank, const int ith);
  /// Run the WaveFrontTracer on one Source and a Receiver grid defined in the TracingJob.
  void run_1s(const TracingJob& Job, RecSig* Signals);
  /// Run the WaveFrontTracer on one Source and one Receiver defined in the TracingJob.
  float run_1s1r(const TracingJob& Job);
  void run_rays(const TracingJob& Job);

  void shootwf(const TracingJob& Job,
	       const point3D<float> SrcPoint, const point3D<float>& direction, const float& aperture,
	       const grid3D RcvGrid,
	       float* tt, Spherical* dir, const int ith = 0);

  void shootwf(const TracingJob& Job,
	       const point3D<float> SrcPoint, const point3D<float>& direction, const float& aperture,
	       const int nRcv, Receiver* Rcv,
	       float* tt, Spherical* dir, point3D<float>* dirVector, const int ith = 0);

  void shootwf(const TracingJob& Job,
	       const point3D<float> SrcPoint, const point3D<float>& direction, const float& aperture,
	       const int nRcv, point3D<float>* Rcv,
	       float* tt, Spherical* dir, point3D<float>* dirVector, const int ith = 0);

  /// Shoot one ray from SrcPoint in direction and store positions in r
  void shootray(const point3D<float> SrcPoint, const Spherical& direction, 
		const float& dt, const int& Nt,
		point3D<float>* r);

// private methods
 private:
//   WaveFrontTracer(){};


  /// Initialize the Rays and Triangles from the Source values.
  void InitRays(trirayMem& rayArray, triMem& triList, WFPtSrc* Source);

  void tracewf(const TracingJob& Job, 
	       WFPtSrc* Source, 
	       ReceiverGrid* RcvGrid,
	       ObserverOperator<TracingOperator_T>& Observer,
	       BoundaryOperator& Outside,
	       const int ith = 0);
 
  void tracewf(const TracingJob& Job, 
	       WFPtSrc* Source, 
	       const int nRcv,
	       Receiver* Rcv,
	       ObserverOperator<TracingOperator_T>& Observer,
	       BoundaryOperator& Outside,
	       const int ith = 0);

  /// Wrapper function for threaded execution of run
  static void* run_ThreadStartup(void *_tgtObject);

  /// 3-dimensional grid of receivers in the subsurface
  //ReceiverGrid* RcvGrid;
  /// Accoustic Source
  //WFPtSrc* Source;

// private attributes
 private:
  /// Operator that refines, splits, etc. the triangulation of the wave front.
  //RefinementOperator* SubDivide;
  /// Operator that projects observables on the wave front into the receivers.
  //ObserverOperator Observer;
  OutputClass Output;
  TTFileHandler TTFH;

  

#ifdef DEBUG_CHECK
  CheckClass Checker;
#endif
};

#include "wavefronttracer.cpp"

#endif
