// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_PATH_HPP

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
        struct existing_path : public ::boost::filesystem::path
        {
          existing_path (std::string const& option)
            : ::boost::filesystem::path (::boost::filesystem::canonical (option))
          {
            if (!::boost::filesystem::exists (*this))
            {
              throw ::boost::program_options::invalid_option_value
                ((::boost::format ("%1% does not exist.") % *this).str());
            }
          }

          existing_path (::boost::filesystem::path const& p)
            : existing_path (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , existing_path*
                             , int
                             )
        {
          validate<existing_path> (v, values);
        }
      }
    }
  }
}

#endif
