// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP

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
        struct nonexisting_path : public ::boost::filesystem::path
        {
          nonexisting_path (std::string const& option)
            : ::boost::filesystem::path (option)
          {
            if (::boost::filesystem::exists (*this))
            {
              throw ::boost::program_options::invalid_option_value
                ((::boost::format ("%1% already exists.") % *this).str());
            }
          }

          nonexisting_path (::boost::filesystem::path const& p)
            : nonexisting_path (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , nonexisting_path*
                             , int
                             )
        {
          validate<nonexisting_path> (v, values);
        }
      }
    }
  }
}

#endif
