/*
 * =====================================================================================
 *
 *       Filename:  IWorkflow.h
 *
 *    Description:  abstract interface to a workflow
 *
 *        Version:  1.0
 *        Created:  10/09/2009 11:12:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef GWDL_WORKFLOW_H
#define GWDL_WORKFLOW_H 1

#include <string>
#include <gwdl/Memory.h>

namespace gwdl
{
  class IWorkflow
  {
  public:
    typedef gwdl::shared_ptr<IWorkflow> ptr_t;
    typedef std::string workflow_id_t;

    virtual const workflow_id_t &getID() const = 0;
    virtual void setID(const workflow_id_t &id) = 0;

    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string &) = 0;
  };
}

#endif
