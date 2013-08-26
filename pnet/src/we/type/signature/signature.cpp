// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class visitor_signature : public boost::static_visitor<signature_type>
        {
        public:
          signature_type operator() (const std::pair< std::string
                                                    , std::string
                                                    >& f
                                    ) const
          {
            return f.second;
          }
          signature_type operator() (const structured_type& s) const
          {
            return s;
          }
        };
      }

      signature_type signature (const field_type& s)
      {
        return boost::apply_visitor (visitor_signature(), s);
      }
    }
  }
}
