// mirko.rahn@itwm.fraunhofer.de

#include <test/setup_logging.hpp>

#include <drts/drts.hpp>

#include <fhg/util/hostname.hpp>

namespace test
{
  void setup_logging (boost::program_options::variables_map& vm)
  {
    //! \todo start logger with specific port
    gspc::set_log_host (vm, fhg::util::hostname());
    gspc::set_log_port (vm, 47095);

    //! \todo either disable gui logging or get the port from a gui manager
    gspc::set_gui_host (vm, fhg::util::hostname());
    gspc::set_gui_port (vm, 47096);
  }
}
