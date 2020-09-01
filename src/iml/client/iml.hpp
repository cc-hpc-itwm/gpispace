#pragma once

#include <iml/client/iml.fwd.hpp>
#include <iml/client/rifd_entry_points.hpp>
#include <iml/client/stream.hpp>
#include <iml/client/virtual_memory.fwd.hpp>

#include <iml/vmem/netdev_id.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <exception>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace iml_client
{
  namespace options
  {
    boost::program_options::options_description installation();
    boost::program_options::options_description external_rifd();
    boost::program_options::options_description virtual_memory();
  }

  class installation
  {
  public:
    installation (boost::filesystem::path const& iml_home);
    installation (boost::program_options::variables_map const& vm);

    boost::filesystem::path const& iml_home() const
    {
      return _iml_home;
    }

  private:
    boost::filesystem::path const _iml_home;
  };

  class scoped_iml_runtime_system
  {
  public:
    scoped_iml_runtime_system ( boost::program_options::variables_map const& vm
                          , installation const&
                          , rifd_entry_points const& entry_points
                          , std::ostream& info_output = std::cerr
                          );
    scoped_iml_runtime_system
      ( boost::program_options::variables_map const& vm
      , installation const&
      , boost::optional<rifd_entry_points> const& entry_points
      , rifd_entry_point const& master
      , std::ostream& info_output = std::cerr
      );

    vmem_allocation alloc
      ( vmem::segment_description
      , unsigned long size
      , std::string const& name
      ) const;
    vmem_allocation alloc_and_fill
      ( vmem::segment_description
      , unsigned long size
      , std::string const& name
      , char const* const data
      ) const;

    stream create_stream ( std::string const& name
                         , iml_client::vmem_allocation const& buffer
                         , iml_client::stream::size_of_slot const&
                         , std::function<void ( gpi::pc::type::range_t const meta
                                              , gpi::pc::type::range_t const data
                                              , char const flag
                                              , std::size_t const id
                                              )
                                        > on_slot_filled
                         ) const;

    scoped_iml_runtime_system (scoped_iml_runtime_system const&) = delete;
    scoped_iml_runtime_system& operator= (scoped_iml_runtime_system const&) = delete;
    scoped_iml_runtime_system (scoped_iml_runtime_system&&) = delete;
    scoped_iml_runtime_system& operator= (scoped_iml_runtime_system&&) = delete;

  private:
    friend class vmem_allocation;
    friend class stream;

    PIMPL (scoped_iml_runtime_system);
  };

#define SET(_name, _type) \
  void set_ ## _name (boost::program_options::variables_map&, _type const&)

  SET (nodefile, boost::filesystem::path);
  SET (iml_home, boost::filesystem::path);

  SET (virtual_memory_socket, boost::filesystem::path);
  SET (virtual_memory_port, unsigned short);
  SET (virtual_memory_startup_timeout, unsigned long);
  SET (virtual_memory_netdev_id, fhg::iml::vmem::netdev_id);

  SET (rif_entry_points_file, boost::filesystem::path);
  SET (rif_port, unsigned short);
  SET (rif_strategy, std::string);
  SET (rif_strategy_parameters, std::vector<std::string>);

#undef SET
}
