// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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
            (QAbstractItemModel*, QObject* = nullptr);

          virtual Qt::ItemFlags flags (const QModelIndex&) const override;
          virtual Qt::DropActions supportedDropActions() const override;
          virtual QStringList mimeTypes() const override;
          virtual QMimeData* mimeData (const QModelIndexList&) const override;
          virtual bool dropMimeData ( const QMimeData*
                                    , Qt::DropAction
                                    , int row
                                    , int column
                                    , const QModelIndex& parent
                                    ) override;
        };

        class list_view_with_drop_source_check : public QListView
        {
          Q_OBJECT

        public:
          list_view_with_drop_source_check (QWidget* parent = nullptr);

          void allowed_drop_sources (QSet<QWidget*>);
          QSet<QWidget*> allowed_drop_sources() const;

        protected:
          virtual void dragMoveEvent (QDragMoveEvent*) override;

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
                           , QWidget* = nullptr
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
