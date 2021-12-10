// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/boost/program_options/separated_argument_list_parser.hpp>

#include <algorithm>
#include <iterator>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        separated_argument_list_parser::separated_argument_list_parser
          (decltype (_sections) sections)
            : _sections (std::move (sections))
        {}
        separated_argument_list_parser::separated_argument_list_parser
          ( std::string open
          , std::string close
          , std::string option
          )
            : _sections ({{open, {close, option}}})
        {}

        namespace
        {
          //! prefer pred1 over pred2
          template<typename IT, typename Pred1, typename Pred2>
            IT find (IT begin, IT end, Pred1 pred1, Pred2 pred2)
          {
            auto const x (std::find_if (begin, end, pred1));

            return x != end ? x : std::find_if (begin, end, pred2);
          }

          bool is_prefix_of (std::string const& p, std::string const& x)
          {
            return p.size() <= x.size()
              && std::equal (p.cbegin(), p.cend(), x.cbegin())
              ;
          }
          bool is_suffix_of (std::string const& s, std::string const& x)
          {
            return s.size() <= x.size()
              && std::equal (s.crbegin(), s.crend(), x.crbegin())
              ;
          }
          bool remove_prefix (std::string const& p, std::string& x)
          {
            x.erase (0, p.size());

            return x.empty();
          }
          bool remove_suffix (std::string const& s, std::string& x)
          {
            x.erase (x.size() - s.size(), x.size());

            return x.empty();
          }
        }

        std::vector<::boost::program_options::option>
          separated_argument_list_parser::operator()
            (std::vector<std::string>& args) const
        {
          auto const begin (std::begin (args));

  #define AUTO decltype (*_sections.cbegin())

          auto const section
            ( find ( _sections.cbegin(), _sections.cend()
                  , [&] (AUTO section_description)
                    {
                      return operator== (section_description.first, *begin);
                    }
                  , [&] (AUTO section_description)
                    {
                      return is_prefix_of (section_description.first, *begin);
                    }
                  )
            );

  #undef AUTO

          if (section == _sections.end())
          {
            return {};
          }

          auto const& open (section->first);
          auto const& close (section->second.first);

          auto const begin_empty (remove_prefix (open, *begin));

  #define AUTO decltype (*begin)

          auto const end
            ( find ( begin_empty ? std::next (begin) : begin, std::end (args)
                  , [&] (AUTO arg)
                    {
                      return operator== (close, arg);
                    }
                  , [&] (AUTO arg)
                    {
                      return is_suffix_of (close, arg);
                    }
                  )
            );

  #undef AUTO

          auto const end_empty
            (end != std::end (args) && remove_suffix (close, *end));

          std::vector<std::string> const
            values ( begin_empty ? std::next (begin) : begin
                  , end == args.end() || end_empty ? end : std::next (end)
                  );
          args.erase (begin, end == args.end() ? end : std::next (end));

          if (values.empty())
          {
            return {};
          }

          //! \note workaround for https://svn.boost.org/trac/boost/ticket/11645
          ::boost::program_options::option option (section->second.second, values);
          option.position_key = -1;
          return {option};
        }
      }
    }
  }
}
