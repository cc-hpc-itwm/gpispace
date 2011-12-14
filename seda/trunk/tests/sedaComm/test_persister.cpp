/*
 * =====================================================================================
 *
 *       Filename:  test_udp.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/25/2009 04:24:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <unistd.h>

#include <seda/comm/Locator.hpp>
#include <seda/comm/LocatorPersister.hpp>

int main(int, char **)
{
  fhg::log::Configurator::configure();

  seda::comm::Locator loc;
  loc.insert("foo", "1.2.3.4", 42);

  seda::comm::LocatorPersister persister("foo.loc");
  persister.store(loc);

  seda::comm::Locator loc2;
  persister.load(loc2);

  LOG(DEBUG, loc2.lookup("foo").host());

  unlink("foo.loc");

  return 0;
}
