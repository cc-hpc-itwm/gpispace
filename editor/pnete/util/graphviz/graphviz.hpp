// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_UTIL_GRAPHVIZ_GRAPHVIZ_HPP
#define FHG_UTIL_GRAPHVIZ_GRAPHVIZ_HPP

class QSizeF;
class QGraphicsItem;

#include <QString>
#include <QPointF>
#include <QList>

// Yes, I know the typedef is ugly, but I prefer this to having an include here.
struct Agraph_t;
struct Agedge_t;
struct Agnode_t;
struct GVC_s;
typedef struct GVC_s GVC_t;

namespace fhg
{
  namespace pnete
  {
    namespace graphviz
    {
      class graph_type;
      class node_type;
      class edge_type;

      class context_type
      {
      public:
        context_type();
        ~context_type();

      private:
        GVC_t* _context;

        friend class graph_type;
      };

      class node_type
      {
      public:
        node_type (Agraph_t* graph, const QString& name);

        void size (const QSizeF& size);
        void fixed_size (const bool& set);
        void shape (const QString& shape);

        QPointF position() const;

        Agnode_t* node () const;

      private:
        Agnode_t* _node;
      };

      class edge_type
      {
      public:
        edge_type (Agraph_t*, Agnode_t*, Agnode_t*);

        QList<QPointF> points() const;

      private:
        Agedge_t* _edge;
      };

      class graph_type
      {
      public:
        graph_type (context_type& context, const QString& name = "__G");
        ~graph_type();

        void rankdir (const QString& dir);
        void splines (const QString& mode);

        node_type add_node (const QString& name);
        node_type add_node (const QGraphicsItem* const item);
        edge_type add_edge (const node_type& from, const node_type& to);

        void layout (const QString& engine);

      private:
        Agraph_t* _graph;

        context_type& _context;
      };
    }
  }
}

#endif
