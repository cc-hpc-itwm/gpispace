// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_PNETE_WEAVER_DISPLAY_HPP
#define FHG_PNETE_WEAVER_DISPLAY_HPP

#include <pnete/data/internal.fwd.hpp>
#include <pnete/data/proxy.fwd.hpp>
#include <pnete/ui/graph/place.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>
#include <pnete/ui/graph/transition.fwd.hpp>

#include <xml/parse/id/types.fwd.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace display
      {
        data::proxy::type function
          (const ::xml::parse::id::ref::function&, data::internal_type*);

        void transition ( const ::xml::parse::id::ref::transition&
                        , data::internal_type*
                        , ui::graph::scene_type*
                        );

        void place (const ::xml::parse::id::ref::place&, ui::graph::place_item*);

        void top_level_port ( const ::xml::parse::id::ref::port&
                            , ui::graph::scene_type*
                            , data::internal_type*
                            );

      }
    }
  }
}

#endif
