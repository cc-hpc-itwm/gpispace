// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connectable_item.hpp>

#include <pnete/ui/graph/association.hpp>
#include <pnete/ui/graph/scene.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QDebug>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connectable_item::connectable_item (base_item* parent)
          : base_item (parent)
          , _associations ()
        {}

        void connectable_item::add_association (association* c)
        {
          if (_associations.contains (c))
          {
            throw std::runtime_error ("tried adding the same association twice.");
          }
          _associations.insert (c);
          emit association_added (c);
        }
        void connectable_item::remove_association (association* c)
        {
          if (!_associations.contains (c))
          {
            throw std::runtime_error ("item did not have that association.");
          }
          emit association_removed (c);
          _associations.remove (c);
        }

        bool connectable_item::is_connectable_with (const connectable_item* i) const
        {
          //! \note Default to connecting to anything with the same type.
          return i->we_type() == we_type();
        }
        bool connectable_item::may_be_connected() const
        {
          return true;
        }

        const QSet<association*>& connectable_item::associations() const
        {
          return _associations;
        }

        const std::string& connectable_item::we_type (const std::string& unmapped) const
        {
          //! \todo Reimplement with getting type_map at call-time from handle.
          // - get type_map
          // - it = type_map.find (unmapped)
          // - it ? return it->mapped_type : unmapped
          return unmapped;
        }

        QLinkedList<base_item*> connectable_item::childs() const
        {
          QLinkedList<base_item*> childs;

          foreach (association* association, associations())
          {
            childs << association;
          }

          return childs;
        }

        //! \todo This should be in the different connectable items instead.
        void connectable_item::mousePressEvent
          (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::NoModifier)
          {
            //! \todo Instead of prohibiting connections, remove
            //! existing ones and create a new one.
            if (may_be_connected())
            {
              scene()->create_pending_connection (this);
            }
          }
          else
          {
            base_item::mousePressEvent (event);
          }
        }
      }
    }
  }
}
