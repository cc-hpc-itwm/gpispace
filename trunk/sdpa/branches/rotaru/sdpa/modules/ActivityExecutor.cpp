/*
 * =====================================================================================
 *
 *       Filename:  ActivityExecutor.cpp
 *
 *    Description:  activity executor implementation
 *
 *        Version:  1.0
 *        Created:  10/15/2009 06:33:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <string>
#include "ActivityExecutor.hpp"

using namespace sdpa::modules;

void ActivityExecutor::execute(sdpa::wf::Activity &act) throw (std::exception)
{
  // FIXME: does it make sense to make a backup copy of the parameters?
  // sdpa::wf::Activity::parameters_t params(act.parameters());
  const std::string mod_name(act.method().module());
  const std::string fun_name(act.method().name());

  loader_->get(mod_name).call(fun_name, act.parameters());
}
