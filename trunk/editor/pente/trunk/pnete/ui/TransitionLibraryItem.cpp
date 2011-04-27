#include "TransitionLibraryItem.hpp"

#include "data/Transition.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      TransitionLibraryItem::TransitionLibraryItem(data::Transition* data, QObject* parent)
      : QObject(parent),
      _data(data),
      _name("SHOULD NEVER DISPLAY"),
      _parent(qobject_cast<TransitionLibraryItem*>(parent))
      {
      }
      
      TransitionLibraryItem::TransitionLibraryItem(const QString& name, QObject* parent)
      : QObject(parent),
      _data(NULL),
      _name(name),
      _parent(qobject_cast<TransitionLibraryItem*>(parent))
      {
      }
      
      void TransitionLibraryItem::appendChild(TransitionLibraryItem* child)
      {
        _children.append(child);
      }
          
      TransitionLibraryItem* TransitionLibraryItem::child(int row) const
      {
        return _children.at(row);
      }
      
      int TransitionLibraryItem::childCount() const
      {
        return _children.count();
      }
      
      data::Transition* TransitionLibraryItem::data() const
      {
        return _data;
      }
      
      const QString& TransitionLibraryItem::name() const
      {
        return _name;
      }
      
      int TransitionLibraryItem::row() const
      {
        if(_parent)
        {
          return _parent->_children.indexOf(const_cast<TransitionLibraryItem*>(this));
        }
        else
        {
          return 0;
        }
      }
      
      TransitionLibraryItem* TransitionLibraryItem::parent() const
      {
        return _parent;
      }
      
      void TransitionLibraryItem::clearChildren()
      {
        return _children.clear();
      }
      
      const QList<TransitionLibraryItem*>& TransitionLibraryItem::children() const
      {
        return _children;
      }
      
      void TransitionLibraryItem::sortChildren(bool descending)
      {
        //! \todo folders first, then alphabetically. (i.e. by name(), then by data()->name().
      }
    }
  }
}
