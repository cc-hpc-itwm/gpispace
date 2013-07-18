// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/show.hpp>
#include <we2/type/signature/traverse.hpp>

#include <fhg/util/print_container.hpp>

#include <boost/bind.hpp>

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
            fhg::util::print_container<structure_type>
              ( _os, s.first, " :: [", ",", "]", boost::ref (s.second)
              , boost::bind (&printer::print, *this, _1)
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
          void print (const field_type& f) const
          {
            traverse (*this, f);
          }
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
      std::ostream& operator<< (std::ostream& os, const show& s)
      {
        return s (os);
      }
    }
  }
}
