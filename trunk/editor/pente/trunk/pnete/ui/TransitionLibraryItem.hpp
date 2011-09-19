// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_TRANSITION_LIBRARY_ITEM_HPP
#define _PNETE_UI_TRANSITION_LIBRARY_ITEM_HPP 1

#include <QList>
#include <QObject>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class Transition;
    }
    namespace ui
    {
      class TransitionLibraryItem : public QObject
      {
        Q_OBJECT

        public:
          TransitionLibraryItem ( const QString& name
                                , bool is_folder
                                , bool trusted = false
                                , QObject* parent = NULL
                                );

          void appendChild (TransitionLibraryItem* child);

          bool is_folder() const;
          const QString& name() const;
          const bool& trusted() const;

          TransitionLibraryItem* child (int row) const;
          int childCount() const;
          int row() const;
          TransitionLibraryItem* parent() const;

          const QList<TransitionLibraryItem*>& children() const;

          void clearChildren();

          void sortChildren (bool descending = false);

        private:
          bool _is_folder;
          QString _name;
          bool _trusted;

          QList<TransitionLibraryItem*> _children;
          TransitionLibraryItem* _parent;
      };
    }
  }
}

#endif
