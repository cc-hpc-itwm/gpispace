// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_BASE_HPP
#define _FHG_PNETE_DATA_HANDLE_BASE_HPP 1

#include <pnete/data/handle/base.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/internal.fwd.hpp>

#include <we/type/property.fwd.hpp>

class QObject;
class QPointF;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class base
        {
        public:
          base (internal_type* document);
          virtual ~base() { }

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          //! \note This is not nice, as not all elements actually can
          //! move. And even worse, as only port can ever have outer == false.
          virtual void move ( const QObject* sender
                            , const QPointF& position
                            , const bool outer
                            ) const;

          virtual void no_undo_move ( const QObject* sender
                                    , const QPointF& position
                                    ) const;

          void connect_to_change_mgr ( const QObject* object
                                     , const char* signal
                                     , const char* slot
                                     , const char* arguments
                                     ) const;
          void connect_to_change_mgr ( const QObject* object
                                     , const char* signal_slot
                                     , const char* arguments
                                     ) const;

          change_manager_t& change_manager() const;
          internal_type* document() const;

        private:
          internal_type* _document;
        };
      }
    }
  }
}

#endif
