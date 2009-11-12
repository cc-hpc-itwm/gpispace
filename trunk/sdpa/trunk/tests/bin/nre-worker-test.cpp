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

int main(int ac, char **av)
{
  fhg::log::Configurator::configure();

  std::string worker_location("127.0.0.1:8000");
  if (ac > 1)
  {
    worker_location = av[1];
  }

  sdpa::nre::worker::NreWorkerClient client(worker_location);
  client.start();
  return 0;
}

