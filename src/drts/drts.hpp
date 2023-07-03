// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <drts/certificates.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/information_to_reattach.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/rifd_entry_points.hpp>
#include <drts/worker_description.hpp>

#include <logging/endpoint.hpp>

#include <we/type/value.hpp>

#if GSPC_WITH_IML
  #include <drts/stream.hpp>
  #include <drts/virtual_memory.hpp>
#else
  #include <gspc/iml/macros.hpp>
#endif
#include <gspc/iml/MemorySize.hpp>

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

namespace gspc
{
  namespace options
  {
    GSPC_DLLEXPORT ::boost::program_options::options_description logging();
    GSPC_DLLEXPORT ::boost::program_options::options_description installation();
    GSPC_DLLEXPORT ::boost::program_options::options_description drts();
    GSPC_DLLEXPORT ::boost::program_options::options_description external_rifd();
    GSPC_DLLEXPORT ::boost::program_options::options_description remote_iml();
    GSPC_DLLEXPORT ::boost::program_options::options_description virtual_memory();
  }

  class GSPC_DLLEXPORT installation
  {
  public:
    installation (::boost::filesystem::path const& gspc_home);
    installation (::boost::program_options::variables_map const& vm);

    ::boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }

  private:
    ::boost::filesystem::path const _gspc_home;
  };

  class GSPC_DLLEXPORT scoped_runtime_system
  {
  public:
    scoped_runtime_system ( ::boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          , std::ostream& info_output = std::cerr
                          , Certificates const& certificates = ::boost::none
                          );
    scoped_runtime_system ( ::boost::program_options::variables_map const& vm
                          , installation const&
                          , std::string const& topology_description
                          , rifd_entry_points const& entry_points
                          , std::ostream& info_output = std::cerr
                          , Certificates const& certificates = ::boost::none
                          );
    scoped_runtime_system
      ( ::boost::program_options::variables_map const& vm
      , installation const&
      , std::string const& topology_description
      , ::boost::optional<rifd_entry_points> const& entry_points
      , rifd_entry_point const& parent
      , std::ostream& info_output = std::cerr
      , Certificates const& certificates = ::boost::none
      );

    std::unordered_map< rifd_entry_point
                      , std::list<std::exception_ptr>
                      , rifd_entry_point_hash
                      >
      add_worker
        ( rifd_entry_points const&
        , Certificates const& certificates = ::boost::none
        );

    std::unordered_map< rifd_entry_point
                      , std::list<std::exception_ptr>
                      , rifd_entry_point_hash
                      >
      add_worker
        ( std::vector<worker_description> const&
        , rifd_entry_points const&
        , Certificates const& certificates = ::boost::none
        );

    std::unordered_map< rifd_entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      , rifd_entry_point_hash
                      >
      remove_worker (rifd_entry_points const&);

    #if GSPC_WITH_IML
    //! \note \a name is ignored and exists for API stability only.
    vmem_allocation alloc
      ( vmem::segment_description
      , unsigned long size
      , std::string const& name
      ) const;
    //! \note \a name is ignored and exists for API stability only.
    vmem_allocation alloc_and_fill
      ( vmem::segment_description
      , unsigned long size
      , std::string const& name
      , char const* data
      ) const;

    //! \note \a name is ignored and exists for API stability only.
    stream create_stream ( std::string const& name
                         , gspc::vmem_allocation const& buffer
                         , iml::MemorySize
                         , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                         ) const;
    #else
      GSPC_WITHOUT_IML_API_ERROR (alloc)
      GSPC_WITHOUT_IML_API_ERROR (alloc_and_fill)
      GSPC_WITHOUT_IML_API_ERROR (create_stream)
    #endif

    fhg::logging::endpoint top_level_log_demultiplexer() const;

    scoped_runtime_system (scoped_runtime_system const&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system const&) = delete;
    scoped_runtime_system (scoped_runtime_system&&) = delete;
    scoped_runtime_system& operator= (scoped_runtime_system&&) = delete;

  private:
    friend class information_to_reattach;

    PIMPL (scoped_runtime_system);
  };

  GSPC_DLLEXPORT void set_application_search_path
    ( ::boost::program_options::variables_map&
    , ::boost::filesystem::path const&
    );
  GSPC_DLLEXPORT void set_gspc_home ( ::boost::program_options::variables_map&
                                    , ::boost::filesystem::path const&
                                    );

  GSPC_DLLEXPORT void set_remote_iml_vmem_socket
    ( ::boost::program_options::variables_map&
    , ::boost::filesystem::path const&
    );
}
