// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/boost/program_options/validators.hpp>

#include <util-generic/ostream/modifier.hpp>

#include <boost/format.hpp>

#include <exception>
#include <stdexcept>
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
            try
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
                  (( ::boost::format ("Missing '/' in '%1%'")
                   % option.substr (pos)
                   ).str()
                  );
              }

              ++pos;

              _denumerator = parse (option, &pos);

              if (pos < option.size())
              {
                throw std::invalid_argument
                  (( ::boost::format ("Additional input '%1%'")
                   % option.substr (pos)
                   ).str());
              }
            }
            catch (...)
            {
              std::throw_with_nested
                ( ::boost::program_options::invalid_option_value
                  { ( ::boost::format
                      ("Parse error: Input '%1%': Expected format: '<%2%>/<%2%>'.")
                      % option
                      % typeid (T).name()
                    ).str()
                  }
                );
            }
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
          inline void validate ( ::boost::any& v
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
