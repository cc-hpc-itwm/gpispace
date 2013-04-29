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
        class show_field : public boost::static_visitor<std::ostream&>
        {
        public:
          show_field (std::ostream&);
          std::ostream& operator() (const std::pair< std::string
                                                   , std::string
                                                   >&
                                   ) const;
          std::ostream& operator() (const structured_type&) const;
        private:
          std::ostream& _os;
        };
        class show_struct : public boost::static_visitor<std::ostream&>
        {
        public:
          show_struct (std::ostream&);
          std::ostream& operator() (const std::pair< std::string
                                                   , std::list<field_type>
                                                   >&
                                   ) const;
        private:
          std::ostream& _os;
          void print (const field_type&) const;
        };

        class show_sig : public boost::static_visitor<std::ostream&>
        {
        public:
          show_sig (std::ostream&);
          std::ostream& operator() (const std::string&) const;
          std::ostream& operator() (const structured_type&) const;
        private:
          std::ostream& _os;
        };

        show_field::show_field (std::ostream& os)
          : _os (os)
        {}
        std::ostream& show_field::operator() (const std::pair< std::string
                                                             , std::string
                                                             >& f
                                             ) const
        {
          return _os << f.first << " :: " << f.second;
        }
        std::ostream& show_field::operator() (const structured_type& s) const
        {
          return boost::apply_visitor (show_struct (_os), s);
        }

        show_struct::show_struct (std::ostream& os)
          : _os (os)
        {}
        std::ostream&
        show_struct::operator() (const std::pair< std::string
                                                , std::list<field_type>
                                                >& s
                                ) const
        {
          return fhg::util::print_container<std::list<field_type> >
            ( _os, s.first, " :: [", ",", "]", boost::ref (s.second)
            , boost::bind (&show_struct::print, *this, _1)
            );
        }
        void show_struct::print (const field_type& f) const
        {
          boost::apply_visitor (show_field (_os), f);
        }

        show_sig::show_sig (std::ostream& os)
          : _os (os)
        {}
        std::ostream& show_sig::operator() (const std::string& tname) const
        {
          return _os << tname;
        }
        std::ostream& show_sig::operator() (const structured_type& s) const
        {
          return boost::apply_visitor (show_struct (_os), s);
        }
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
