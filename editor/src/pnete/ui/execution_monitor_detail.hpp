// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_EXECUTION_MONITOR_DETAIL_HPP
#define FHG_PNETE_UI_EXECUTION_MONITOR_DETAIL_HPP

#include <pnete/ui/execution_monitor_worker_model.hpp>

#include <util/qt/mvc/fixed_proxy_models.hpp>
#include <util/qt/mvc/section_index.hpp>
#include <util/qt/mvc/header_delegate.hpp>

#include <QStyledItemDelegate>

class QDateTimeEdit;
class QCheckBox;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class execution_monitor_proxy : public util::qt::mvc::id_proxy
      {
        Q_OBJECT

      public:
        enum roles
        {
          //! \note worker_model has first three
          visible_range_role = Qt::UserRole + 100,
          visible_range_to_role,
          automatically_move_role,
          elapsed_time_role,
          column_type_role
        };

        struct visible_range_type
        {
          long _to;
          long _length;
          long to() const
          {
            return _to;
          }
          long from() const
          {
            return _to - _length;
          }
          void to (long t) { _length += (t - _to); _to = t; }
          void from (long f) { _length = _to - f; }
          long length() const
          {
            return _length;
          }
          visible_range_type (long t = 0)
            : _to (t)
            , _length (1)
          { }
          visible_range_type (long f, long t)
            : _to (t)
            , _length (t - f)
          { }
        };

        enum column_type
        {
          name_column,
          gantt_column,
          current_states_column
        };

        execution_monitor_proxy (QAbstractItemModel* model, QObject* parent = NULL);

        virtual QVariant headerData
          (int section, Qt::Orientation, int role = Qt::DisplayRole) const;
        virtual bool setHeaderData
          (int section, Qt::Orientation, const QVariant&, int role = Qt::EditRole);

        void setSourceModel (QAbstractItemModel*);

        virtual int columnCount (const QModelIndex& = QModelIndex()) const;
        virtual QModelIndex mapToSource (const QModelIndex& proxy) const;
        virtual QModelIndex index (int, int, const QModelIndex&) const;
        virtual bool insertColumns
          (int column, int count, const QModelIndex& parent = QModelIndex());
        virtual bool removeColumns
          (int column, int count, const QModelIndex& parent = QModelIndex());

      private slots:
        void move_tick();
        void source_dataChanged (const QModelIndex&, const QModelIndex&);

      private:
        using util::qt::mvc::id_proxy::setSourceModel;

        void move_existing_columns (int begin, int offset);

        QMap<util::qt::mvc::section_index, visible_range_type> _visible_ranges;
        QSet<util::qt::mvc::section_index> _auto_moving;
        QMap<util::qt::mvc::section_index, column_type> _column_types;

        int _column_count;
      };

      class execution_monitor_delegate : public QStyledItemDelegate
                                       , public util::qt::mvc::header_delegate
      {
        Q_OBJECT

      public:
        execution_monitor_delegate ( boost::function<void (QString)> set_filter
                                   , boost::function<QString()> get_filter
                                   , QMap<worker_model::state_type, QColor>
                                   , QWidget* parent = NULL
                                   );

        virtual void paint ( QPainter* painter
                           , const QStyleOptionViewItem& option
                           , const QModelIndex& index
                           ) const;

        virtual void paint
          (QPainter*, const QRect&, const util::qt::mvc::section_index&);
        virtual QWidget* create_editor ( const QRect&
                                       , util::qt::mvc::delegating_header_view*
                                       , const util::qt::mvc::section_index&
                                       );
        virtual void release_editor
          (const util::qt::mvc::section_index&, QWidget* editor);
        virtual void update_editor
          (util::qt::mvc::section_index, QWidget* editor);
        virtual bool can_edit_section (util::qt::mvc::section_index) const;
        virtual QMenu* menu_for_section (util::qt::mvc::section_index) const;

        QColor color_for_state (worker_model::state_type) const;

      signals:
        void color_for_state_changed (worker_model::state_type, QColor);

      public slots:
        void color_for_state (worker_model::state_type, QColor);

        bool helpEvent ( QHelpEvent* event
                       , QAbstractItemView* view
                       , const QStyleOptionViewItem& option
                       , const QModelIndex& index
                       );

      private:
        boost::function<void (QString)> _set_filter;
        boost::function<QString()> _get_filter;

        QMap<worker_model::state_type, QColor> _color_for_state;
      };

      class execution_monitor_editor : public QWidget
      {
        Q_OBJECT

      public:
        execution_monitor_editor ( execution_monitor_delegate*
                                 , util::qt::mvc::section_index
                                 , QWidget* parent = NULL
                                 );

      public slots:
        void update();
        void update_maximum();

      private:
        QScrollBar* _scrollbar;
        QDateTimeEdit* _upper;
        QCheckBox* _automove;

        util::qt::mvc::section_index _index;
      };
    }
  }
}

//! \note visible_range_role
Q_DECLARE_METATYPE (fhg::pnete::ui::execution_monitor_proxy::visible_range_type)
//! \note column_type_role
Q_DECLARE_METATYPE (fhg::pnete::ui::execution_monitor_proxy::column_type)

#endif
