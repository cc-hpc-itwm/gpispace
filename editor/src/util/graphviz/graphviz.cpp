// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <util/graphviz/graphviz.hpp>

#include <QSizeF>
#include <QGraphicsItem>

#include <iostream>
#include <stdexcept>

#include <gvc.h>

#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace graphviz
    {
      namespace
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
        : _node (agnode (graph, unsafe (name)))
      {
        set_attribute (_node, "label", "");
      }
      Agnode_t* node_type::node () const
      {
        return _node;
      }
      void node_type::size (const QSizeF& size)
      {
        //! \todo correct dpi factor
        const qreal factor (1.0 / 50.0);
        set_attribute ( _node
                      , "width"
                      , QString::number (size.width() * factor)
                      .replace(',', '.')
                      );
        set_attribute ( _node
                      , "height"
                      , QString::number (size.height() * factor)
                      .replace(',', '.')
                      );
      }

      void node_type::fixed_size (const bool& set)
      {
        set_attribute (_node, "fixedsize", set ? "true" : "false");
      }

      void node_type::shape (const QString& shape)
      {
        set_attribute (_node, "shape", shape);
      }

      template<typename T>
      static T parse (const QString& s)
      {
        return boost::lexical_cast<T> (s.toStdString());
      }

      QPointF node_type::position() const
      {
        const QStringList position_strings
          (QString (get_attribute (_node, "pos")).split (","));

        if (position_strings.length() < 2)
          {
            throw std::runtime_error ("node_type::position: wrong position");
          }

        //! \todo check the faster (but unsafe?) alternative:
        //        QPointF (ND_coord (_node).x, ND_coord (_node).y)
        return QPointF ( parse<qreal> (position_strings[0])
                       , parse<qreal> (position_strings[1])
                       );
      }

      edge_type::edge_type ( Agraph_t* graph
                           , Agnode_t* from
                           , Agnode_t* to
                           )
        : _edge (agedge (graph, from, to))
      {}
      QList<QPointF> edge_type::points() const
      {
        QList<QPointF> points;

        const QStringList position_strings
          (QString (get_attribute (_edge, "pos")).split(" "));

        foreach (const QString& point_string, position_strings)
          {
            QStringList components (point_string.split(","));

            if (components[0] == "e")
              {
                components.pop_front();
              }

            if (components.length() < 2)
              {
                throw std::runtime_error ("edge_type::points: wrong position");
              }

            points.push_back ( QPointF ( parse<qreal> (components[0])
                                       , parse<qreal> (components[1])
                                       )
                             );
          }

        return points;
      }

      graph_type::graph_type (context_type& context, const QString& name)
        : _graph (agopen (unsafe (name), AGDIGRAPH))
        , _context (context)
      {}

      graph_type::~graph_type()
      {
        gvFreeLayout (_context._context, _graph);
        agclose (_graph);
      }

      void graph_type::rankdir (const QString& dir)
      {
        set_attribute (_graph, "rankdir", dir);
      }

      void graph_type::splines (const QString& mode)
      {
        set_attribute (_graph, "splines", mode);
      }

      node_type graph_type::add_node (const QString& name)
      {
        return node_type (_graph, name);
      }

      node_type graph_type::add_node (const QGraphicsItem* const item)
      {
        node_type node (_graph, unique_name (item));
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
        gvLayout (_context._context, _graph, unsafe (engine));
        gvRender (_context._context, _graph, unsafe (engine), NULL);
      }
    }
  }
}
