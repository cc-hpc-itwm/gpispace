// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_REQUIRE_ALL_IF_ONE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_REQUIRE_ALL_IF_ONE_HPP

#include <boost/program_options.hpp>

#include <algorithm>
#include <functional>
#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        static
        void require_all_if_one
          (::boost::program_options::variables_map const& vm, std::initializer_list<const char*> options)
        {
          std::function<bool (const char*)> const has_option
            ( [&vm] (const char* option)
              {
                return !!vm.count (option);
              }
            );

          if ( std::any_of (options.begin(), options.end(), has_option)
             && !std::all_of (options.begin(), options.end(), has_option)
             )
          {
            std::stringstream message;
            message << "setting any option of";
            for (const char* option : options)
            {
              message << " '" << option << "'";
            }
            message << " requires setting all of them";
            throw std::invalid_argument (message.str());
          }
        }
      }
    }
  }
}

#endif
