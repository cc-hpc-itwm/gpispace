// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_INTEGRAL_GREATER_THAN_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_INTEGRAL_GREATER_THAN_HPP

#include <fhg/util/boost/program_options/validators.hpp>

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
              throw boost::program_options::invalid_option_value
                ((boost::format ("'%1%' not greater than %2%.") % _value % than).str());
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
                      validate ( boost::any& v
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
