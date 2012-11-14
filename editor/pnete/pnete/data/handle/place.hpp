// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_PLACE_HPP
#define _FHG_PNETE_DATA_HANDLE_PLACE_HPP 1

#include <pnete/data/handle/place.fwd.hpp>

#include <pnete/data/change_manager.fwd.hpp>
#include <pnete/data/handle/base.hpp>
#include <pnete/data/handle/net.fwd.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/place.fwd.hpp>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        class place : public base
        {
        public:
          place ( const ::xml::parse::id::ref::place& place
                , change_manager_t& change_manager
                );

          const ::xml::parse::type::place_type& get() const;
          ::xml::parse::type::place_type& get_ref() const;

          bool operator== (const place& other) const;

          void remove (const QObject* sender) const;

          const ::xml::parse::id::ref::place& id() const;

          net parent() const;

        private:
          ::xml::parse::id::ref::place _id;
        };
      }
    }
  }
}

#endif
