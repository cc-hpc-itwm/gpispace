// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/virtual_memory.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <boost/format.hpp>

#include <exception>

namespace gspc
{
  namespace
  {
    gpi::pc::type::handle_id_t vmem_alloc ( gpi::pc::client::api_t* api
                                          , unsigned long const size
                                          , std::string const& description
                                          )
    {
      // taken from bin/gpish
      gpi::pc::type::segment_id_t const segment_id (1);

      gpi::pc::type::handle_id_t handle_id
        (api->alloc ( segment_id
                    , size
                    , description
                    , gpi::pc::F_GLOBAL | gpi::pc::F_PERSISTENT
                    )
        );

      if (gpi::pc::type::handle::is_null (handle_id))
      {
        throw std::runtime_error
          ( ( boost::format
              ("Could not allocate %1% bytes with description '%2%'")
            % size
            % description
            ).str()
          );
      }

      return handle_id;
    }
  }

  struct vmem_allocation::implementation
  {
    implementation ( gpi::pc::client::api_t* api
                   , unsigned long size
                   , std::string const& description
                   )
      : _api (api)
      , _handle_id (vmem_alloc (_api, size, description))
      , _disowned (false)
    {}
    ~implementation()
    {
      if (!_disowned)
      {
        _api->free (gpi::pc::type::handle_t (_handle_id));
      }
    }
    implementation (implementation&& other)
      : _api (std::move (other._api))
      , _handle_id (std::move (other._handle_id))
      , _disowned (std::move (other._disowned))
    {
      other._disowned = true;
    }

    gpi::pc::client::api_t* _api;
    gpi::pc::type::handle_id_t _handle_id;
    bool _disowned;
  };

  vmem_allocation::vmem_allocation ( scoped_runtime_system const* const drts
                                   , unsigned long size
                                   , std::string const& description
                                   )
    : _ ( new vmem_allocation::implementation
          (drts->_virtual_memory_api, size, description)
        )
  {}
  vmem_allocation::~vmem_allocation()
  {
    delete _;
  }
  std::string const vmem_allocation::handle() const
  {
    // taken from gpi-space/pc/type/handle.hpp
    std::ostringstream oss;

    oss << "0x";
    oss.flags (std::ios::hex);
    oss.width (18);
    oss.fill ('0');
    oss << _->_handle_id;

    return oss.str();
  }
  vmem_allocation::vmem_allocation (vmem_allocation&& other)
    : _ (std::move (other._))
  {
    other._ = nullptr;
  }
}
