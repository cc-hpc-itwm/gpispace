// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/graph/connectable_item.fwd.hpp>

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/association.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>

#include <QObject>
#include <QSet>
#include <QRectF>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class connectable_item : public base_item
        {
          Q_OBJECT

        public:
          connectable_item (base_item* parent = nullptr);

          void add_association (association* c);
          void remove_association (association * c);

          virtual bool is_connectable_with (const connectable_item*) const;

          const QSet<association*>& associations() const;

          virtual const std::string& we_type() const = 0;

          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event) override;

          virtual QLinkedList<base_item*> childs() const override;

        signals:
          void association_added (association* c);
          void association_removed (association* c);

        protected:
          QSet<association*> _associations;

          const std::string& we_type (const std::string&) const;
        };
      }
    }
  }
}
