// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <util/graphviz.hpp>

#include <QSizeF>
#include <QGraphicsItem>

#include <iostream>

#include <gvc.h>

namespace fhg
{
  namespace pnete
  {
    namespace graphviz
    {
      namespace internal
      {
        class unsafe
        {
        public:
          explicit unsafe (const QString& str)
            : _arr ( strcpy ( new char [strlen (qPrintable (str)) + 1]
                            , qPrintable (str)
                            )
                   )
          {}
          ~unsafe () { delete[] _arr; }

          operator char * () { return _arr; }

        private:
          char * _arr;
        };

        int set_attribute ( void* a
                          , const QString& key
                          , const QString& v_1
                          , const QString& v_2 = ""
                          )
        {
          return agsafeset (a, unsafe (key), unsafe (v_1), unsafe (v_2));
        }

        char* get_attribute (void* a, const QString& key)
        {
          return agget (a, unsafe (key));
        }

        QString unique_name (const void* const a)
        {
          return QString::number (reinterpret_cast<size_t> (a));
        }
      }

      context_type::context_type()
        : _context (gvContext())
      {}

      context_type::~context_type()
      {
        gvFreeContext (_context);
      }



      node_type::node_type (Agraph_t* graph, const QString& name)
        : _node (agnode (graph, internal::unsafe (name)))
      {
        internal::set_attribute (_node, "label", "");
      }
      Agnode_t* node_type::node () const
      {
        return _node;
      }
      void node_type::size (const QSizeF& size)
      {
        //! \todo correct dpi factor
        const qreal factor (1.0 / 50.0);
        internal::set_attribute ( _node
                                , "width"
                                , QString::number (size.width() * factor)
                                .replace(',', '.')
                                );
        internal::set_attribute ( _node
                                , "height"
                                , QString::number (size.height() * factor)
                                .replace(',', '.')
                                );
      }

      void node_type::fixed_size (const bool& set)
      {
        internal::set_attribute ( _node
                                , "fixedsize"
                                , set ? "true" : "false"
                                );
      }

      void node_type::shape (const QString& shape)
      {
        internal::set_attribute (_node, "shape", shape);
      }

      QPointF node_type::position() const
      {
        QStringList position_strings
          (QString (internal::get_attribute (_node, "pos")).split (","));

        //! \todo check the faster (but unsafe?) alternative:
        //        QPointF (ND_coord (_node).x, ND_coord (_node).y)
        return QPointF ( position_strings[0].toInt()
                       , position_strings[1].toInt()
                       );
      }

      edge_type::edge_type ( Agraph_t* graph
                           , Agnode_t* from
                           , Agnode_t* to
                           )
        : _edge (agedge (graph, from, to))
      {}
      void edge_type::beep () const
      {
        std::cout << _edge << ": ";

        const bezier* b (ED_spl(_edge)->list);

        for (int i (0); i < b->size; ++i)
          {
            const pointf& p (b->list[i]);

            std::cout << p.x << ":" << p.y << ", ";
          }
        std::cout << std::endl;

        std::cout << "## " << internal::get_attribute (_edge, "pos") << std::endl;

      }

      graph_type::graph_type (context_type& context, const QString& name)
        : _graph (agopen (internal::unsafe (name), AGDIGRAPH))
        , _context (context)
      {}

      graph_type::~graph_type()
      {
        gvFreeLayout (_context._context, _graph);
        agclose (_graph);
      }

      void graph_type::rankdir (const QString& dir)
      {
        internal::set_attribute (_graph, "rankdir", dir);
      }

      void graph_type::splines (const QString& mode)
      {
        internal::set_attribute (_graph, "splines", mode);
      }

      node_type graph_type::add_node (const QString& name)
      {
        return node_type (_graph, name);
      }

      node_type graph_type::add_node (const QGraphicsItem* const item)
      {
        node_type node (_graph, internal::unique_name (item));
        node.size (item->boundingRect().size());
        node.fixed_size (true);
        node.shape ("rectangle");
        return node;
      }

      edge_type graph_type::add_edge ( const node_type& from
                                     , const node_type& to
                                     )
      {
        return edge_type (_graph, from.node(), to.node());
      }

      void graph_type::layout (const QString& engine)
      {
        gvLayout (_context._context, _graph, internal::unsafe (engine));
        gvRender (_context._context, _graph, internal::unsafe (engine), NULL);
      }
    }
  }
}
