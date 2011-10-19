// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/predicate.hpp>

#include <pnete/ui/graph/item.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace style
        {
          namespace predicate
          {
#define IS_A(_n,_f) \
        bool is_##_n (const graph::item* item) { return item->type() == _f; }

            IS_A(connection,graph::item::connection_graph_type)
            IS_A(transition,graph::item::transition_graph_type)
            IS_A(port,graph::item::port_graph_type)
            IS_A(place,graph::item::place_graph_type)
            IS_A(top_level_port,graph::item::top_level_port_graph_type)

#undef IS_A

            bool starts_with (const std::string& what, const std::string& s)
            {
              std::string::const_iterator what_pos (what.begin());
              const std::string::const_iterator what_end (what.end());
              std::string::const_iterator pos (s.begin());
              const std::string::const_iterator end (s.end());

              while (what_pos != what_end && pos != end && *what_pos == *pos)
                {
                  ++what_pos; ++pos;
                }

              return (what_pos == what_end);
            }
          }
        }
      }
    }
  }
}
