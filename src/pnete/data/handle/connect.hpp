// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_CONNECT_HPP
#define _FHG_PNETE_DATA_HANDLE_CONNECT_HPP 1

#include <pnete/data/handle/connect.fwd.hpp>

#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>

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
          connect (const connect_meta_base::id_type&, internal_type*);

          bool is_in() const;
          bool is_out() const;
          bool is_read() const;

          void is_read (const bool& s) const;

          port resolved_port() const;
          place resolved_place() const;

          void remove() const;

          virtual void set_property ( const ::we::type::property::path_type&
                                    , const ::we::type::property::value_type&
                                    ) const override;

          using connect_meta_base::operator==;
        };
      }
    }
  }
}

#endif
