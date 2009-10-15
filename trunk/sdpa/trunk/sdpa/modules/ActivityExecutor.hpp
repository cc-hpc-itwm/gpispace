/*
 * =====================================================================================
 *
 *       Filename:  ActivityExecutor.hpp
 *
 *    Description:  Executes activities
 *
 *        Version:  1.0
 *        Created:  10/15/2009 06:15:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_MODULES_ACTIVITY_EXECUTOR_HPP
#define SDPA_MODULES_ACTIVITY_EXECUTOR_HPP 1

#include <stdexcept>

#include <sdpa/memory.hpp>
#include <sdpa/modules/ModuleLoader.hpp>

namespace sdpa { namespace modules {

  class ActivityExecutor
  {
  public:
    typedef shared_ptr<ActivityExecutor> ptr_t;

    explicit
    ActivityExecutor(const ModuleLoader::ptr_t &loader)
      : loader_(loader)
    {}

    void execute(sdpa::wf::Activity &) throw (std::exception);
  private:
    ModuleLoader::ptr_t loader_;
  };
}}

#endif
