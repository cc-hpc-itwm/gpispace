// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_PATH_HPP

#include <util-generic/boost/program_options/validators.hpp>

#include <boost/filesystem.hpp>

#include <filesystem>
#include <fmt/core.h>
#include <fmt/std.h>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        struct existing_path
        {
          existing_path (std::string const& option)
            : existing_path {std::filesystem::path {option}}
          {}

          existing_path (std::filesystem::path const& path)
            : _path {std::filesystem::canonical (path)}
          {
            if (!std::filesystem::exists (_path))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ("{} does not exist.", _path)
                };
            }
          }

          operator std::filesystem::path() const
          {
            return _path;
          }

          [[deprecated ("use existing_path (std::filesystem::path)")]]
          existing_path (::boost::filesystem::path const& p)
            : existing_path (p.string())
          {}

          [[deprecated ("use existing_path::operator (std::filesystem::path)")]]
          explicit operator ::boost::filesystem::path() const
          {
            return _path.string();
          }

        private:
          std::filesystem::path _path;
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , existing_path*
                             , int
                             )
        {
          validate<existing_path> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , existing_path const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }
    }
  }
}

#endif
