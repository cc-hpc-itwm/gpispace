#include <iml/client/iml.hpp>
#include <iml/client/virtual_memory.hpp>

#include <iml/revision.hpp>

#include <iml/client/private/iml_impl.hpp>
#include <iml/client/private/option.hpp>
#include <iml/client/private/pimpl.hpp>
#include <iml/client/private/rifd_entry_points_impl.hpp>
#include <iml/client/private/startup_and_shutdown.hpp>

#include <iml/vmem/gaspi/pc/client/api.hpp>

#include <fhg/util/boost/program_options/require_all_if_one.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/split.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/format.hpp>

#include <ostream>
#include <sstream>
#include <stdexcept>

namespace iml_client
{
  installation::installation
    (boost::filesystem::path const& iml_home)
      : _iml_home (boost::filesystem::canonical (iml_home))
  {
    boost::filesystem::path const path_revision (_iml_home / "revision");

    if (!boost::filesystem::exists (path_revision))
    {
      throw std::invalid_argument
        (( boost::format ("IML revision mismatch: File '%1%' does not exist.")
         % path_revision
         ).str());
    }

    std::string const revision (fhg::util::read_file (path_revision));

    if (revision != fhg::iml::project_revision())
    {
      throw std::invalid_argument
        (( boost::format ( "IML revision mismatch: Expected '%1%'"
                         ", installation in '%2%' has version '%3%'"
                         )
         % fhg::iml::project_revision()
         % _iml_home
         % revision
         ).str()
        );
    }
  }
  installation::installation
    (boost::program_options::variables_map const& vm)
      : installation (require_iml_home (vm))
  {}

  scoped_iml_runtime_system::scoped_iml_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , rifd_entry_points const& entry_points
    , std::ostream& info_output
    )
      : scoped_iml_runtime_system
          ( vm
          , installation
          , entry_points
          , [&entry_points]() -> rifd_entry_point
            {
              if (entry_points._->_entry_points.empty())
              {
                throw std::logic_error
                  ("scoped_iml_runtime_system: no entry_points given");
              }

              return { new rifd_entry_point::implementation
                         (entry_points._->_entry_points.front())
                     };
            }()
          , info_output
          )
  {}
  scoped_iml_runtime_system::scoped_iml_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    )
      : _ (new implementation ( vm
                              , installation
                              , entry_points
                              , master
                              , info_output
                              )
          )
  {}

  scoped_iml_runtime_system::implementation::started_runtime_system::started_runtime_system
      ( boost::optional<boost::filesystem::path> gpi_socket
      , iml_client::installation_path installation_path
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , boost::optional<unsigned short> vmem_port
      , boost::optional<fhg::iml::vmem::netdev_id> vmem_netdev_id
      , std::vector<fhg::iml::rif::entry_point> const& rif_entry_points
      , fhg::iml::rif::entry_point const& master
      , std::ostream& info_output
      )
    : _master (master)
    , _info_output (info_output)
    , _gpi_socket (gpi_socket)
    , _installation_path (installation_path)
    , _processes_storage (_info_output)
  {
    fhg::iml_drts::startup
      ( _gpi_socket
      , _installation_path
      , _processes_storage
      , vmem_startup_timeout
      , vmem_port
      , vmem_netdev_id
      , rif_entry_points
      , _master
      , _info_output
      );
  }

  scoped_iml_runtime_system::implementation::implementation
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , boost::optional<rifd_entry_points> const& entry_points
    , rifd_entry_point const& master
    , std::ostream& info_output
    )
      : _virtual_memory_socket (get_virtual_memory_socket (vm))
      , _virtual_memory_startup_timeout
        ( get_virtual_memory_startup_timeout (vm)
        ? boost::make_optional
          (std::chrono::seconds (get_virtual_memory_startup_timeout (vm).get()))
        : boost::none
        )
      , _started_runtime_system
          ( _virtual_memory_socket
          , installation.iml_home()
          , _virtual_memory_startup_timeout
          , get_virtual_memory_port (vm)
          , get_virtual_memory_netdev_id (vm)
          , !entry_points
            ? decltype (entry_points->_->_entry_points) {}
            : entry_points->_->_entry_points
          , master._->_entry_point
          , info_output
          )
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::cxx14::make_unique<gpi::pc::client::api_t>
            (_virtual_memory_socket->string())
        : nullptr
        )
  {}

  PIMPL_DTOR (scoped_iml_runtime_system)

  vmem_allocation scoped_iml_runtime_system::alloc
    ( vmem::segment_description segment_description
    , unsigned long size
    , std::string const& name
    ) const
  {
    return vmem_allocation (this, segment_description, size, name);
  }
  vmem_allocation scoped_iml_runtime_system::alloc_and_fill
    ( vmem::segment_description segment_description
    , unsigned long size
    , std::string const& name
    , char const* const data
    ) const
  {
    return vmem_allocation (this, segment_description, size, name, data);
  }

  stream scoped_iml_runtime_system::create_stream
    ( std::string const& name
    , iml_client::vmem_allocation const& buffer
    , stream::size_of_slot const& size_of_slot
    , std::function<void ( gpi::pc::type::range_t const meta
                         , gpi::pc::type::range_t const data
                         , char const flag
                         , std::size_t const id
                         )
                   > on_slot_filled
    ) const
  {
    return stream (*this, name, buffer, size_of_slot, on_slot_filled);
  }
}
