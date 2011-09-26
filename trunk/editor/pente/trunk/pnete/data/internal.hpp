// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_INTERNAL_HPP
#define _PNETE_DATA_INTERNAL_HPP 1

#include <boost/shared_ptr.hpp>

#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>

#include <pnete/data/change_manager.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class internal
      {
      private:
        ::xml::parse::state::type _state;
        ::xml::parse::type::function_type _function;
        change_manager _change_manager;

      public:
        typedef boost::shared_ptr<internal> ptr;

        explicit internal ();
        explicit internal (const QString& filename);

        ::xml::parse::type::function_type & function ();
        const ::xml::parse::type::function_type & function () const;
        const ::xml::parse::state::key_values_t & context () const;
        const ::xml::parse::state::type & state () const;
      };
    }
  }
}

#endif
