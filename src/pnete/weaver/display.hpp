// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/handle/net.fwd.hpp>
#include <pnete/data/handle/place.fwd.hpp>
#include <pnete/data/handle/port.fwd.hpp>
#include <pnete/data/handle/transition.fwd.hpp>
#include <pnete/data/manager.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>
#include <pnete/ui/graph/transition.fwd.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace display
      {
        ui::graph::scene_type* net ( data::manager&
                                   , const data::handle::net&
                                   , const data::handle::function& parent
                                   );

        void transition (const data::handle::transition&, ui::graph::scene_type*);
        void place (const data::handle::place&, ui::graph::scene_type*);
        void top_level_port (const data::handle::port&, ui::graph::scene_type*);
        void port (const data::handle::port&, ui::graph::transition_item*);
      }
    }
  }
}
