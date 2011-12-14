/***************************************************************************
                          raytracer.h  -  description

    The RayTracer class provides the main API for the Seismic Ray Tracer.  

                             -------------------
    begin                : Thu Jan 24 2007
    copyright            : (C) 2007 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

       merten | 2010-01-14
                changed to template with parameter TracingOperator_T

       merten | 2009-11-05
                base class TracerBase added;
                static member function for threaded start-up of run added

***************************************************************************/


#ifndef RAYTRACER_H
#define RAYTRACER_H

#define DIMENSION 100

/**
 The RayTracer class provides the main API for the Seismic Ray Tracer.  

 *@author Dirk Merten
 *@copyright ITWM
 */

#include "include/types.h"

#include "include/defs.h"
#include "memory/memman3d_malloc.h"
#include "memory/memman3d_vector.h"
#include "memory/memman3d_stlvector.h"
#include "memory/memman3d_bucket.h"
#include "structures/vti_velocity.h"
#include "structures/angsig.h"
#include "structures/tracingjob.h"
#include "filehandler/ttangfilehandler.h"
#include "utils/Acq_geometry.hpp"
#include "utils/progressbar.h"
#include "utils/timer.h"
#include "ray3d.h"
#include "wfptsrc.h"
#include "boundaryoperator.h"
#include "boundarytubeoperator.h"
#include "reflectionoperator.h"
#include "observeroperator.h"
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
//#define TIMING

/** 
 * @ingroup group2
 * @brief The RayTracer class provides the main API for the Seismic Ray Tracer.
 */

template<class TracingOperator_T> class RayTracer : public TracerBase<TracingOperator_T>{

  // helper structure for the threaded start of RayTracer::run
  typedef struct {
      RayTracer* Object;
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
  RayTracer():TracerBase<TracingOperator_T>(){};

  RayTracer(const TracingJob& Job, int& ierr):TracerBase<TracingOperator_T>(Job, ierr){};

  ~RayTracer(){};

  /**
   * \brief   Parallel and threaded execution of the job.
   * \details This is the main user routine for a job execution.
   *          According to the parallel environment passed through PE the 
   *          job is split into sub jobs consisting of one single shot.
   *          Each thread executes one of these sub jobs using the
   *          member function run described below.
   * \param Job General tracing job description
   * \param PE A link to the parallel environment
   * \return error flag
   */
  int execute(const TracingJob& Job, ParallelEnvironment* PE);

  //! Run the RayTracer on a Source and Receiver grid defined in the TracingJob.
  /*! Run the RayTracer on a Source and Receiver grid defined in the TracingJob. */
  void run(const TracingJob& Job, ProgressBar* progress, const int PRank = 0, const int ith = 0);

  /**
   * \brief   Return amount of memory needed for this job
   * \details The memory is mainly used for the representation of the 
   *          velocity and property models. 
   *          The value returned is rather an estimation than an exact number.
   * \param Job General tracing job description
   * \return the amount of memory
   */
  unsigned long GetMemoryNeeded(const TracingJob& Job) const;

// public attributes
 public: 

// private methods
 private:

  /// Initialize the Rays and Triangles from the Source values.
  int InitRays(rayMem& rayArray, WFPtSrc* Source) const;

  /// Wrapper function for threaded execution of run
  static void* run_ThreadStartup(void *_tgtObject);

// private attributes
 private:

  TTAngFileHandler TTAFH;
#ifdef DEBUG_CHECK
  CheckClass Checker;
#endif

  
};

#include "raytracer.cpp"

#endif
