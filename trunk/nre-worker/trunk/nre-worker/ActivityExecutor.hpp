/*
 * =====================================================================================
 *
 *       Filename:  ActivityExecutor.hpp
 *
 *    Description:  executes activities
 *
 *        Version:  1.0
 *        Created:  11/09/2009 04:01:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_APP_NRE_ACTIVITY_EXECUTOR_HPP
#define SDPA_APP_NRE_ACTIVITY_EXECUTOR_HPP 1

#include <iostream>

#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/nre/ExecutionContext.hpp>

namespace sdpa { namespace nre { namespace worker {
  class Request;
  class Reply;

  class ActivityExecutor : public ExecutionContext
  {
  public:
    ActivityExecutor(const sdpa::modules::ModuleLoader::ptr_t &mod_loader, const std::string &my_location)
      : loader_(mod_loader)
      , location_(my_location)
    {
    }

    const std::string &location() const { return location_; }

    void loop();

    // message context
    sdpa::modules::ModuleLoader &loader() { return *loader_; }
  private:
    Request *decode(const std::string &);
    std::string encode(Reply *);
    
    sdpa::modules::ModuleLoader::ptr_t loader_;
    std::string location_;
  };
}}}

#endif
