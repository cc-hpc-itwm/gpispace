// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/base.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/internal.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <QObject>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        base::base (internal_type* document)
          : _document (document)
        { }

        void base::set_property ( const QObject* sender
                                , const ::we::type::property::key_type&
                                , const ::we::type::property::value_type&
                                ) const
        {
          throw fhg::util::backtracing_exception
            ("handle::base::set_property() called instead "
            "of overloaded version.");
        }

        void base::move ( const QObject* sender
                        , const QPointF& position
                        ) const
        {
          throw fhg::util::backtracing_exception
            ("handle::base::move() called instead of overloaded version.");
        }

        void base::no_undo_move ( const QObject* sender
                                , const QPointF& position
                                ) const
        {
          throw fhg::util::backtracing_exception
            ("handle::base::no_undo_move() called instead of "
            "overloaded version.");
        }

        change_manager_t& base::change_manager() const
        {
          return document()->change_manager();
        }
        internal_type* base::document() const
        {
          return _document;
        }

        void base::connect_to_change_mgr ( const QObject* object
                                         , const char* signal
                                         , const char* slot
                                         , const char* arguments
                                         ) const
        {

#define STRINGIFY_HELPER(s) #s
#define STRINGIFY(s) STRINGIFY_HELPER(s)

          QObject::connect ( &change_manager()
                           , ( std::string (STRINGIFY (QSIGNAL_CODE))
                             + signal
                             + "(const QObject*," + arguments + ")"
                             ).c_str()
                           , object
                           , ( std::string (STRINGIFY (QSLOT_CODE))
                             + slot
                             + "(const QObject*," + arguments + ")"
                             ).c_str()
                           , Qt::DirectConnection
                           );

#undef STRINGIFY
#undef STRINGIFY_HELPER

        }

        void base::connect_to_change_mgr ( const QObject* object
                                         , const char* signal_slot
                                         , const char* arguments
                                         ) const
        {
          connect_to_change_mgr (object, signal_slot, signal_slot, arguments);
        }
      }
    }
  }
}
