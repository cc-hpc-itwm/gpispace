// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/type/signature/show.hpp>
#include <we/type/signature/traverse.hpp>

#include <util-generic/print_container.hpp>

#include <functional>
#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class printer
        {
        public:
          printer (std::ostream& os)
            : _os (os)
          {}
          void _struct (std::pair<std::string, structure_type> const& s) const
          {
            _os << fhg::util::print_container
              ( s.first + " :: [", ", ", "]", s.second
              , [this] (std::ostream& os, field_type const& f) -> std::ostream&
                {
                  traverse (*this, f);
                  return os;
                }
              );
          }
          void _field (std::pair<std::string, std::string> const& f) const
          {
            _os << f.first << " :: " << f.second;
          }
          void _field_struct
            (std::pair<std::string, structure_type> const& s) const
          {
            traverse (*this, s);
          }
        private:
          std::ostream& _os;
        };

        class show_sig : public ::boost::static_visitor<std::ostream&>
        {
        public:
          show_sig (std::ostream& os)
            : _os (os)
          {}
          std::ostream& operator() (std::string const& tname) const
          {
            return _os << tname;
          }
          std::ostream& operator() (structured_type const& s) const
          {
            traverse (printer (_os), s);
            return _os;
          }
        private:
          std::ostream& _os;
        };
      }

      show::show (signature_type const& signature)
        : _signature (signature)
      {}
      std::ostream& show::operator() (std::ostream& os) const
      {
        return ::boost::apply_visitor (show_sig (os), _signature);
      }
    }
  }
}
