// Copyright (C) 2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/rif/strategy/local.hpp>

#include <gspc/util/system_with_blocked_SIGCHLD.hpp>
#include <gspc/util/hostname.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <stdexcept>
#include <string>




      namespace gspc::rif::strategy::local
      {
        namespace
        {
          void do_local (std::string const& command)
          {
            gspc::util::system_with_blocked_SIGCHLD (command);
          }
        }

        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& all_hostnames
                    , std::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , std::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    )
        {
          if (!parameters.empty())
          {
            throw std::invalid_argument
              ("rif-strategy 'local' does not accept parameters");
          }

          if (all_hostnames.size() != 1)
          {
            throw std::invalid_argument
              ( "rif-strategy 'local' expects exactly one host, got "
              + std::to_string (all_hostnames.size())
              );
          }

          auto const localhost (gspc::util::hostname());
          auto const& hostname (all_hostnames.front());
          if (hostname != localhost)
          {
            throw std::invalid_argument
              ( "local strategy used to bootstrap on non-localhost host: "
                "got=" + hostname + ", expected=" + localhost
              );
          }

          try
          {
            do_local ( fmt::format
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

            return std::unordered_map<std::string, std::exception_ptr>{};
          }
          catch (...)
          {
            return {{hostname, std::current_exception()}};
          }
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
            ( std::unordered_map<std::string, gspc::rif::entry_point> const& all_entry_points
            , std::vector<std::string> const& parameters
            )
        {
          if (!parameters.empty())
          {
            throw std::invalid_argument
              ("rif-strategy 'local' does not accept parameters");
          }

          if (all_entry_points.size() != 1)
          {
            throw std::invalid_argument
              ( "rif-strategy 'local' expects exactly one entry_point, got "
              + std::to_string (all_entry_points.size())
              );
          }

          auto const localhost (gspc::util::hostname());
          auto const& entry_point (*all_entry_points.begin());
          auto const& hostname (entry_point.first);
          if (hostname != localhost)
          {
            throw std::invalid_argument
              ( "local strategy used to teardown on non-localhost host: "
                "got=" + hostname + ", expected=" + localhost
              );
          }

          try
          {
            do_local ( fmt::format ( "/bin/kill -TERM {}"
                                   , entry_point.second.pid
                                   )
                     );

            return { {hostname}
                   , std::unordered_map<std::string, std::exception_ptr>{}
                   };
          }
          catch (...)
          {
            return { std::unordered_set<std::string>{}
                   , {{hostname, std::current_exception()}}
                   };
          }
        }
      }
