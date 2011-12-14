/***************************************************************************
                          tracerbase.h  -  description
*/

/** 
    Base class for full tracer application classes.
    Provides the initialization of the property models.
**/

/*                           -------------------
    begin                : Sun Nov 01 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fraunhofer.de

     change log:

       merten | 2010-01-14
                changed to template with parameter TracingOperator_T

       merten | 2009-11-05
                new variables for TTI added

***************************************************************************/


#ifndef TRACERBASE_H
#define TRACERBASE_H

// user defined includes
#include "include/defs.h"
#include "structures/tracingjob.h"
#include "utils/propmodel.h"
#include "utils/velgridpointlinear.h"
#include "utils/velgridpointbspline.h"
// standard includes
//#include <stdlib.h>



/**
  *@author Dirk Merten
  */

/** 
 * @ingroup group2
 * @brief Base class for the tracer API classes.
 */

template<class TracingOperator_T> class TracerBase {

 private:
    // provide access to velocity representation and model type of TracingOperator_T
    // defined here private and not protected due to compiler problems
    // with inerited typedefs
    typedef typename TracingOperator_T::VelModelRep_t VelModelRep_t;
    typedef typename TracingOperator_T::VMODEL_T VMODEL_T;
    typedef typename TracingOperator_T::VREPR_T VREPR_T;

// public methods
 public:
  /**
   * \brief Empty constructor
   * \details All property models are set to NULL and the TracingOperator is
   *          initialized with some standard values.
   *          This constructor is useful if some class functionality is needed
   *          to decide if the property models can or should be loaded at all.
   */
  TracerBase();

  ~TracerBase();

  void BindVelModel( const VelModelRep_t& VModels){ StepTracer.BindModels(VModels);};

  /**
   * \brief Check for the validity of the TTT grid defined in the Job
   * \details The positions of ray tracing sources are compared to the 
   *          ray tracing volume specified in Job.X0Vol and Job.X1Vol.
   *          An appropriate error message if some or any sources are
   *          outside will be issued.
   * \param Job General tracing job description
   * \return false if all sources are outside, true otherwise
   */
  bool CheckTTGrid(const TracingJob& Job) const;

// public attributes
 public:

// protected methods
 protected:
  /**
   * \brief Initialize the RayTracer with the global variables of the TracingJob.
   * \details The property models are initialized with the files and dimensions
   *          given in the job description. This includes the full read of all
   *          input files necessary.
   * \param Job General tracing job description
   * \param ierr Error flag
   */
  TracerBase(const TracingJob& Job, int& ierr);
  
  /// Initialize the Source with indicees ix, iy from the job description.
  int InitSource(WFPtSrc*& Source, const TracingJob& Job, const int& ix, const int& iy , const int& iz, const int ith );

// protected attributes
 protected:
  /// Operator that traces a ray or a bundle of rays for one step in time.
  TracingOperator_T StepTracer;

// private methods
 private:
    TracerBase(const TracerBase& T);
    
    TracerBase& operator = (const TracerBase& T);

// private attributes
 private:

  
};

#include "tracerbase.cpp"

#endif
