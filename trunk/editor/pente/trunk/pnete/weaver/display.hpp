// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_TRAVERSE_DISPLAY_HPP
#define _PNETE_TRAVERSE_DISPLAY_HPP 1

#include <xml/parse/types.hpp>

#include <pnete/data/proxy.hpp>

#include <pnete/weaver/weaver.hpp>

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
      class display
      {
      public:
        explicit display (data::proxy::function_type &);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

        data::proxy::type* proxy () const;

        XMLTYPE(function_type)& get_function
        (const XMLTYPE(transition_type::f_type) &);

      private:
        data::proxy::type* _proxy;
        data::proxy::function_type& _fun;

        struct
        {
          XMLTYPE(ports_type) in;
          XMLTYPE(ports_type) out;
        } _function_state;

        graph::Scene* _scene;
        graph::Transition* _current;
      };

      namespace visitor
      {
        typedef XMLTYPE(function_type) function_type;

        function_type& default_fun();

        class get_function
          : public boost::static_visitor<function_type&>
        {
        public:
          function_type& operator () (const function_type& fun) const
          {
            return const_cast<function_type &> (fun);
          }

          function_type& operator () (const XMLTYPE(use_type)& use) const
          {
            //! \todo implement lookup in parent net
            return default_fun();
          }
        };
      }
    }
  }
}

#endif
