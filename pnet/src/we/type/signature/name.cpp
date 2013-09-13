// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/name.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class visitor_name_structured
          : public boost::static_visitor<const std::string&>
        {
        public:
          const std::string& operator() (const std::pair< std::string
                                                        , structure_type
                                                        >& s
                                        ) const
          {
            return s.first;
          }
        };

        class visitor_name : public boost::static_visitor<const std::string&>
        {
        public:
          const std::string& operator() (const std::pair< std::string
                                                        , std::string
                                                        >& f
                                        ) const
          {
            return f.first;
          }
          const std::string& operator() (const structured_type& s) const
          {
            return boost::apply_visitor (visitor_name_structured(), s);
          }
        };
      }

      const std::string& name (const field_type& s)
      {
        return boost::apply_visitor (visitor_name(), s);
      }
    }
  }
}
