// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/dump.hpp>

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
        class show_field : public boost::static_visitor<>
        {
        public:
          show_field (fhg::util::xml::xmlstream&);
          void operator() (const std::pair<std::string, std::string>&) const;
          void operator() (const structured_type&) const;
        private:
          fhg::util::xml::xmlstream& _os;
        };
        class show_struct : public boost::static_visitor<>
        {
        public:
          show_struct (fhg::util::xml::xmlstream&);
          void operator() (const std::pair< std::string
                                          , std::list<field_type>
                                          >&
                          ) const;
        private:
          fhg::util::xml::xmlstream& _os;
          void print (const field_type&) const;
        };

        show_field::show_field (fhg::util::xml::xmlstream& os)
          : _os (os)
        {}
        void show_field::operator() (const std::pair< std::string
                                                    , std::string
                                                    >& f
                                    ) const
        {
          _os.open ("field");
          _os.attr ("name", f.first);
          _os.attr ("type", f.second);
          _os.close();
        }
        void show_field::operator() (const structured_type& s) const
        {
          return boost::apply_visitor (show_struct (_os), s);
        }

        show_struct::show_struct (fhg::util::xml::xmlstream& os)
          : _os (os)
        {}
        void show_struct::operator() (const std::pair< std::string
                                                     , std::list<field_type>
                                                     >& s
                                     ) const
        {
          _os.open ("struct");
          _os.attr ("name", s.first);
          BOOST_FOREACH (const field_type& f, s.second)
          {
            boost::apply_visitor (show_field (_os), f);
          }
          _os.close();
        }
      }

      dump::dump (const structured_type& structured)
        : _structured (structured)
      {}
      std::ostream& dump::operator() (std::ostream& os) const
      {
        fhg::util::xml::xmlstream s (os);

        boost::apply_visitor (show_struct (s), _structured);

        return os;
      }
      std::ostream& operator<< (std::ostream& os, const dump& d)
      {
        return d (os);
      }
    }
  }
}
