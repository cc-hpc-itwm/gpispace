#pragma once

#include <drts/drts.fwd.hpp>

#include <drts/information_to_reattach.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/rifd_entry_points.hpp>
#include <drts/stream.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <we/type/value.hpp>

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

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t;
    }
  }
}

namespace gspc
{
  namespace options
  {
    boost::program_options::options_description logging();
    boost::program_options::options_description installation();
    boost::program_options::options_description drts();
    boost::program_options::options_description external_rifd();
    boost::program_options::options_description virtual_memory();
  }

  class installation
  {
  public:
    installation (boost::filesystem::path const& gspc_home);
    installation (boost::program_options::variables_map const& vm);

    boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }

  private:
    boost::filesystem::path const _gspc_home;
  };

  class scoped_runtime_system
  {
  public:
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          );
    scoped_runtime_system ( boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          , rifd_entry_points const& entry_points
                          );
    scoped_runtime_system
      ( boost::program_options::variables_map const& vm
      , installation const&
      , std::string const& topology_description
      , boost::optional<rifd_entry_points> const& entry_points
      , rifd_entry_point const& master
      );
    scoped_runtime_system
      ( boost::program_options::variables_map const& vm
      , installation const&
      , std::string const& topology_description
      , boost::optional<rifd_entry_points> const& entry_points
      , rifd_entry_point const& master
      , std::ostream& info_output
      );

    scoped_runtime_system
      ( boost::program_options::variables_map const& vm
      , installation const& installation
      , std::string const& topology_description
      , rifd_entry_points const& entry_points
      , std::size_t const& shared_cache_size
      );

    std::unordered_map< rifd_entry_point
                      , std::list<std::exception_ptr>
                      , rifd_entry_point_hash
                      >
      add_worker (rifd_entry_points const&);
    std::unordered_map< rifd_entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      , rifd_entry_point_hash
                      >
      remove_worker (rifd_entry_points const&);

    scoped_vmem_segment_and_allocation alloc
      ( vmem::segment_description
      , unsigned long size
      ) const;
    scoped_vmem_segment_and_allocation alloc_and_fill
      ( vmem::segment_description
      , unsigned long size
      , char const* const data
      ) const;

    stream create_stream ( gspc::scoped_vmem_segment_and_allocation const& buffer
                         , gspc::stream::size_of_slot const&
                         , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                         ) const;

    scoped_runtime_system (scoped_runtime_system const&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system const&) = delete;
    scoped_runtime_system (scoped_runtime_system&&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system&&) = delete;

  private:
    friend class scoped_vmem_segment_and_allocation;
    friend class information_to_reattach;
    friend class stream;

    PIMPL (scoped_runtime_system);
  };

  void set_application_search_path ( boost::program_options::variables_map&
                                   , boost::filesystem::path const&
                                   );
  void set_gspc_home ( boost::program_options::variables_map&
                     , boost::filesystem::path const&
                     );
}
