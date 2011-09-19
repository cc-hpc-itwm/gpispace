// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/internal.hpp>

#include <xml/parse/parser.hpp>

#include <QObject>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      internal::internal ()
        : _state ()
        , _function ()
        , _undo_redo_manager (*this)
      {}

      internal::internal (const QString& filename)
        : _state ()
        , _function (::xml::parse::just_parse (_state, filename.toStdString()))
        , _undo_redo_manager (*this)
      {}

      QString internal::name () const
      {
        return _function.name
          ? QString ((*_function.name).c_str())
          : QObject::tr ("noname")
          ;
      }

      ::xml::parse::type::function_type & internal::function ()
      {
        return const_cast< ::xml::parse::type::function_type &> (_function);
      }

      const ::xml::parse::type::function_type & internal::function () const
      {
        return _function;
      }
      const ::xml::parse::state::key_values_t & internal::context () const
      {
        return _state.key_values();
      }
      const ::xml::parse::state::type & internal::state () const
      {
        return _state;
      }
    }
  }
}
