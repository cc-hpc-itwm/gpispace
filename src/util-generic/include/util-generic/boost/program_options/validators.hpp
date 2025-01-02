// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_HPP

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        template<typename T>
          inline void validate ( ::boost::any& v
                               , const std::vector<std::string>& values
                               )
        {
          ::boost::program_options::validators::check_first_occurrence (v);

          v = ::boost::any (T (::boost::program_options::validators::get_single_string (values)));
        }
      }
    }
  }
}

#endif
