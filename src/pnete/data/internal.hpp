// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <pnete/data/internal.fwd.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/state.hpp>

#include <pnete/data/change_manager.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal_type
      {
      public:
        enum kind {expression, module_call, net};

        explicit internal_type (const kind&);
        explicit internal_type (const QString& filename);

        const ::xml::parse::id::ref::function& function() const;
        change_manager_t& change_manager();

      private:
        ::xml::parse::state::type _state;
        ::xml::parse::id::ref::function _function;
        change_manager_t _change_manager;
      };
    }
  }
}
