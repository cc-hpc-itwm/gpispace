#include <test/certificates_data.hpp>

namespace gspc
{
  namespace testing
  {
    Certificates no_certs()
    {
      return boost::none;
    }
    Certificates yes_certs()
    {
      return Certificates (GSPC_SSL_CERTIFICATES_FOR_TESTS);
    }
  }
}
