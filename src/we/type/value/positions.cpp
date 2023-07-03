// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/path/append.hpp>
#include <we/type/value/positions.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        using position_type = std::pair<std::list<std::string>, value_type>;
        using positions_type = std::list<position_type>;

        class visitor_positions : public ::boost::static_visitor<void>
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

              ::boost::apply_visitor (*this, key_value.second);
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

        ::boost::apply_visitor (visitor_positions (path, positions), value);

        return positions;
      }
    }
  }
}
