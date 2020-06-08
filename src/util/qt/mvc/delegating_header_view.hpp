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

          virtual void setModel (QAbstractItemModel*) override;

          void delegate_for_section (int, header_delegate*);
          header_delegate* delegate_for_section (int) const;
          void delegate (header_delegate*);
          boost::optional<int> current_editor() const;

        public slots:
          void request_editor (int section);
          void close_editor();

        protected:
          virtual void paintSection (QPainter*, const QRect&, int) const override;
          virtual QSize sizeHint() const override;
          virtual void keyPressEvent (QKeyEvent*) override;
          virtual void contextMenuEvent (QContextMenuEvent*) override;
          virtual void wheelEvent (QWheelEvent*) override;
          virtual bool event (QEvent*) override;

        private slots:
          void sections_inserted (const QModelIndex&, int, int);
          void sections_removed (const QModelIndex&, int, int);
          void data_changed (Qt::Orientation, int, int);

          void set_editor_geometry();

        private:
          QRect editor_geometry() const;

          void invalidate_cached_size_hint();

          QVector<header_delegate*> _delegates;
          header_delegate* _delegate;
          struct _editor_type
          {
            boost::optional<int> section;
            QWidget* widget;
            widget::mini_button* close_button;
            _editor_type() : section (boost::none), widget (nullptr), close_button (nullptr) {}
          } _editor;

          friend class header_delegate;
        };
      }
    }
  }
}
