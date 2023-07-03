// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_IN_EXISTING_DIRECTORY_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_IN_EXISTING_DIRECTORY_HPP

#include <util-generic/boost/program_options/validators.hpp>
#include <util-generic/boost/program_options/validators/existing_directory.hpp>
#include <util-generic/boost/program_options/validators/nonexisting_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        struct nonexisting_path_in_existing_directory : public nonexisting_path
        {
          nonexisting_path_in_existing_directory (std::string const& option)
            : nonexisting_path (option)
            , _existing_directory
                (::boost::filesystem::path (option).parent_path().string())
          {}

          nonexisting_path_in_existing_directory
              (::boost::filesystem::path const& p)
            : nonexisting_path_in_existing_directory (p.string())
          {}

        private:
          existing_directory _existing_directory;
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , nonexisting_path_in_existing_directory*
                             , int
                             )
        {
          validate<nonexisting_path_in_existing_directory> (v, values);
        }
      }
    }
  }
}

#endif
