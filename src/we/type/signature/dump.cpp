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

#include <we/type/signature/dump.hpp>
#include <we/type/signature/traverse.hpp>

#include <fhg/util/xml.hpp>

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
          printer (fhg::util::xml::xmlstream& os)
            : _os (os)
          {}
          void _struct (const std::pair<std::string, structure_type>& s) const
          {
            _os.open ("struct");
            _os.attr ("name", s.first);
            for (const field_type& f : s.second)
            {
              traverse (*this, f);
            }
            _os.close();
          }
          void _field (const std::pair<std::string, std::string>& f) const
          {
            _os.open ("field");
            _os.attr ("name", f.first);
            _os.attr ("type", f.second);
            _os.close();
          }
          void _field_struct
            (const std::pair<std::string, structure_type>& s) const
          {
            traverse (*this, s);
          }
        private:
          fhg::util::xml::xmlstream& _os;
        };
      }

      dump::dump (const structured_type& structured)
        : _structured (structured)
      {}
      std::ostream& dump::operator() (std::ostream& os) const
      {
        fhg::util::xml::xmlstream s (os);

        traverse (printer (s), _structured);

        return os;
      }
      std::ostream& operator<< (std::ostream& os, const dump& d)
      {
        return d (os);
      }

      void dump_to ( fhg::util::xml::xmlstream& s
                   , const structured_type& structured
                   )
      {
        traverse (printer (s), structured);
      }
    }
  }
}
