// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_TRAVERSE_DISPLAY_HPP
#define _PNETE_TRAVERSE_DISPLAY_HPP 1

#include <xml/parse/types.hpp>

#include <pnete/data/proxy.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Scene;
      class Transition;
    }
    namespace weaver
    {
      namespace display
      {
        class weaver
        {
        public:
          explicit weaver (data::proxy::function_type &);

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

          data::proxy::type* proxy () const;

        private:
          data::proxy::type* _proxy;
          data::proxy::function_type& _fun;

          graph::Scene* _scene;
        };
      }
    }
  }
}

#endif
