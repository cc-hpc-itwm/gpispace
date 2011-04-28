#include "TransitionLibraryItem.hpp"

#include "data/Transition.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      TransitionLibraryItem::TransitionLibraryItem(data::Transition* data, bool trusted, QObject* parent)
      : QObject(parent),
      _data(data),
      _name("SHOULD NEVER DISPLAY"),
      _trusted(trusted),
      _parent(qobject_cast<TransitionLibraryItem*>(parent))
      {
      }
      
      TransitionLibraryItem::TransitionLibraryItem(const QString& name, bool trusted, QObject* parent)
      : QObject(parent),
      _data(NULL),
      _name(name),
      _trusted(trusted),
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
        if(data())
        {
          return data()->name();
        }
        else
        {
          return _name;
        }
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
      
      const bool& TransitionLibraryItem::trusted() const
      {
        return _trusted;
      }
    }
  }
}
