// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_DIRECTORY_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_DIRECTORY_HPP

#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <util-generic/boost/program_options/validators.hpp>

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
        struct existing_directory : public existing_path
        {
          existing_directory (std::string const& option)
            : existing_path (option)
          {
            if (!::boost::filesystem::is_directory (*this))
            {
              throw ::boost::program_options::invalid_option_value
                ((::boost::format ("%1% is not a directory.") % *this).str());
            }
          }

          existing_directory (::boost::filesystem::path const& p)
            : existing_directory (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , existing_directory*
                             , int
                             )
        {
          validate<existing_directory> (v, values);
        }
      }
    }
  }
}

#endif
