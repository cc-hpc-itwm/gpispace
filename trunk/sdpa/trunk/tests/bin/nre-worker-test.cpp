/*
 * =====================================================================================
 *
 *       Filename:  test_worker.cpp
 *
 *    Description:  tests the nre worker component
 *
 *        Version:  1.0
 *        Created:  11/12/2009 12:23:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/nre/NreWorkerClient.hpp>
#include <sdpa/wf/Activity.hpp>

int main(int ac, char **av)
{
  fhg::log::Configurator::configure();

  std::string worker_location("127.0.0.1:8000");
  std::string function_call("init@init");
  if (ac > 1)
  {
    worker_location = av[1];
  }
  if (ac > 2)
  {
    function_call = av[2];
  }

  sdpa::nre::worker::NreWorkerClient client(worker_location);
  client.start();

  // try to execute an activity
  sdpa::wf::Activity a_in("activity-1", sdpa::wf::Method(function_call));
  sdpa::wf::Activity a_ou(client.execute(a_in));
  LOG(INFO, "got reply: " << a_ou);

  client.stop();
  return 0;
}

