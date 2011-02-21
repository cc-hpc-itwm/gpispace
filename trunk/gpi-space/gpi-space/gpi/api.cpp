#include "api.hpp"

#include <fhglog/minimal.hpp>

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
#ifdef ENABLE_REAL_GPI
        if (impl == "gpi.api.real")
        {
           instance.reset (new real_gpi_api_t);
        }
#endif

        if (instance)
        {
           return *instance;
        }
        else
        {
           throw std::runtime_error("Implemenation not available: " + impl);
        }
      }
      else
      {
        throw std::runtime_error ("already created!");
      }
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

    gpi_api_t::~gpi_api_t ()
    {}
  }
}
