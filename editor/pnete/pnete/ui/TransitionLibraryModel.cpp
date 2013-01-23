// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/TransitionLibraryModel.hpp>

#include <QWidget>
#include <QModelIndex>
#include <QMimeData>
#include <QDir>
#include <QFileSystemWatcher>
#include <QIcon>
#include <QCoreApplication>
#include <QDebug>

#include <pnete/ui/TransitionLibraryItem.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      const QString TransitionLibraryModel::mimeType ("pnete/transition"); // hardcoded constant

      TransitionLibraryModel::TransitionLibraryModel (QWidget* parent)
        : QAbstractItemModel (parent)
        , _fileSystemWatcher (NULL)
        , _items (new TransitionLibraryItem(this))
      {}

      void
      TransitionLibraryModel::addContentFromDirectory ( const QString& path
                                                      , bool trusted
                                                      )
      {
        if(trusted)
          {
            _trustedPaths.push_back(path);
          }
        else
          {
            _untrustedPaths.push_back(path);
          }
        readContentFromDirectoryRecursive(_items, trusted, path);
        _items->sortChildren();
      }

      void
      TransitionLibraryModel::readContentFromDirectoryRecursive
      ( TransitionLibraryItem* currentRoot
      , const bool& trusted
      , const QString& path
      )
      {
        setFileSystemWatcher (path);

        const QDir directory (path);

        foreach ( QFileInfo fileinfo
                , directory.entryInfoList( QDir::Dirs
                                         | QDir::NoSymLinks
                                         | QDir::NoDotAndDotDot
                                         )
                )
        {
          TransitionLibraryItem* newRoot
            (currentRoot->child_with_fileinfo (fileinfo));

          if (!newRoot)
            {
              currentRoot->appendChild
                ( newRoot = new TransitionLibraryItem ( fileinfo
                                                      , true
                                                      , trusted
                                                      , currentRoot
                                                      )
                );
            }

          readContentFromDirectoryRecursive ( newRoot
                                            , trusted
                                            , fileinfo.absoluteFilePath()
                                            );
        }

        foreach ( QFileInfo fileinfo
                , directory.entryInfoList( QStringList("*.xpnet")
                                         , QDir::Files
                                         | QDir::NoSymLinks
                                         | QDir::NoDotAndDotDot
                                         )
                )
          {
            currentRoot->appendChild
              ( new TransitionLibraryItem ( fileinfo
                                          , false
                                          , trusted
                                          , currentRoot
                                          )
              );
          }
      }

      void TransitionLibraryModel::rereadAllDirectories(const QString& path)
      {
        //! \todo only re-read the changed directory instead of deleting everything.
        if (_fileSystemWatcher)
          {
            delete _fileSystemWatcher;
          }
        _fileSystemWatcher = new QFileSystemWatcher(this);
        connect ( _fileSystemWatcher
                , SIGNAL (directoryChanged (QString))
                , SLOT (rereadAllDirectories (QString))
                );

        emit layoutAboutToBeChanged();
        _items->clearChildren();
        emit layoutChanged();

        foreach (QString path, _trustedPaths)
          {
            readContentFromDirectoryRecursive(_items, true, path);
          }
        foreach(QString path, _untrustedPaths)
          {
            readContentFromDirectoryRecursive(_items, false, path);
          }

        emit layoutAboutToBeChanged();
        _items->sortChildren();
        emit layoutChanged();
      }

      void TransitionLibraryModel::setFileSystemWatcher(const QString& path)
      {
        if(!_fileSystemWatcher)
        {
          _fileSystemWatcher = new QFileSystemWatcher(this);

          connect ( _fileSystemWatcher
                  , SIGNAL (directoryChanged (QString))
                  , SLOT (rereadAllDirectories (QString))
                  );
        }
        _fileSystemWatcher->addPath(path);
      }

      int TransitionLibraryModel::rowCount(const QModelIndex& parent) const
      {
        if (parent.column() > 0)
        {
          return 0;
        }

        const TransitionLibraryItem* parentItem
          ( !parent.isValid()
          ? _items
          : static_cast<TransitionLibraryItem*>(parent.internalPointer())
          );

        return parentItem->childCount();
      }

      int TransitionLibraryModel::columnCount(const QModelIndex& parent) const
      {
        return 2;
      }

      QVariant
      TransitionLibraryModel::data (const QModelIndex& index, int role) const
      {
        if (index.isValid())
          {
            TransitionLibraryItem* item
              (static_cast<TransitionLibraryItem*>(index.internalPointer()));

            switch(role)
              {
              case Qt::DisplayRole:
                if(index.column() == 0)
                  {
                    return item->name();
                  }
                break;

              case Qt::DecorationRole:
                //! \todo folder icon?
                if ( index.column() == 1
                   && !item->is_folder()
                   && item->trusted()
                   )
                  {
                    return QIcon(":/lock.png"); // hardcoded constant
                  }
                break;

              case Qt::ToolTipRole:
                if ( index.column() == 1
                   && !item->is_folder()
                   && item->trusted()
                   )
                  {
                    return tr("Trusted Transition");
                  }
                break;

            default:
              ;
          }
        }

        return QVariant();
      }

      QVariant
      TransitionLibraryModel::headerData ( int section
                                         , Qt::Orientation orientation
                                         , int role
                                         ) const
      {
        if ( orientation != Qt::Horizontal
           || role != Qt::DisplayRole
           || section != 0
           )
          {
            return QVariant();
          }
        else
          {
            return QString(tr("Transition"));
          }
      }

      QMimeData*
      TransitionLibraryModel::mimeData (const QModelIndexList& indices) const
      {
        QSet<QString> paths;

        foreach (const QModelIndex& index, indices)
          {
            TransitionLibraryItem* item
              (static_cast<TransitionLibraryItem*> (index.internalPointer()));

            if (!item->is_folder())
              {
                paths << item->path();
              }
          }

        if (!paths.empty())
        {
          QByteArray byteArray;
          QDataStream stream (&byteArray, QIODevice::WriteOnly);

          stream << paths;

          QMimeData* mimeData (new QMimeData);
          mimeData->setData (mimeType, byteArray);

          return mimeData;
        }

        return 0;
      }

      QModelIndex
      TransitionLibraryModel::index ( int row
                                    , int column
                                    , const QModelIndex& parent
                                    ) const
      {
        if (hasIndex (row, column, parent))
          {
            TransitionLibraryItem* parentItem
              ( !parent.isValid()
              ? _items
              : static_cast<TransitionLibraryItem*>(parent.internalPointer())
              );

            TransitionLibraryItem* childItem (parentItem->child(row));

            if (childItem)
              {
                return createIndex(row, column, childItem);
              }
          }

        return QModelIndex();
      }

      QModelIndex
      TransitionLibraryModel::parent (const QModelIndex& index) const
      {
        if (!index.isValid())
          {
            return QModelIndex();
          }

        TransitionLibraryItem* childItem
          (static_cast<TransitionLibraryItem*>(index.internalPointer()));
        TransitionLibraryItem* parentItem (childItem->parent());

        return (parentItem == _items)
          ? QModelIndex()
          : createIndex(parentItem->row(), 0, parentItem)
          ;
      }

      Qt::ItemFlags
      TransitionLibraryModel::flags(const QModelIndex& index) const
      {
        if (!index.isValid())
          {
            return Qt::NoItemFlags;
          }
        else
          {
            return Qt::ItemIsDragEnabled
                 | Qt::ItemIsEnabled
                 | Qt::ItemIsSelectable
                 ;
          }
      }
    }
  }
}
