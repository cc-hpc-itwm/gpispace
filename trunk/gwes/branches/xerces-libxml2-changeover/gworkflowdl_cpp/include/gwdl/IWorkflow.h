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
#include <list>
#include <map>
#include <gwdl/Place.h>
#include <gwdl/Token.h>
//#include <gwdl/IToken.h>

namespace gwdl
{
  class Place;

  typedef std::list<Token::ptr_t> token_list_t;
  typedef std::map<std::string, token_list_t> workflow_result_t;

  class IWorkflow
  {
  public:
    typedef gwdl::shared_ptr<IWorkflow> ptr_t;
    typedef std::string workflow_id_t;

    virtual const workflow_id_t &getID() const = 0;
    virtual void setID(const workflow_id_t &id) = 0;

    // Use interface IBuilder.h and implementation Libxml2Builder to serialize or deserialize workflows!
    //virtual std::string serialize() const = 0;
    //virtual void deserialize(const std::string &) = 0;

    virtual Place::ptr_t& getPlace(const std::string& id) = 0;

    // one has to explicitly deallocate the memory!
    // all tokens contained in the result are copies!
    virtual workflow_result_t getResults() const = 0;
  };

  // ToDo: Is this still neccessary? Now GWDL uses Token::ptr_t to refer to tokens.
  // convenience deallocation method
  inline void deallocate_workflow_result(workflow_result_t &result)
  {
    for (workflow_result_t::iterator place(result.begin()); place != result.end(); ++place)
    {
      for (token_list_t::iterator token(place->second.begin()); token != place->second.end(); ++token)
      {
        //delete (*token);
    	  (*token).reset();
      }
      place->second.clear();
    }
    result.clear();
  }
}

#endif
