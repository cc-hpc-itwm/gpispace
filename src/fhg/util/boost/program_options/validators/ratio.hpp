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

#pragma once

#include <fhg/util/boost/program_options/validators.hpp>

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <boost/format.hpp>

#include <string>
#include <typeinfo>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        namespace detail
        {
          namespace ratio
          {
            template<typename> struct parser;

#define PARSER(_type, _parse)                                   \
            template<> struct parser<_type>                     \
            {                                                   \
              static _type parse ( std::string const& input     \
                                 , std::size_t* pos             \
                                 )                              \
              {                                                 \
                return _parse(input, pos);                      \
              }                                                 \
            }

            PARSER (int, std::stoi);
            PARSER (long long, std::stoll);
            PARSER (long, std::stol);
            PARSER (unsigned long long, std::stoull);
            PARSER (unsigned long, std::stoul);

#undef PARSER
          }
        }

        template<typename T>
          struct ratio : public ostream::modifier
        {
        public:
          T const& numerator() const
          {
            return _numerator;
          }
          T const& denumerator() const
          {
            return _denumerator;
          }

          ratio (T const& numerator, T const& denumerator)
            : _numerator (numerator)
            , _denumerator (denumerator)
          {}
          ratio (std::string const& option)
          {
            fhg::util::nest_exceptions<boost::program_options::invalid_option_value>
              ([this, &option]()
               {
                 auto&& parse
                   ([](std::string const& input, std::size_t* pos)
                    {
                      std::size_t pos_parse (0);

                      T const x
                        { detail::ratio::parser<T>::parse
                            (input.substr (*pos), &pos_parse)
                        };

                      *pos += pos_parse;

                      return x;
                   }
                   );

                 std::size_t pos (0);

                 _numerator = parse (option, &pos);

                 while (pos < option.size() && std::isspace (option.at (pos)))
                 {
                   ++pos;
                 }

                 if (! (pos < option.size()))
                 {
                   throw std::invalid_argument ("Missing '/'");
                 }

                 if (option.at (pos) != '/')
                 {
                   throw std::invalid_argument
                     (( boost::format ("Missing '/' in '%1%'")
                      % option.substr (pos)
                      ).str()
                     );
                 }

                 ++pos;

                 _denumerator = parse (option, &pos);

                 if (pos < option.size())
                 {
                   throw std::invalid_argument
                     (( boost::format ("Additional input '%1%'")
                      % option.substr (pos)
                      ).str());
                 }
               }
              , ( boost::format
                  ("Parse error: Input '%1%': Expected format: '<%2%>/<%2%>'.")
                % option
                % typeid (T).name()
                ).str()
              );
          }

          virtual std::ostream& operator() (std::ostream& os) const override
          {
            return os << _numerator << "/" << _denumerator;
          }

        private:
          T _numerator;
          T _denumerator;
        };

        template<typename R>
          inline void validate ( boost::any& v
                               , const std::vector<std::string>& values
                               , ratio<R>*
                               , int
                               )
        {
          validate<ratio<R>> (v, values);
        }
      }
    }
  }
}
