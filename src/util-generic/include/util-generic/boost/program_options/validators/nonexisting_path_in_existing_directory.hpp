// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_IN_EXISTING_DIRECTORY_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_IN_EXISTING_DIRECTORY_HPP

#include <util-generic/boost/program_options/validators.hpp>
#include <util-generic/boost/program_options/validators/existing_directory.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <filesystem>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        struct nonexisting_path_in_existing_directory
        {
          nonexisting_path_in_existing_directory
            ( std::filesystem::path const& path
            )
              : _path {nonexisting_path {path}}
          {
            std::ignore = existing_directory (_path.parent_path());
          }

          nonexisting_path_in_existing_directory (std::string const& option)
            : nonexisting_path_in_existing_directory
                {std::filesystem::path {option}}
          {}

          operator std::filesystem::path() const
          {
            return _path;
          }

          [[deprecated ("use nonexisting_path_in_existing_directory (std::filesystem::path)")]]
          nonexisting_path_in_existing_directory (::boost::filesystem::path const& p)
            : nonexisting_path_in_existing_directory {p.string()}
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
                             , nonexisting_path_in_existing_directory*
                             , int
                             )
        {
          validate<nonexisting_path_in_existing_directory> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , nonexisting_path_in_existing_directory const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }
    }
  }
}

#endif
