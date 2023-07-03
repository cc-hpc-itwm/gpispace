// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/callback/bracket.hpp>
#include <util-generic/ostream/callback/range.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <boost/optional.hpp>

#include <iterator>
#include <ostream>

namespace fhg
{
  namespace util
  {
    template<typename Iterator, typename Separator>
      class join_iterator : public ostream::modifier
    {
      using value_type = typename std::iterator_traits<Iterator>::value_type;

    public:
      join_iterator ( Iterator begin
                    , Iterator end
                    , Separator separator
                    , ostream::callback::print_function<value_type> const& print
                    = ostream::callback::id<value_type>()
                    , std::string const& open = ""
                    , std::string const& close = ""
                    )
        : _begin (begin)
        , _end (end)
        , _separator (separator)
        , _print (print)
        , _open (open)
        , _close (close)
      {}
      std::ostream& operator() (std::ostream& os) const override
      {
        os << _open;

        ostream::callback::range<Iterator, Separator> (_separator, _print)
          (os, {_begin, _end});

        os << _close;

        return os;
      }

    private:
      Iterator const _begin;
      Iterator const _end;
      Separator const _separator;
      ostream::callback::print_function<value_type> const _print;
      std::string const _open;
      std::string const _close;
    };

    template<typename C, typename Separator>
      class join_reference : public ostream::modifier
    {
      using size_type = Difference<C>;

    public:
      join_reference ( C const& container
                     , Separator separator
                     , ostream::callback::print_function<Value<C>> const& print
                     = ostream::callback::id<Value<C>>()
                     , std::string const& open = ""
                     , std::string const& close = ""
                     , ::boost::optional<size_type> const& max_elements_to_print
                     = ::boost::none
                     )
        : _container (container)
        , _separator (separator)
        , _print (print)
        , _open (open)
        , _close (close)
        , _max_elements_to_print (max_elements_to_print)
      {}
      std::ostream& operator() (std::ostream& os) const override
      {
        size_type const number_of_elements
          ( !_max_elements_to_print
          ? 0
          : std::distance (std::begin (_container), std::end (_container))
          );

        if (!!_max_elements_to_print)
        {
          os << "[" << number_of_elements << "]: ";
        }

        return os
          << join_iterator<Iterator<C>, Separator>
          ( std::begin (_container)
          , ( !!_max_elements_to_print
            ? std::next ( std::begin (_container)
                        , std::min ( number_of_elements
                                   , _max_elements_to_print.get()
                                   )
                        )
            : std::end (_container)
            )
          , _separator
          , _print
          , _open
          , ( (  !!_max_elements_to_print
              && number_of_elements > _max_elements_to_print.get()
              )
            ? "..." + _close
            : _close
            )
          );
      }

    private:
      C const& _container;
      Separator const _separator;
      ostream::callback::print_function<Value<C>> const _print;
      std::string const _open;
      std::string const _close;
      ::boost::optional<size_type> _max_elements_to_print;
    };

    template<typename Container, typename Separator>
      constexpr join_reference<Container, Separator>
        join ( Container const& c
             , Separator s
             , ostream::callback::print_function<Value<Container>> const& print
             , std::string const& open
             , std::string const& close
             , ::boost::optional<Difference<Container>> const& max_elements_to_print
             )
    {
      return join_reference<Container, Separator>
        (c, s, print, open, close, max_elements_to_print);
    }

    template<typename Iterator, typename Separator>
      constexpr join_iterator<Iterator, Separator>
        join ( Iterator begin, Iterator end
             , Separator separator
             , std::string const& local_open
             , std::string const& local_close
             )
    {
      return join_iterator<Iterator, Separator>
        ( begin, end, separator
        , ostream::callback::bracket<typename Iterator::value_type>
            (local_open, local_close)
        );
    }
  }
}
