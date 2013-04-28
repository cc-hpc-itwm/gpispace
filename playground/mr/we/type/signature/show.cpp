// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/show.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
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
          visitor_show (std::ostream& os)
            : _os (os)
          {}

          template<typename T>
          std::ostream& operator() (const T&) const
          {
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
        return boost::apply_visitor (visitor_show (os), _signature);
      }

      std::ostream& operator<< (std::ostream& os, const show& s)
      {
        return s (os);
      }
    }
  }
}
