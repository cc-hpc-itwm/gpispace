// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/data/manager.fwd.hpp>

#include <QList>
#include <QObject>
#include <QFileInfo>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class TransitionLibraryItem : public QObject
      {
        Q_OBJECT

      public:
        TransitionLibraryItem (QObject*);
        TransitionLibraryItem ( const QFileInfo& fileinfo
                              , data::manager&
                              , bool is_folder
                              , bool trusted = false
                              , QObject* parent = nullptr
                              );

        void appendChild (TransitionLibraryItem* child);

        bool is_folder() const;
        const QFileInfo& fileinfo() const;
        QString path() const;
        QString name() const;
        const bool& trusted() const;
        TransitionLibraryItem* child (int row) const;
        TransitionLibraryItem* child_with_fileinfo (const QFileInfo&) const;
        int childCount() const;
        int row() const;
        TransitionLibraryItem* parent() const;

        const QList<TransitionLibraryItem*>& children() const;

        void clearChildren();

        void sortChildren (bool descending = false);

      private:
        bool _is_folder;
        QFileInfo _fileinfo;
        bool _trusted;
        QString _name;

        QList<TransitionLibraryItem*> _children;
        TransitionLibraryItem* _parent;
      };
    }
  }
}
