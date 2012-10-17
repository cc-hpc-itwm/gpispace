// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/TransitionLibraryItem.hpp>

#include <pnete/data/manager.hpp>

#include <QtAlgorithms>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      TransitionLibraryItem::TransitionLibraryItem (QObject* parent)
        : QObject(parent)
        , _is_folder (true)
        , _fileinfo (QFileInfo())
        , _trusted (false)
        , _data (0)
        , _children ()
        , _parent (qobject_cast<TransitionLibraryItem*>(parent))
      {}

      TransitionLibraryItem::TransitionLibraryItem ( const QFileInfo& fileinfo
                                                   , bool is_folder
                                                   , bool trusted
                                                   , QObject* parent
                                                   )
        : QObject(parent)
        , _is_folder (is_folder)
        , _fileinfo (fileinfo)
        , _trusted (trusted)
        , _data (_is_folder
                ? 0
                : data::manager::instance().load (_fileinfo.absoluteFilePath())
                )
        , _children ()
        , _parent (qobject_cast<TransitionLibraryItem*>(parent))
      {}

      void TransitionLibraryItem::appendChild(TransitionLibraryItem* child)
      {
        _children.append(child);
      }

      TransitionLibraryItem* TransitionLibraryItem::child(int row) const
      {
        return _children.at(row);
      }

      TransitionLibraryItem*
      TransitionLibraryItem::child_with_fileinfo
      (const QFileInfo& fileinfo) const
      {
        foreach (TransitionLibraryItem* child, children())
          {
            if (child->fileinfo() == fileinfo)
              {
                return child;
              }
          }

        return 0;
      }

      int TransitionLibraryItem::childCount() const
      {
        return _children.count();
      }

      bool TransitionLibraryItem::is_folder() const
      {
        return _is_folder;
      }

      const QFileInfo& TransitionLibraryItem::fileinfo() const
      {
        return _fileinfo;
      }

      const data::internal_type* TransitionLibraryItem::data() const
      {
        return _data;
      }

      QString TransitionLibraryItem::path() const
      {
        return fileinfo().absoluteFilePath();
      }
      QString TransitionLibraryItem::name() const
      {
        //! \todo better handling for the anonymous case
        return is_folder()
          ? fileinfo().baseName()
          : QString::fromStdString ( data()->function().name()
                                   ? *data()->function().name()
                                   : "<<anonymous>>"
                                   )
          ;
      }

      const bool& TransitionLibraryItem::trusted() const
      {
        return _trusted;
      }

      int TransitionLibraryItem::row() const
      {
        if(_parent)
        {
          return _parent->_children.indexOf
            (const_cast<TransitionLibraryItem*>(this));
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
        if(!l->is_folder() && r->is_folder())
        {
          return false;
        }
        else if(l->is_folder() && !r->is_folder())
        {
          return true;
        }
        else
        {
          return l->path() < r->path();
        }
      }

      bool sortDescending(const TransitionLibraryItem* l, const TransitionLibraryItem* r)
      {
        if(!l->is_folder() && r->is_folder())
        {
          return true;
        }
        else if(l->is_folder() && !r->is_folder())
        {
          return false;
        }
        else
        {
          return l->path() > r->path();
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
    }
  }
}
