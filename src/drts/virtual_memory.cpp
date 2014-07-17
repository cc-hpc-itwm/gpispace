// mirko.rahn@itwm.fraunhofer.de

#include <drts/virtual_memory.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <exception>

namespace gspc
{
  namespace
  {
    std::string vmem_alloc ( boost::filesystem::path const& vmem_socket
                           , unsigned long const size
                           , std::string const& description
                           )
    {
      // taken from bin/gpish
      gpi::pc::client::api_t capi (vmem_socket.string());
      capi.start();

      gpi::pc::type::segment_id_t const segment_id (1);

      gpi::pc::type::handle_id_t handle_id
        (capi.alloc ( segment_id
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

      // taken from gpi-space/pc/type/handle.hpp
      std::ostringstream oss;

      oss << "0x";
      oss.flags (std::ios::hex);
      oss.width (18);
      oss.fill ('0');
      oss << handle_id;

      return oss.str();
    }
  }

  vmem_allocation::vmem_allocation
    ( boost::filesystem::path const& gspc_home
    , boost::filesystem::path const& virtual_memory_socket
    , unsigned long size
    , std::string const& description
    )
      : _gspc_home (gspc_home)
      , _vmem_socket (virtual_memory_socket)
      , _handle (vmem_alloc (_vmem_socket, size, description))
      , _disowned (false)
  {}
  vmem_allocation::~vmem_allocation()
  {
    if (!_disowned)
    {
      // taken from bin/gpish
      gpi::pc::client::api_t capi (_vmem_socket.string());
      capi.start();
      capi.free (boost::lexical_cast<gpi::pc::type::handle_t> (_handle));
    }
  }

  vmem_allocation::vmem_allocation (vmem_allocation&& other)
    : _gspc_home (std::move (other._gspc_home))
    , _vmem_socket (std::move (other._vmem_socket))
    , _handle (std::move (other._handle))
    , _disowned (std::move (other._disowned))
  {
    other._disowned = true;
  }
}
