/*
 * =====================================================================================
 *
 *       Filename:  IActivity.h
 *
 *    Description:  abstract interface to the activity class
 *
 *        Version:  1.0
 *        Created:  10/09/2009 12:03:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef GWES_IACTIVITY_H
#define GWES_IACTIVITY_H 1

#include <exception>
#include <string>
#include <gwes/memory.hpp>

#include <gwdl/IWorkflow.h>

namespace gwes
{
  class IActivity
  {
  public:
    typedef shared_ptr<IActivity> ptr_t;
    typedef std::string activity_id_t;

    virtual void  setID(const activity_id_t &) = 0;
    virtual const activity_id_t &getID() const = 0;

    virtual const gwdl::IWorkflow::workflow_id_t &getOwnerWorkflowID() const = 0;

    virtual gwdl::IWorkflow::ptr_t transform2Workflow() const throw(std::exception) = 0;
  };
  
  typedef IActivity activity_t;
  typedef activity_t::activity_id_t activity_id_t;
}

#endif
