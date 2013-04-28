// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/show.hpp>

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
        class visitor_show : public boost::static_visitor<std::ostream&>
        {
        public:
          visitor_show (std::ostream& os);
          std::ostream& os() const;
          std::ostream& operator() (const fields_type&) const;
        private:
          std::ostream& _os;

          void print_item (const fields_type::value_type&) const;
        };

        class visitor_show_rec : public boost::static_visitor<std::ostream&>
        {
        public:
          visitor_show_rec (const visitor_show& vs)
          : _vs (vs)
          {}

          std::ostream& operator() (const std::string& tname) const
          {
            return _vs.os() << tname;
          }
          std::ostream& operator() (const signature_type& sig) const
          {
            return boost::apply_visitor (_vs, sig);
          }

        private:
          const visitor_show& _vs;
        };

        visitor_show::visitor_show (std::ostream& os)
          : _os (os)
        {}
        std::ostream& visitor_show::os() const
        {
          return _os;
        }
        std::ostream& visitor_show::operator() (const fields_type& fields) const
        {
          return fhg::util::print_container<fields_type>
            ( _os, "", "[", ",", "]", boost::ref (fields)
            , boost::bind (&visitor_show::print_item, *this, _1)
            );
        }
      }
      void visitor_show::print_item (const fields_type::value_type& f) const
      {
        _os << f.first << " :: ";

        boost::apply_visitor (visitor_show_rec (*this), f.second);
      }

      show::show (const signature_type& signature)
        : _signature (signature)
      {}

      std::ostream& show::operator() (std::ostream& os) const
      {
        return boost::apply_visitor (visitor_show (os), _signature);
      }

      std::ostream& operator<< (std::ostream& os, const show& s)
      {
        return s (os);
      }
    }
  }
}
