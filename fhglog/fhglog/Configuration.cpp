// bernd.loerwald@itwm.fraunhofer.de

#include <fhglog/DefaultConfiguration.hpp>

namespace fhg
{
  namespace log
  {
    void configure()
    {
      DefaultConfiguration()();
    }

    void configure (int ac, char *av[])
    {
      // parameters currently ignored
      configure();
    }
  }
}
