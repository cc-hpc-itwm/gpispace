// alexander.petry@itwm.fraunhofer.de

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

  FHGLOG_TERM ();

  return 0;
}
