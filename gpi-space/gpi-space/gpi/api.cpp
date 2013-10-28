#include "api.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include "fake_api.hpp"
#ifdef ENABLE_REAL_GPI
#   include "real_api.hpp"
#endif

namespace gpi
{
  namespace api
  {
    boost::shared_ptr<gpi_api_t> gpi_api_t::instance;
    const char * gpi_api_t::REAL_API = "gpi.api.real";
    const char * gpi_api_t::FAKE_API = "gpi.api.fake";

    gpi_api_t & gpi_api_t::create (std::string const & impl)
    {
      if (!instance)
      {
        if (impl == FAKE_API)
        {
          instance.reset (new fake_gpi_api_t);
        }
#ifdef ENABLE_REAL_GPI
        else if (impl == REAL_API)
        {
          instance.reset (new real_gpi_api_t);
        }
#endif
        else
        {
          throw std::runtime_error
            ("requested implementation not available in this build: " + impl);
        }
      }
      else
      {
        throw std::runtime_error
          ("gpi_api_t: singleton already created, go fix your code!");
      }

      return *instance;
    }

    gpi_api_t & gpi_api_t::get ()
    {
      assert (instance);
      return *instance;
    }

    void gpi_api_t::destroy ()
    {
      if (instance)
      {
        instance.reset();
      }
    }
  }
}
