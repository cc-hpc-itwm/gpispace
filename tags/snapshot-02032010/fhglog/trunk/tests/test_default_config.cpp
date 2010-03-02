/*
 * =====================================================================================
 *
 *       Filename:  test_default_config.cpp
 *
 *    Description:  manually test the default config and its effects
 *
 *        Version:  1.0
 *        Created:  11/14/2009 01:44:13 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <sstream>

int main(int ac, char *av[])
{
  using namespace fhg::log;

  Configurator::configure();

  std::ostringstream msg;
  for (int i = 1; ;)
  {
    if (i == ac) break;

    msg << av[i];

    ++i;
    if (i < ac) msg << " ";
  }

  for (int level = LogLevel::MIN_LEVEL; level < LogLevel::MAX_LEVEL; ++level)
  {
    getLogger().log(LogEvent(LogLevel((LogLevel::Level)level), __FILE__, FHGLOG_FUNCTION, __LINE__, msg.str()));
  }

  return 0;
}
