// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_DELEGATING_HEADER_VIEW_HPP
#define FHG_UTIL_QT_DELEGATING_HEADER_VIEW_HPP

#include <util/qt/mini_button.fwd.hpp>
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
          delegating_header_view (QWidget* parent = NULL);

          virtual void setModel (QAbstractItemModel*);

          void delegate_for_section (int, header_delegate*);
          header_delegate* delegate_for_section (int) const;
          void delegate (header_delegate*);

        protected:
          virtual void paintSection (QPainter*, const QRect&, int) const;
          virtual QSize sizeHint() const;
          virtual void keyPressEvent (QKeyEvent*);
          virtual void contextMenuEvent (QContextMenuEvent*);

        private slots:
          void sections_inserted (const QModelIndex&, int, int);
          void sections_removed (const QModelIndex&, int, int);
          void data_changed (Qt::Orientation, int, int);

          void request_editor (int section);
          void set_editor_geometry();
          void close_editor();

        private:
          QRect editor_geometry() const;

          void invalidate_cached_size_hint();

          QVector<header_delegate*> _delegates;
          header_delegate* _delegate;
          struct _editor_type
          {
            boost::optional<int> section;
            QWidget* widget;
            mini_button* close_button;
            _editor_type() : section (boost::none), widget (NULL), close_button (NULL) {}
          } _editor;

          friend class header_delegate;
        };
      }
    }
  }
}

#endif
