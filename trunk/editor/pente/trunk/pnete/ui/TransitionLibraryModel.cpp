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
        readContentFromDirectory(path.path());
      }
      
      void TransitionLibraryModel::readContentFromDirectory(const QString& path)
      {
        _items->clearChildren();
 
        readContentFromDirectoryRecursive(_items, path);
        _items->sortChildren();
      }
      
      void TransitionLibraryModel::addContentFromDirectory(const QString& path)
      {
        readContentFromDirectoryRecursive(_items, path);
        _items->sortChildren();
      }
      
      void TransitionLibraryModel::readContentFromDirectoryRecursive(TransitionLibraryItem* currentRoot, const QString& path)
      {
        setFileSystemWatcher(path);
        
        QDirIterator directoryWalkerDirs(path, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        while(directoryWalkerDirs.hasNext())
        {
          directoryWalkerDirs.next();
          
          const QString newName = directoryWalkerDirs.fileInfo().fileName();
          
          TransitionLibraryItem* newRoot;
          bool found = false;
          const QList<TransitionLibraryItem*>& children = currentRoot->children();
          for(QList<TransitionLibraryItem*>::const_iterator it = children.begin(); it != children.end(); ++it)
          {
            if(!(*it)->data() && (*it)->name() == newName)
            {
              newRoot = *it;
              found = true;
              break;
            }
          }
          if(!found)
          {
            newRoot = new TransitionLibraryItem(newName, currentRoot);
            currentRoot->appendChild(newRoot);
          }
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
        //! \todo fix this to be working with more than one folder! (i.e. rescan all directories?)
        return;
        
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
        //! \todo symbols for trusted and untrusted entries? (lib / user)
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
        if(hasIndex(row, column, parent))
        {
          TransitionLibraryItem* parentItem = !parent.isValid() ? _items : static_cast<TransitionLibraryItem*>(parent.internalPointer());
    
          TransitionLibraryItem* childItem = parentItem->child(row);
          if(childItem)
          {
            return createIndex(row, column, childItem);
          }
        }
        
        return QModelIndex();
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
