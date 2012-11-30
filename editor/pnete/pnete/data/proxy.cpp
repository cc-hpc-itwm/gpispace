// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/proxy.hpp>

#include <boost/variant.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace proxy
      {
        namespace
        {
          class visitor_root : public boost::static_visitor<internal_type*>
          {
          public:
            template<typename T>
            internal_type* operator () (const T& x) const
            {
              return x.root();
            }
          };

          class visitor_function
            : public boost::static_visitor<const handle::function&>
          {
          public:
            template<typename T>
              const handle::function& operator() (const T& x) const
            {
              return x.function();
            }
          };
        }

        handle::function function (const type& proxy)
        {
          return boost::apply_visitor (visitor_function(), proxy);
        }

        internal_type* root (const type& proxy)
        {
          return boost::apply_visitor (visitor_root(), proxy);
        }
      }
    }
  }
}
