// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_INTERNAL_HPP
#define _PNETE_DATA_INTERNAL_HPP 1

#include <boost/shared_ptr.hpp>

#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>

#include <pnete/data/undo_redo_manager.hpp>

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
        undo_redo_manager _undo_redo_manager;

      public:
        typedef boost::shared_ptr<internal> ptr;

        explicit internal ();
        explicit internal (const QString& filename);

        QString name () const;
        ::xml::parse::type::function_type & function ();
        const ::xml::parse::type::function_type & function () const;
        const ::xml::parse::state::key_values_t & context () const;
        const ::xml::parse::state::type & state () const;
      };
    }
  }
}

#endif
