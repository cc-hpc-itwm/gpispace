// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_DIRECTORY_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_DIRECTORY_HPP

#include <util-generic/boost/program_options/validators/existing_path.hpp>

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
        struct existing_directory
        {
          existing_directory (std::string const& option)
            : existing_directory {std::filesystem::path {option}}
          {}

          existing_directory (std::filesystem::path const& path)
            : _path {existing_path {path}}
          {
            if (!std::filesystem::is_directory (_path))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ( "{} is not a directory.", _path)
                };
            }
          }

          operator std::filesystem::path() const
          {
            return _path;
          }

          [[deprecated ("use existing_directory (std::filesystem::path)")]]
          existing_directory (::boost::filesystem::path const& p)
            : existing_directory (p.string())
          {}

          [[deprecated ("use existing_directory::operator (std::filesystem::path)")]]
          explicit operator ::boost::filesystem::path() const
          {
            return _path.string();
          }

        private:
          std::filesystem::path _path;
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , existing_directory*
                             , int
                             )
        {
          validate<existing_directory> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , existing_directory const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }
    }
  }
}

#endif
