// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
          void _struct (std::pair<std::string, structure_type> const& s) const
          {
            _os.open ("struct");
            _os.attr ("name", s.first);
            for (field_type const& f : s.second)
            {
              traverse (*this, f);
            }
            _os.close();
          }
          void _field (std::pair<std::string, std::string> const& f) const
          {
            _os.open ("field");
            _os.attr ("name", f.first);
            _os.attr ("type", f.second);
            _os.close();
          }
          void _field_struct
            (std::pair<std::string, structure_type> const& s) const
          {
            traverse (*this, s);
          }
        private:
          fhg::util::xml::xmlstream& _os;
        };
      }

      dump::dump (structured_type const& structured)
        : _structured (structured)
      {}
      std::ostream& dump::operator() (std::ostream& os) const
      {
        fhg::util::xml::xmlstream s (os);

        traverse (printer (s), _structured);

        return os;
      }
      std::ostream& operator<< (std::ostream& os, dump const& d)
      {
        return d (os);
      }

      void dump_to ( fhg::util::xml::xmlstream& s
                   , structured_type const& structured
                   )
      {
        traverse (printer (s), structured);
      }
    }
  }
}
