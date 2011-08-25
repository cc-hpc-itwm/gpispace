#include "TransitionLibraryItem.hpp"

#include "data/Transition.hpp"

#include <QDebug>
#include <QtAlgorithms>

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
        foreach(TransitionLibraryItem* child, _children)
        {
          child->clearChildren();
        }
        _children.clear();
      }

      const QList<TransitionLibraryItem*>& TransitionLibraryItem::children() const
      {
        return _children;
      }

      bool sortAscending(const TransitionLibraryItem* l, const TransitionLibraryItem* r)
      {
        if(l->data() && !r->data())
        {
          return false;
        }
        else if(!l->data() && r->data())
        {
          return true;
        }
        else
        {
          return l->name() < r->name();
        }
      }

      bool sortDescending(const TransitionLibraryItem* l, const TransitionLibraryItem* r)
      {
        if(l->data() && !r->data())
        {
          return true;
        }
        else if(!l->data() && r->data())
        {
          return false;
        }
        else
        {
          return l->name() > r->name();
        }
      }

      void TransitionLibraryItem::sortChildren(bool descending)
      {
        if(descending)
        {
          qSort(_children.begin(), _children.end(), sortDescending);
        }
        else
        {
          qSort(_children.begin(), _children.end(), sortAscending);
        }
        foreach(TransitionLibraryItem* child, _children)
        {
          child->sortChildren();
        }
      }

      const bool& TransitionLibraryItem::trusted() const
      {
        return _trusted;
      }
    }
  }
}
