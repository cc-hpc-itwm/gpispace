// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_INTEGRAL_GREATER_THAN_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_INTEGRAL_GREATER_THAN_HPP

#include <util-generic/boost/program_options/validators.hpp>

#include <util-generic/warning.hpp>

#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        template<typename Base>
          inline Base string_to_integral (std::string const& s);

        template<>
          inline
          unsigned long string_to_integral<unsigned long> (std::string const& s)
        {
          return std::stoul (s);
        }
        template<>
          inline
          unsigned short string_to_integral<unsigned short>
          (std::string const& s)
        {
          unsigned long const x (std::stoul (s));

          return (x > std::numeric_limits<unsigned short>::max())
            ? throw std::out_of_range ("stous")
            : fhg::util::suppress_warning::precision_losing_conversion<unsigned short>
                (x, "verified to fit into short range")
            ;
        }
        template<> inline long string_to_integral<long> (std::string const& s)
        {
          return std::stol (s);
        }

        template<typename Base, Base than>
          struct integral_greater_than
        {
          integral_greater_than (std::string const& option)
            : integral_greater_than (string_to_integral<Base> (option))
          {}
          integral_greater_than (Base value)
            : _value (value)
          {
            if (!(_value > than))
            {
              throw ::boost::program_options::invalid_option_value
                ((::boost::format ("'%1%' not greater than %2%.") % _value % than).str());
            }
          }
          operator Base const& () const
          {
            return _value;
          }
        private:
          Base _value;
        };

        template<typename Base, Base than>
          inline
          typename std::enable_if<std::is_integral<Base>::value>::type
                      validate ( ::boost::any& v
                               , const std::vector<std::string>& values
                               , integral_greater_than<Base, than>*
                               , int
                               )
        {
          validate<integral_greater_than<Base, than>> (v, values);
        }
      }
    }
  }
}

#endif
