// mirko.rahn@itwm.fhg.de

#include <we2/type/compat.sig.hpp>

namespace pnet
{
  namespace type
  {
    namespace compat
    {
      namespace
      {
        class field : public boost::static_visitor<signature::field_type>
        {
        public:
          field (const std::string& name)
            : _name (name)
          {}

          signature::field_type operator() (const std::string& t) const
          {
            return std::make_pair (_name, t);
          }
          signature::field_type operator()
            (const ::signature::structured_t& s) const
          {
            signature::structure_type s2;

            for ( ::signature::structured_t::const_iterator pos (s.begin())
                ; pos != s.end()
                ; ++pos
                )
            {
              s2.push_back (boost::apply_visitor ( field (pos->first)
                                                 , pos->second
                                                 )
                           );
            }

            return signature::structured_type (std::make_pair (_name, s2));
          }
        private:
          const std::string& _name;
        };

        class sig2sig2
          : public boost::static_visitor<signature::signature_type>
        {
        public:
          sig2sig2 (const ::signature::type& sig)
            : _sig (sig)
          {}

          signature::signature_type operator() (const std::string& t) const
          {
            return t;
          }
          signature::signature_type operator()
            (const ::signature::structured_t& s) const
          {
            signature::structure_type s2;

            for ( ::signature::structured_t::const_iterator pos (s.begin())
                ; pos != s.end()
                ; ++pos
                )
            {
              s2.push_back (boost::apply_visitor ( field (pos->first)
                                                 , pos->second
                                                 )
                           );
            }

            return signature::structured_type (std::make_pair ( _sig.nice()
                                                              , s2
                                                              )
                                              );
          }
        private:
          const ::signature::type& _sig;
        };
      }

      signature::signature_type COMPAT (const ::signature::type& x)
      {
        return boost::apply_visitor (sig2sig2 (x), x.desc());
      }
    }
  }
}
