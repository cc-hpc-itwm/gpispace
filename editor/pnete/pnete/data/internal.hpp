// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_INTERNAL_HPP
#define _PNETE_DATA_INTERNAL_HPP 1

#include <xml/parse/type/function.hpp>
#include <xml/parse/state.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/proxy.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal_type
      {
      private:
        ::xml::parse::state::type _state;
        ::xml::parse::type::function_type _function;
        change_manager_t _change_manager;
        proxy::type _root_proxy;

        proxy::type* create_proxy();

      public:
        enum kind {expression, module_call, net};

        explicit internal_type (const kind& = internal_type::expression);
        explicit internal_type (const QString& filename);

        ::xml::parse::type::function_type & function ();
        const ::xml::parse::type::function_type & function () const;
        const ::xml::parse::state::key_values_t & context () const;
        const ::xml::parse::state::type & state () const;
        ::xml::parse::state::type & state ();
        change_manager_t& change_manager();
        const proxy::type& root_proxy() const;
        proxy::type& root_proxy();
      };
    }
  }
}

#endif
