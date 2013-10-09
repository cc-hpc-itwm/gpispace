// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_DUAL_LIST_SELECTOR_HPP
#define FHG_UTIL_QT_DUAL_LIST_SELECTOR_HPP

#include <util/qt/mvc/fixed_proxy_models.hpp>

#include <QListView>
#include <QWidget>

class QAbstractListModel;
class QAction;

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace detail
      {
        //! \note should be private / inner-class, but moc.
        class dual_list_move_and_drag_proxy : public mvc::id_proxy
        {
          Q_OBJECT

        public:
          dual_list_move_and_drag_proxy
            (QAbstractItemModel*, QObject* = NULL);

          virtual Qt::ItemFlags flags (const QModelIndex&) const;
          virtual Qt::DropActions supportedDropActions() const;
          virtual QStringList mimeTypes() const;
          virtual QMimeData* mimeData (const QModelIndexList&) const;
          virtual bool dropMimeData ( const QMimeData*
                                    , Qt::DropAction
                                    , int row
                                    , int column
                                    , const QModelIndex& parent
                                    );
        };

        class list_view_with_drop_source_check : public QListView
        {
          Q_OBJECT

        public:
          list_view_with_drop_source_check (QWidget* parent = NULL);

          void allowed_drop_sources (QSet<QWidget*>);
          QSet<QWidget*> allowed_drop_sources() const;

        protected:
          void dragMoveEvent (QDragMoveEvent*);

        private:
          QSet<QWidget*> _sources;
        };
      }

      class dual_list_selector : public QWidget
      {
        Q_OBJECT

      public:
        dual_list_selector ( QAbstractListModel* available
                           , QAbstractListModel* selected
                           , QWidget* = NULL
                           );

      private slots:
        void enable_actions();

      private:
        detail::dual_list_move_and_drag_proxy* _available;
        detail::dual_list_move_and_drag_proxy* _selected;
        detail::list_view_with_drop_source_check* _available_view;
        detail::list_view_with_drop_source_check* _selected_view;

        QAction* _select;
        QAction* _deselect;
        QAction* _move_up;
        QAction* _move_down;
      };
    }
  }
}

#endif
