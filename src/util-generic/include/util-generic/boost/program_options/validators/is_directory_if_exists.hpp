// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_IS_DIRECTORY_IF_EXISTS_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_IS_DIRECTORY_IF_EXISTS_HPP

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
        struct is_directory_if_exists : public ::boost::filesystem::path
        {
          is_directory_if_exists (std::string const& option)
            : ::boost::filesystem::path (option)
          {
            if ( ::boost::filesystem::exists (*this)
               && !::boost::filesystem::is_directory (*this)
               )
            {
              throw ::boost::program_options::invalid_option_value
                (( ::boost::format ("%1% exists and is not a directory.")
                 % *this
                 ).str()
                );
            }
          }

          is_directory_if_exists (::boost::filesystem::path const& p)
            : is_directory_if_exists (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , is_directory_if_exists*
                             , int
                             )
        {
          validate<is_directory_if_exists> (v, values);
        }
      }
    }
  }
}

#endif
