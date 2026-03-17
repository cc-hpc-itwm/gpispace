#pragma once

#include <gspc/util/boost/program_options/validators.hpp>

#include <gspc/util/ostream/modifier.hpp>

#include <fmt/format.h>

#include <exception>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace gspc
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
                  ( fmt::format ("Missing '/' in '{}'", option.substr (pos))
                  );
              }

              ++pos;

              _denumerator = parse (option, &pos);

              if (pos < option.size())
              {
                throw std::invalid_argument
                  ( fmt::format ("Additional input '{}'", option.substr (pos))
                  );
              }
            }
            catch (...)
            {
              std::throw_with_nested
                ( ::boost::program_options::invalid_option_value
                  { fmt::format
                    ( "Parse error: Input '{}': Expected format: '<{}>/{}>'."
                    , option
                    , typeid (T).name()
                    , typeid (T).name()
                    )
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
