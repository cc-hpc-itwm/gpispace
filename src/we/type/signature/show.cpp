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
          void _struct (const std::pair<std::string, structure_type>& s) const
          {
            _os << fhg::util::print_container
              ( s.first + " :: [", ", ", "]", s.second
              , [this] (std::ostream& os, const field_type& f) -> std::ostream&
                {
                  traverse (*this, f);
                  return os;
                }
              );
          }
          void _field (const std::pair<std::string, std::string>& f) const
          {
            _os << f.first << " :: " << f.second;
          }
          void _field_struct
            (const std::pair<std::string, structure_type>& s) const
          {
            traverse (*this, s);
          }
        private:
          std::ostream& _os;
        };

        class show_sig : public boost::static_visitor<std::ostream&>
        {
        public:
          show_sig (std::ostream& os)
            : _os (os)
          {}
          std::ostream& operator() (const std::string& tname) const
          {
            return _os << tname;
          }
          std::ostream& operator() (const structured_type& s) const
          {
            traverse (printer (_os), s);
            return _os;
          }
        private:
          std::ostream& _os;
        };
      }

      show::show (const signature_type& signature)
        : _signature (signature)
      {}
      std::ostream& show::operator() (std::ostream& os) const
      {
        return boost::apply_visitor (show_sig (os), _signature);
      }
    }
  }
}
