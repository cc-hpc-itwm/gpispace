// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/predicate.hpp>

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/place.hpp>

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
            predicate::predicate (const function_type& function)
              : _function (function)
            {}
            bool predicate::operator () (const base_item* gi) const
            {
              return _function (gi);
            }

#define IS_A(_n,_f) \
        bool is_##_n (const base_item* gi) { return gi->type() == _f; }

            IS_A(connection,base_item::connection_graph_type)
            IS_A(transition,base_item::transition_graph_type)
            IS_A(port,base_item::port_graph_type)
            IS_A(place,base_item::place_graph_type)
            IS_A(top_level_port,base_item::top_level_port_graph_type)

#undef IS_A

            template<typename IT>
            static bool _with ( IT what_pos, const IT& what_end
                              , IT pos, const IT& end
                              )
            {
              while (what_pos != what_end && pos != end && *what_pos == *pos)
                {
                  ++what_pos; ++pos;
                }

              return (what_pos == what_end);
            }

            bool starts_with (const std::string& what, const std::string& s)
            {
              return _with (what.begin(), what.end(), s.begin(), s.end());
            }
            bool ends_with (const std::string& what, const std::string& s)
            {
              return _with (what.rbegin(), what.rend(), s.rbegin(), s.rend());
            }
            bool equals (const std::string& what, const std::string& s)
            {
              return what == s;
            }

            namespace port
            {
              const std::string& name (const base_item * gi)
              {
                return
                  qgraphicsitem_cast<const graph::port_item*> (gi)->name();
              }
              const std::string& type (const base_item * gi)
              {
                return
                  qgraphicsitem_cast<const graph::port_item*> (gi)->we_type();
              }
            }

            namespace transition
            {
              std::string name (const base_item * gi)
              {
                return
                  qgraphicsitem_cast<const graph::transition_item*> (gi)->name();
              }
//               const bool internal (const base_item *);
//               const bool inline (const base_item *);
//               const bool is_expression (const base_item *);
//               const bool is_module_call (const base_item *);
//               const bool is_subnet (const base_item *);
            }

            namespace place
            {
//               const std::string& name (const base_item *);
//               const std::string& type (const base_item *);
//               const bool is_virtual (const base_item *);
            }

            namespace connection
            {
            }
          }
        }
      }
    }
  }
}
