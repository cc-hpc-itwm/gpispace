// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_PREDICATE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_PREDICATE_HPP 1

#include <string>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class item;

        namespace style
        {
          namespace predicate
          {
            bool is_connection (const graph::item*);
            bool is_port (const graph::item*);
            bool is_transition (const graph::item*);
            bool is_place (const graph::item*);
            bool is_top_level_port (const graph::item*);
            bool starts_with (const std::string&, const std::string&);
          }
        }
      }
    }
  }
}

#endif
