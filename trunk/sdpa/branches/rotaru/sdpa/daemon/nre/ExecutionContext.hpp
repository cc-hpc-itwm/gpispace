/*
 * =====================================================================================
 *
 *       Filename:  MessageContext.hpp
 *
 *    Description:  message execution context
 *
 *        Version:  1.0
 *        Created:  11/09/2009 05:37:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_NRE_WORKER_EXECUTION_CONTEXT_HPP
#define SDPA_NRE_WORKER_EXECUTION_CONTEXT_HPP 1

#include <sdpa/modules/ModuleLoader.hpp>

namespace sdpa { namespace nre { namespace worker {
  class ExecutionContext
  {
  public:
    virtual ~ExecutionContext() {}

    virtual sdpa::modules::ModuleLoader &loader() = 0;
    virtual int getRank() const = 0;
  };
}}}

#endif
