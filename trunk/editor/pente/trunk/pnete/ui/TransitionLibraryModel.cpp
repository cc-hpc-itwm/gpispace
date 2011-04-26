#include "TransitionLibraryModel.hpp"
#include "TransitionLibraryItem.hpp"

#include "data/Transition.hpp"

#include <QWidget>
#include <QModelIndex>
#include <QMimeData>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDirIterator>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      const QString TransitionLibraryModel::mimeType = "pnete/transition";
      
      TransitionLibraryModel::TransitionLibraryModel(const QDir& path, QWidget* parent)
      : QAbstractItemModel(parent),
      _fileSystemWatcher(NULL),
      _items(new TransitionLibraryItem("__DUMMY__ROOT__", this))
      {
        setFileSystemWatcher(path.path());
        readContentFromDirectory(path.path());
      }
      
      void TransitionLibraryModel::readContentFromDirectory(const QString& path)
      {
        _items->clearChildren();
 
        readContentFromDirectoryRecursive(_items, path);
      }
      
      void TransitionLibraryModel::readContentFromDirectoryRecursive(TransitionLibraryItem* currentRoot, const QString& path)
      {
        QDirIterator directoryWalkerDirs(path, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        while(directoryWalkerDirs.hasNext())
        {
          directoryWalkerDirs.next();
          
          TransitionLibraryItem* newRoot = new TransitionLibraryItem(directoryWalkerDirs.fileInfo().fileName(), currentRoot);
          currentRoot->appendChild(newRoot);
          readContentFromDirectoryRecursive(newRoot, directoryWalkerDirs.fileInfo().absoluteFilePath());
        }
        
        QDirIterator directoryWalkerFiles(path, QStringList("*.xml"), QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        while(directoryWalkerFiles.hasNext())
        {
          directoryWalkerFiles.next();
          
          data::Transition* transition = new data::Transition(directoryWalkerFiles.fileInfo().absoluteFilePath());
          currentRoot->appendChild(new TransitionLibraryItem(transition, currentRoot));
        }
      }
      
      void TransitionLibraryModel::setFileSystemWatcher(const QString& path)
      {
        delete _fileSystemWatcher;
        
        _fileSystemWatcher = new QFileSystemWatcher(QStringList(path), this);
        connect(_fileSystemWatcher, SIGNAL(directoryChanged(const QString&)), SLOT(readContentFromDirectory(const QString&)));
      }
      
      int TransitionLibraryModel::rowCount(const QModelIndex& parent) const
      {
        if(parent.column() > 0)
        {
          return 0;
        }
    
        TransitionLibraryItem* parentItem = !parent.isValid() ? _items : static_cast<TransitionLibraryItem*>(parent.internalPointer());
        return parentItem->childCount();
      }
      
      int TransitionLibraryModel::columnCount(const QModelIndex& parent) const
      {
        return 1;
      }
      
      QVariant TransitionLibraryModel::data(const QModelIndex& index, int role) const
      {
        if(!index.isValid() || role != Qt::DisplayRole)
        {
          return QVariant();
        }
        else
        {
          TransitionLibraryItem* item = static_cast<TransitionLibraryItem*>(index.internalPointer());
          if(item->data())
          {
            return item->data()->name();
          }
          else
          {
            return item->name();
          }
        }
      }
      
      QVariant TransitionLibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
        if(orientation != Qt::Horizontal || role != Qt::DisplayRole)
        {
          return QVariant();
        }
        else
        {
          return QString(tr("Transition"));
        }
      }
      
      QMimeData* TransitionLibraryModel::mimeData(const QModelIndexList& indexes) const
      {
        if(indexes.size() != 1)
        {
          return NULL;
        }
        
        TransitionLibraryItem* item = static_cast<TransitionLibraryItem*>(indexes.first().internalPointer());
        if(item->data())
        {
          QMimeData* mimeData = new QMimeData;
          QByteArray byteArray;
          // Thanks Qt for providing a "toByteArray()" method.
          QDataStream stream(&byteArray, QIODevice::WriteOnly);
          stream << *item->data();
          mimeData->setData(mimeType, byteArray);
          
          return mimeData;
        }
        else
        {
          return NULL;
        }
      }
      
      QModelIndex TransitionLibraryModel::index(int row, int column, const QModelIndex& parent) const
      {
        if(!hasIndex(row, column, parent))
        {
          return QModelIndex();
        }

        TransitionLibraryItem* parentItem = !parent.isValid() ? _items : static_cast<TransitionLibraryItem*>(parent.internalPointer());
  
        TransitionLibraryItem* childItem = parentItem->child(row);
        if(childItem)
        {
          return createIndex(row, column, childItem);
        }
        else
        {
          return QModelIndex();
        }
      }
      
      QModelIndex TransitionLibraryModel::parent(const QModelIndex& index) const
      {
        if (!index.isValid())
        {
          return QModelIndex();
        }
    
        TransitionLibraryItem* childItem = static_cast<TransitionLibraryItem*>(index.internalPointer());
        TransitionLibraryItem* parentItem = childItem->parent();
    
        if(parentItem == _items)
        {
          return QModelIndex();
        }
        else
        {
          return createIndex(parentItem->row(), 0, parentItem);
        }
      }
      
      Qt::ItemFlags TransitionLibraryModel::flags(const QModelIndex& index) const
      {
        if (!index.isValid())
        {
          return Qt::NoItemFlags;
        }
        else
        {
          return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
      }
    }
  }
}
