// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP

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
        struct nonexisting_path
        {
          nonexisting_path (std::string const& option)
            : nonexisting_path {std::filesystem::path {option}}
          {}

          nonexisting_path (std::filesystem::path const& path)
            : _path {path}
          {
            if (std::filesystem::exists (_path))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ("{} already exists.", _path)
                };
            }
          }

          operator std::filesystem::path() const
          {
            return _path;
          }

          [[deprecated ("use nonexisting_path (std::filesystem::path)")]]
          nonexisting_path (::boost::filesystem::path const& p)
            : nonexisting_path (p.string())
          {}

          [[deprecated ("use nonexisting_path::operator (std::filesystem::path)")]]
          explicit operator ::boost::filesystem::path() const
          {
            return _path.string();
          }

        private:
          std::filesystem::path _path;
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , nonexisting_path*
                             , int
                             )
        {
          validate<nonexisting_path> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , nonexisting_path const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }
    }
  }
}

#endif
