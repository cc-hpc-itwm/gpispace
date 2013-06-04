// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/TransitionLibraryItem.hpp>

#include <pnete/data/handle/function.hpp>
#include <pnete/data/manager.hpp>

#include <fhg/util/read_bool.hpp>

#include <QtAlgorithms>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        QString name_for_file (const QString& filename)
        {
          const data::handle::function f
            (data::manager::instance().load (filename));
          return QString::fromStdString (f.get().properties()
            .get ("fhg.pnete.library_name")
            .get_value_or (f.get().name()
            .get_value_or ("<<anonymous>>")));
        }
      }

      bool is_disabled (const QString& filename)
      {
        const data::handle::function f
          (data::manager::instance().load (filename));
        return fhg::util::read_bool
          ( f.get().properties().get ("fhg.pnete.library_disabled")
          .get_value_or ("false")
          );
      }

      TransitionLibraryItem::TransitionLibraryItem (QObject* parent)
        : QObject(parent)
        , _is_folder (true)
        , _disabled (true)
        , _fileinfo (QFileInfo())
        , _trusted (false)
        , _name()
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
        , _disabled
          (_is_folder ? false : is_disabled (fileinfo.absoluteFilePath()))
        , _fileinfo (fileinfo)
        , _trusted (trusted)
        , _name ( _is_folder
                ? _fileinfo.baseName()
                : name_for_file (_fileinfo.absoluteFilePath())
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
      bool TransitionLibraryItem::disabled() const
      {
        return _disabled;
      }

      const QFileInfo& TransitionLibraryItem::fileinfo() const
      {
        return _fileinfo;
      }

      QString TransitionLibraryItem::path() const
      {
        return fileinfo().absoluteFilePath();
      }
      QString TransitionLibraryItem::name() const
      {
        return _name;
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
