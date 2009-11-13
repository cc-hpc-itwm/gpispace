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
#include <list>
#include <map>

#include <gwdl/memory.hpp>
#include <gwdl/IToken.h>

namespace gwdl
{
  class Place;

  typedef std::list<IToken*> token_list_t;
  typedef std::map<std::string, token_list_t> workflow_result_t;

  class IWorkflow
  {
  public:
    typedef gwdl::shared_ptr<IWorkflow> ptr_t;
    typedef std::string workflow_id_t;

    virtual const workflow_id_t &getID() const = 0;
    virtual void setID(const workflow_id_t &id) = 0;

    virtual Place* getPlace(const std::string& id) = 0;

    // one has to explicitly deallocate the memory!
    // all tokens contained in the result are copies!
    virtual workflow_result_t getResults() const = 0;
  };

  // convenience deallocation method
  inline void deallocate_workflow_result(workflow_result_t &result)
  {
    for (workflow_result_t::iterator place(result.begin()); place != result.end(); ++place)
    {
      for (token_list_t::iterator token(place->second.begin()); token != place->second.end(); ++token)
      {
        delete (*token);
      }
      place->second.clear();
    }
    result.clear();
  }
}

#endif
