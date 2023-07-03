// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-qt/widget/mini_button.fwd.hpp>
#include <util/qt/mvc/header_delegate.fwd.hpp>

#include <boost/optional.hpp>

#include <QHeaderView>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class delegating_header_view : public QHeaderView
        {
          Q_OBJECT

        public:
          delegating_header_view (QWidget* parent = nullptr);

          void setModel (QAbstractItemModel*) override;

          void delegate_for_section (int, header_delegate*);
          header_delegate* delegate_for_section (int) const;
          void delegate (header_delegate*);
          ::boost::optional<int> current_editor() const;

        public slots:
          void request_editor (int section);
          void close_editor();

        protected:
          void paintSection (QPainter*, QRect const&, int) const override;
          QSize sizeHint() const override;
          void keyPressEvent (QKeyEvent*) override;
          void contextMenuEvent (QContextMenuEvent*) override;
          void wheelEvent (QWheelEvent*) override;
          bool event (QEvent*) override;

        private slots:
          void sections_inserted (QModelIndex const&, int, int);
          void sections_removed (QModelIndex const&, int, int);
          void data_changed (Qt::Orientation, int, int);

          void set_editor_geometry();

        private:
          QRect editor_geometry() const;

          void invalidate_cached_size_hint();

          QVector<header_delegate*> _delegates;
          header_delegate* _delegate {nullptr};
          struct _editor_type
          {
            ::boost::optional<int> section;
            QWidget* widget {nullptr};
            widget::mini_button* close_button {nullptr};
          } _editor;

          friend class header_delegate;
        };
      }
    }
  }
}
