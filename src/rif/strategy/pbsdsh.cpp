// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <rif/strategy/pbsdsh.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/blocked.hpp>
#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <fmt/core.h>
#include <stdexcept>
#include <string>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace pbsdsh
      {
        namespace
        {
          namespace option
          {
            namespace po = fhg::util::boost::program_options;

            po::option<std::size_t, po::positive_integral<std::size_t>>
              const block_size { "pbsdshs-at-once"
                               , "how many PBSDSH connections to establish at once"
                               , 64
                               };
          }

#define EXTRACT_PARAMETERS(parameters_)                                 \
          auto const vm ( option::po::options ("pbsdsh")                \
                        . add (option::block_size)                      \
                        . store_and_notify (parameters_)                \
                        );                                              \
                                                                        \
          auto const block_size (option::block_size.get_from (vm))

          void do_pbsdsh
            (std::string const& hostname, std::string const& command)
          {
            //! \todo use torque's tm_spawn API directly to avoid system()
            util::system_with_blocked_SIGCHLD
              (fmt::format ("pbsdsh -h {} {}", hostname, command));
          }
        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , ::boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , ::boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_hostnames
            , block_size
            , [] (std::string const& hostname) { return hostname; }
            , [&] (std::string const& hostname)
              {
                do_pbsdsh ( hostname
                          , fmt::format
                              ( "{0}"
                                "{1}"
                                " --register-host {2} --register-port {3}"
                                " --register-key {4}"
                              , binary
                              , port ? " --port " + std::to_string (*port) : ""
                              , register_host
                              , register_port
                              , hostname
                              )
                          );
              }
            ).second;
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
            ( std::unordered_map<std::string, fhg::rif::entry_point> const& all_entry_points
            , std::vector<std::string> const& parameters
            )
        {
          EXTRACT_PARAMETERS (parameters);

          return util::blocked_async<std::string>
            ( all_entry_points
            , block_size
            , [] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                return entry_point.first;
              }
            , [&] (std::pair<std::string, fhg::rif::entry_point> const& entry_point)
              {
                do_pbsdsh ( entry_point.second.hostname
                          , fmt::format ( "/bin/kill -TERM {}"
                                        , entry_point.second.pid
                                        )
                          );
              }
            );
        }
      }
    }
  }
}
