// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/dual_list_selector.hpp>

#include <util/qt/boost_connect.hpp>

#include <fhg/assert.hpp>

#include <QAbstractListModel>
#include <QAction>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QListView>
#include <QMimeData>
#include <QToolButton>
#include <QVBoxLayout>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

Q_DECLARE_METATYPE (QModelIndex)

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace detail
      {
        dual_list_move_and_drag_proxy::dual_list_move_and_drag_proxy
          (QAbstractItemModel* model, QObject* parent)
            : mvc::id_proxy (parent)
        {
          setSourceModel (model);
        }

        Qt::ItemFlags dual_list_move_and_drag_proxy::flags
          (const QModelIndex& index) const
        {
          return mvc::id_proxy::flags (index)
            | (index.isValid() ? Qt::ItemIsDragEnabled : Qt::NoItemFlags)
            | Qt::ItemIsDropEnabled;
        }
        Qt::DropActions dual_list_move_and_drag_proxy::supportedDropActions() const
        {
          return Qt::MoveAction;
        }

        namespace
        {
          QString mime_type()
          {
            static const QString mime_type_prefix
              ("application/fhg/dual_list_move_and_drag_proxy/item");
            return mime_type_prefix;
          }
        }

        QStringList dual_list_move_and_drag_proxy::mimeTypes() const
        {
          return QStringList() << mime_type();
        }

        QMimeData* dual_list_move_and_drag_proxy::mimeData
          (const QModelIndexList& indices) const
        {
          QByteArray encoded;
          QDataStream stream (&encoded, QIODevice::WriteOnly);

          stream << indices.size();

          BOOST_FOREACH (QModelIndex index, indices)
          {
            fhg_assert (index.isValid(), "only valid indices shall have ItemIsDragEnabled");
            stream << itemData (index);
          }

          QMimeData* data (new QMimeData);
          data->setData (mime_type(), encoded);
          return data;
        }

        bool dual_list_move_and_drag_proxy::dropMimeData
          ( const QMimeData* data
          , Qt::DropAction action
          , int row
          , int column
          , const QModelIndex& parent
          )
        {
          fhg_assert (data->hasFormat (mime_type()), "only able to drag&drop from same model");
          fhg_assert (action == Qt::MoveAction, "move is the only drop action");
          fhg_assert (column == -1 && row == -1, "should always drop on nothing");

          QByteArray encoded (data->data (mime_type()));
          QDataStream stream (&encoded, QIODevice::ReadOnly);

          int count (-1);
          stream >> count;

          int begin_row (parent.isValid() ? parent.row() : rowCount (parent));
          insertRows (begin_row, count, QModelIndex());

          for (; count > 0 && !stream.atEnd(); --count, ++begin_row)
          {
            QMap<int, QVariant> item_data;
            stream >> item_data;
            setItemData (index (begin_row, 0, QModelIndex()), item_data);
          }

          fhg_assert (count == 0 && stream.atEnd(), "given count shall match data");

          return true;
        }

        list_view_with_drop_source_check::list_view_with_drop_source_check
          (QWidget* parent)
            : QListView (parent)
        { }

        void list_view_with_drop_source_check::allowed_drop_sources
          (QSet<QWidget*> sources_)
        {
          _sources = sources_;
        }
        QSet<QWidget*>
          list_view_with_drop_source_check::allowed_drop_sources() const
        {
          return _sources;
        }

        void list_view_with_drop_source_check::dragMoveEvent (QDragMoveEvent* e)
        {
          if (!_sources.contains (e->source()))
          {
            e->ignore();
            return;
          }
          QListView::dragMoveEvent (e);
        }
      }

      namespace
      {
        void move_selected_entries_list ( QAbstractItemView* from_view
                                        , QAbstractItemView* to
                                        , const QModelIndexList selection
                                        )
        {
          {
            int row (to->model()->rowCount());
            to->model()->insertRows (row, selection.size());
            to->selectionModel()->clear();
            BOOST_FOREACH (const QModelIndex idx, selection)
            {
              const QModelIndex index (to->model()->index (row, 0));
              to->model()->setItemData (index, idx.model()->itemData (idx));
              to->selectionModel()->setCurrentIndex
                (index, QItemSelectionModel::Select);
              ++row;
            }
            to->setFocus();
          }

          {
            QAbstractItemModel* from (from_view->model());
            for (int row (selection.size() - 1); row >= 0; --row)
            {
              from->removeRow (selection.at (row).row());
            }
          }
        }

        void move_selected_entries ( QAbstractItemView* from_view
                                   , QAbstractItemView* to
                                   )
        {
          QModelIndexList selection
            (from_view->selectionModel()->selectedIndexes());
          // "The list contains no duplicates, and is not sorted.", while we
          // would prefer to have things sorted to not fuck up deleting
          qSort (selection);
          move_selected_entries_list (from_view, to, selection);
        }

        void move_selected_entries ( QAbstractItemView* from_view
                                   , QAbstractItemView* to
                                   , const QModelIndex index
                                   )
        {
          move_selected_entries_list (from_view, to, QModelIndexList() << index);
        }

        void swap (QModelIndex lhs, QModelIndex rhs, QAbstractItemModel* model)
        {
          fhg_assert (model == lhs.model(), "swap: lhs and rhs shall have the same models");
          fhg_assert (model == rhs.model(), "swap: lhs and rhs shall have the same models");
          const QMap<int, QVariant> data_lhs (model->itemData (lhs));
          model->setItemData (lhs, model->itemData (rhs));
          model->setItemData (rhs, data_lhs);
        }

        template<int offset>
          void move_selected (QAbstractItemView* view, QAbstractItemModel* model)
        {
          const QModelIndex to_move (view->selectionModel()->currentIndex());
          const QModelIndex sibling
            (to_move.sibling (to_move.row() + offset, to_move.column()));

          fhg_assert (sibling.isValid(), "move_selected: don't move above 0 or below max");

          swap (to_move, sibling, model);
          view->setCurrentIndex (sibling);
        }

        void connect_any_change
          (QAbstractItemModel* model, QObject* dest, const char* method)
        {
          QObject::connect
            (model, SIGNAL (dataChanged (QModelIndex, QModelIndex)), dest, method);
          QObject::connect
            (model, SIGNAL (layoutChanged()), dest, method);
          QObject::connect
            (model, SIGNAL (modelReset()), dest, method);
          QObject::connect
            (model, SIGNAL (rowsInserted (QModelIndex, int, int)), dest, method);
          QObject::connect
            (model, SIGNAL (rowsRemoved (QModelIndex, int, int)), dest, method);
          QObject::connect
            ( model, SIGNAL (rowsMoved (QModelIndex, int, int, QModelIndex, int))
            , dest, method
            );
        }

        QToolButton* button_for_action (QAction* action, QWidget* parent)
        {
          QToolButton* button (new QToolButton (parent));
          button->setDefaultAction (action);
          return button;
        }
      }

      dual_list_selector::dual_list_selector ( QAbstractListModel* available
                                             , QAbstractListModel* selected
                                             , QWidget* parent
                                             )
        : QWidget (parent)
        , _available (new detail::dual_list_move_and_drag_proxy (available, this))
        , _selected (new detail::dual_list_move_and_drag_proxy (selected, this))
        , _available_view (new detail::list_view_with_drop_source_check (this))
        , _selected_view (new detail::list_view_with_drop_source_check (this))
        , _select (new QAction (tr ("select"), this))
        , _deselect (new QAction (tr ("deselect"), this))
        , _move_up (new QAction (tr ("up"), this))
        , _move_down (new QAction (tr ("down"), this))
      {
        {
          QHBoxLayout* layout (new QHBoxLayout (this));
          layout->addWidget (_available_view);
          {
            QVBoxLayout* swapper (new QVBoxLayout);
            layout->addLayout (swapper);
            swapper->addWidget (button_for_action (_deselect, this));
            swapper->addWidget (button_for_action (_select, this));
          }
          layout->addWidget (_selected_view);
          {
            QVBoxLayout* mover (new QVBoxLayout);
            layout->addLayout (mover);
            mover->addWidget (button_for_action (_move_up, this));
            mover->addWidget (button_for_action (_move_down, this));
          }
        }

        _available_view->setModel (_available);
        _selected_view->setModel (_selected);
        _available_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
        _selected_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
        _available_view->setDragEnabled (true);
        _selected_view->setDragEnabled (true);
        _available_view->setAcceptDrops (true);
        _selected_view->setAcceptDrops (true);
        _available_view->setDropIndicatorShown (true);
        _selected_view->setDropIndicatorShown (true);

        _available_view->allowed_drop_sources
          (QSet<QWidget*>() << _selected_view);
        _selected_view->allowed_drop_sources
          (QSet<QWidget*>() << _available_view << _selected_view);

        fhg::util::qt::boost_connect<void()>
          ( _deselect, SIGNAL (triggered())
          , boost::bind (move_selected_entries, _selected_view, _available_view)
          );
        fhg::util::qt::boost_connect<void()>
          ( _select, SIGNAL (triggered())
          , boost::bind (move_selected_entries, _available_view, _selected_view)
          );
        fhg::util::qt::boost_connect<void()>
          ( _move_up, SIGNAL (triggered())
          , boost::bind (move_selected<-1>, _selected_view, _selected)
          );
        fhg::util::qt::boost_connect<void()>
          ( _move_down, SIGNAL (triggered())
          ,  boost::bind (move_selected<1>, _selected_view, _selected)
          );

        boost_connect<void (QModelIndex)>
          ( _selected_view, SIGNAL (doubleClicked (QModelIndex))
          , boost::bind (move_selected_entries, _selected_view, _available_view, _1)
          );
        boost_connect<void (QModelIndex)>
          ( _available_view, SIGNAL (doubleClicked (QModelIndex))
          , boost::bind (move_selected_entries, _available_view, _selected_view, _1)
          );

        connect_any_change (_selected, this, SLOT (enable_actions()));
        connect_any_change (_available, this, SLOT (enable_actions()));
        connect ( _selected_view->selectionModel()
                , SIGNAL (selectionChanged (QItemSelection, QItemSelection))
                , SLOT (enable_actions())
                );
        connect ( _available_view->selectionModel()
                , SIGNAL (selectionChanged (QItemSelection, QItemSelection))
                , SLOT (enable_actions())
                );
        enable_actions();
      }

      namespace
      {
        boost::optional<int> selected_row (QAbstractItemView* view)
        {
          const QModelIndexList indices
            (view->selectionModel()->selectedIndexes());
          return indices.isEmpty()
            ? boost::optional<int> (boost::none) : indices.first().row();
        }
      }

      void dual_list_selector::enable_actions()
      {
        const boost::optional<int> left_row (selected_row (_available_view));
        const boost::optional<int> right_row (selected_row (_selected_view));

        _select->setEnabled (left_row);
        _deselect->setEnabled (right_row);
        _move_up->setEnabled (right_row && *right_row > 0);
        _move_down->setEnabled
          (right_row && *right_row < _selected->rowCount() - 1);
      }
    }
  }
}
