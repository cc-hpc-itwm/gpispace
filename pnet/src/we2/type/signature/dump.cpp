// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/dump.hpp>
#include <we2/type/signature/traverse.hpp>

#include <fhg/util/print_container.hpp>
#include <fhg/util/xml.hpp>

#include <boost/bind.hpp>

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
            BOOST_FOREACH (const field_type& f, s.second)
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
