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

#include <we/type/value/positions.hpp>
#include <we/type/value/path/append.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        typedef std::pair<std::list<std::string>, value_type> position_type;
        typedef std::list<position_type> positions_type;

        class visitor_positions : public boost::static_visitor<void>
        {
        public:
          visitor_positions ( std::list<std::string>& path
                            , positions_type& positions
                            )
            : _path (path)
            , _positions (positions)
          {}

          void operator() (structured_type const& s) const
          {
            for (std::pair<std::string, value_type> const& key_value : s)
            {
              path::append const _ (_path, key_value.first);

              boost::apply_visitor (*this, key_value.second);
            }
          }
          template<typename T>
            void operator() (T const& x) const
          {
            _positions.push_back (std::make_pair (_path, x));
          }
        private:
          std::list<std::string>& _path;
          positions_type& _positions;
        };
      }

      positions_type positions (value_type const& value)
      {
        positions_type positions;
        std::list<std::string> path;

        boost::apply_visitor (visitor_positions (path, positions), value);

        return positions;
      }
    }
  }
}
