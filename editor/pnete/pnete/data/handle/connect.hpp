// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_CONNECT_HPP
#define _FHG_PNETE_DATA_HANDLE_CONNECT_HPP 1

#include <pnete/data/handle/connect.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/connect.fwd.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        typedef meta_base < ::xml::parse::id::ref::connect
                          , ::xml::parse::type::connect_type
                          > connect_meta_base;
        class connect : public connect_meta_base
        {
        public:
          connect ( const connect_meta_base::id_type& id
                  , change_manager_t& change_manager
                  );

          bool is_in() const;
          bool is_out() const;
          bool is_read() const;

          void remove (const QObject* sender) const;

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          using connect_meta_base::operator==;
        };
      }
    }
  }
}

#endif
