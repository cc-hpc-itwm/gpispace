// Copyright (C) 2015,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/serialize.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <sstream>



    namespace gspc::pnet::type::value
    {
      std::string to_string (value_type const& value)
      {
        std::ostringstream oss;
        ::boost::archive::text_oarchive oa (oss);
        oa & value;
        return oss.str();
      }

      value_type from_string (std::string const& input)
      {
        std::istringstream iss (input);
        ::boost::archive::text_iarchive ia (iss);
        value_type value;
        ia & value;
        return value;
      }
    }
