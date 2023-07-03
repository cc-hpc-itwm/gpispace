// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXECUTABLE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXECUTABLE_HPP

#include <util-generic/boost/program_options/validators.hpp>

#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <system_error>

#include <unistd.h>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        struct executable : public existing_path
        {
          executable (std::string const& option)
            : existing_path (option)
          {
            int const rc (::access (c_str(), F_OK | X_OK));
            int const errno_value (errno);
            if (rc == -1)
            {
              throw ::boost::program_options::invalid_option_value
                ( ( ::boost::format ("%1% is not executable: %2%")
                  % *this
                  % std::system_category().message (errno_value)
                  ).str()
                );
            }
          }

          executable (::boost::filesystem::path const& p)
            : executable (p.string())
          {}
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , executable*
                             , int
                             )
        {
          validate<executable> (v, values);
        }
      }
    }
  }
}

#endif
