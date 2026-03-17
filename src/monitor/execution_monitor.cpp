// Copyright (C) 2013-2019,2021-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/monitor/execution_monitor.hpp>

#include <gspc/monitor/execution_monitor_detail.hpp>
#include <gspc/monitor/execution_monitor_worker_model.hpp>

#include <gspc/util/qt/variant.hpp>
#include <gspc/util/qt/widget/mini_button.hpp>
#include <gspc/util/qt/dual_list_selector.hpp>
#include <gspc/util/qt/mvc/alphanum_sort_proxy.hpp>
#include <gspc/util/qt/mvc/delegating_header_view.hpp>
#include <gspc/util/qt/mvc/filter_ignoring_branch_nodes_proxy.hpp>
#include <gspc/util/qt/mvc/flat_to_tree_proxy.hpp>
#include <gspc/util/qt/restricted_tree_column_sorter.hpp>
#include <gspc/util/qt/treeview_with_delete.hpp>

#include <optional>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <QAction>
#include <QScrollArea>
#include <QSettings>
#include <QTreeView>
#include <QVBoxLayout>

#include <functional>
#include <sstream>
#include <stdexcept>
#include <utility>

Q_DECLARE_METATYPE (gspc::scheduler::daemon::NotificationEvent::state_t)



    namespace gspc::monitor
    {
      namespace
      {
        using gspc::util::qt::mvc::transform_functions_model;

        struct hostname_of_worker
          : public transform_functions_model::transform_function
        {
          QString operator() (QModelIndex index) const override
          {
            auto const worker_name
              (gspc::util::qt::value<QString> (index.data (worker_model::name_role)));

            auto const start (worker_name.indexOf ('-') + 1);
            return worker_name.mid
              (start, worker_name.lastIndexOf ('-') - start);
          }

          template<class Archive>
            void serialize (Archive&, unsigned int)
          {
            ::boost::serialization::void_cast_register
              ( static_cast<hostname_of_worker*> (nullptr)
              , static_cast<transform_functions_model::transform_function*> (nullptr)
              );
          }
        };
        struct worker_type_of_worker
          : public transform_functions_model::transform_function
        {
          QString operator() (QModelIndex index) const override
          {
            auto const worker_name
              (gspc::util::qt::value<QString> (index.data (worker_model::name_role)));

            return worker_name.left (worker_name.indexOf ('-'));
          }

          template<class Archive>
            void serialize (Archive&, unsigned int)
          {
            ::boost::serialization::void_cast_register
              ( static_cast<worker_type_of_worker*> (nullptr)
              , static_cast<transform_functions_model::transform_function*> (nullptr)
              );
          }
        };
      }
    }



BOOST_CLASS_EXPORT (gspc::monitor::hostname_of_worker)
BOOST_CLASS_EXPORT (gspc::monitor::worker_type_of_worker)

template<typename T>
  QDataStream& operator<< (QDataStream& stream, std::shared_ptr<T> const& ptr)
{
  std::ostringstream os;
  ::boost::archive::text_oarchive oa (os);
  oa << ptr;
  stream << QString::fromStdString (os.str());
  return stream;
}
template<typename T>
  QDataStream& operator>> (QDataStream& stream, std::shared_ptr<T>& ptr)
{
  QString serialized;
  stream >> serialized;
  std::istringstream is (serialized.toStdString());
  ::boost::archive::text_iarchive ia (is);
  ia >> ptr;
  return stream;
}



    namespace gspc::monitor
    {
      namespace
      {
        using gspc::util::qt::mvc::transform_functions_model;

        //! \todo Other defaults? User defined functions?
        QMap<int, QVariant> item
          (QString name, transform_functions_model::transform_function* f)
        {
          QMap<int, QVariant> roles;
          roles[Qt::DisplayRole] = name;
          roles[transform_functions_model::function_role]
            = QVariant::fromValue
            (std::shared_ptr<transform_functions_model::transform_function> (f));
          return roles;
        }

        void add_defaults (transform_functions_model* model)
        {
          model->insertRows (0, 2);
          model->setItemData
            ( model->index (0, 0)
            , item ("by worker type", new worker_type_of_worker())
            );
          model->setItemData
            ( model->index (1, 0)
            , item ("by node", new hostname_of_worker())
            );
        }

        void set_filter_regexp
          (gspc::util::qt::mvc::filter_ignoring_branch_nodes_proxy* model, QString regexp)
        {
          model->setFilterRegExp (regexp);
        }
        QString get_filter_regexp
          (gspc::util::qt::mvc::filter_ignoring_branch_nodes_proxy* model)
        {
          return model->filterRegExp().pattern();
        }

        void add_columns (int count, QAbstractItemModel* model)
        {
          const int current_count (model->columnCount());
          if (count > 0)
          {
            model->insertColumns (current_count, count);

            for (int i (0); i < count; ++i)
            {
              model->setHeaderData ( current_count + i
                                   , Qt::Horizontal
                                   , QVariant::fromValue (execution_monitor_proxy::gantt_column)
                                   , execution_monitor_proxy::column_type_role
                                   );
            }
          }
          else
          {
            model->removeColumns (current_count + count, -count);
          }
        }

        void disable_if_column_adding_not_possible
          (QAction* action, int count, QAbstractItemModel* model)
        {
          action->setEnabled (model->columnCount() + count >= 1);
        }

        QString to_string (worker_model::state_type state)
        {
          return QString::fromStdString
            (gspc::scheduler::daemon::NotificationEvent::to_string (state));
        }

        QColor get_or_set_with_default
          (QSettings& settings, QString key, QColor def)
        {
          const QVariant value (settings.value (key));
          if (!value.isValid())
          {
            settings.setValue (key, def);
            return def;
          }
          return gspc::util::qt::value<QColor> (value);
        }

        QMap<worker_model::state_type, QColor> get_or_set_with_defaults()
        {
          QSettings settings;
          settings.beginGroup ("gantt");

          QMap<worker_model::state_type, QColor> color_for_state;

          using event = gspc::scheduler::daemon::NotificationEvent;

#define INIT(enummed, dflt)                                             \
          color_for_state[event:: STATE_ ## enummed]                    \
            = get_or_set_with_default                                   \
              (settings, to_string (event:: STATE_ ## enummed), dflt)

          INIT (STARTED, QColor (255, 255, 0));
          INIT (FINISHED, QColor (0, 200, 0));
          INIT (FAILED, QColor (255, 0, 0));
          INIT (CANCELED, QColor (165, 42, 42));

#undef INIT

          settings.endGroup();

          return color_for_state;
        }
      }

      execution_monitor::execution_monitor()
        : QSplitter (Qt::Horizontal)
      {
        auto* available_transform_functions
          (new gspc::util::qt::mvc::transform_functions_model);
        auto* transform_functions
          (new gspc::util::qt::mvc::transform_functions_model);
        add_defaults (available_transform_functions);
        qRegisterMetaTypeStreamOperators
          <std::shared_ptr<gspc::util::qt::mvc::transform_functions_model::transform_function>>
          ("std::shared_ptr<gspc::util::qt::mvc::transform_functions_model::transform_function>");

        QAbstractItemModel* next (nullptr);

        base = new worker_model (this);
        next = base;

        auto* transformed_to_tree
          (new gspc::util::qt::mvc::flat_to_tree_proxy (next, transform_functions, this));
        next = transformed_to_tree;

        auto* filtered_by_user
          (new gspc::util::qt::mvc::filter_ignoring_branch_nodes_proxy (next, this));
        next = filtered_by_user;

        auto* with_view_meta_information
          (new execution_monitor_proxy (next, this));
        next = with_view_meta_information;

        auto* alphanum_sorted
          (new gspc::util::qt::mvc::alphanum_sort_proxy (next, this));
        next = alphanum_sorted;

        filtered_by_user->setFilterKeyColumn (0);
        alphanum_sorted->setDynamicSortFilter (true);


        auto* tree
          (new gspc::util::qt::treeview_with_delete (this));
        tree->setModel (next);

        auto* delegate
          ( new execution_monitor_delegate
            ( std::bind (set_filter_regexp, filtered_by_user, std::placeholders::_1)
            , std::bind (get_filter_regexp, filtered_by_user)
            , get_or_set_with_defaults()
            , tree
            )
          );
        tree->setItemDelegate (delegate);

        auto* header_view
          (new gspc::util::qt::mvc::delegating_header_view (tree));
        tree->setHeader (header_view);

        tree->setUniformRowHeights (true);
        tree->expandAll();

        header_view->setStretchLastSection (true);
        header_view->setSectionsClickable (true);
        header_view->setSortIndicatorShown (true);
        header_view->delegate (delegate);

        new gspc::util::qt::restricted_tree_column_sorter (tree, QSet<int>() << 0, this);


        auto* add_column (new QAction (tr ("add_column_action"), next));
        auto* remove_column (new QAction (tr ("remove_column_action"), next));

        connect
          (add_column, &QAction::triggered, std::bind (&add_columns, 1, next));
        connect
          (remove_column, &QAction::triggered, std::bind (&add_columns, -1, next));

        connect
          ( next, &QAbstractItemModel::columnsInserted
          , std::bind (&disable_if_column_adding_not_possible, add_column, 1, next)
          );
        connect
          ( next, &QAbstractItemModel::columnsRemoved
          , std::bind (&disable_if_column_adding_not_possible, add_column, 1, next)
          );
        connect
          ( next, &QAbstractItemModel::columnsInserted
          , std::bind (&disable_if_column_adding_not_possible, remove_column, -1, next)
          );
        connect
          ( next, &QAbstractItemModel::columnsRemoved
          , std::bind (&disable_if_column_adding_not_possible, remove_column, -1, next)
          );

        disable_if_column_adding_not_possible (add_column, 1, next);
        disable_if_column_adding_not_possible (remove_column, -1, next);

        //! \note This will only work as long as there is a
        //! freedesktop-style theme installed and found, which will
        //! not be the case on windows, osx, some unix setups.
        //! \todo fallback-icons
        add_column->setIcon (QIcon::fromTheme ("list-add"));
        remove_column->setIcon (QIcon::fromTheme ("list-remove"));

        auto* add_column_button
          (new gspc::util::qt::widget::mini_button (add_column, this));
        auto* remove_column_button
          (new gspc::util::qt::widget::mini_button (remove_column, this));


        auto* clear_model (new QAction (tr ("clear_action"), base));
        clear_model->setIcon (QIcon::fromTheme ("edit-clear"));

        connect
          ( clear_model, &QAction::triggered
          , this
          , [this, header_view]
            {
              //! \note HACK: for some reason the signal blocking in
              //! execution_monitor_editor::update() does not work,
              //! resulting in the _scrollbar->setValue() call to
              //! uncheck _automove (while that's working fine on all
              //! other invokations) when an editor is open while the
              //! model is reset. Thus, store which editor is
              //! visible, hide it, and show it again.
              std::optional<int> const section
                (header_view->current_editor());
              header_view->close_editor();

              base->clear();

              if (section)
              {
                header_view->request_editor (*section);
              }
            }
          );

        auto* clear_button
          (new gspc::util::qt::widget::mini_button (clear_model, this));


        auto* list_builder
          ( new gspc::util::qt::dual_list_selector ( available_transform_functions
                                             , transform_functions
                                             , this
                                             )
          );

        auto* sidebar (new QScrollArea (this));
        auto* sidebar_content (new QWidget (sidebar));
        sidebar->setWidget (sidebar_content);
        sidebar->setWidgetResizable (true);

        auto* sidebar_layout (new QVBoxLayout (sidebar_content));

        addWidget (tree);
        addWidget (sidebar);
        sidebar_layout->addWidget (add_column_button);
        sidebar_layout->addWidget (remove_column_button);
        sidebar_layout->addWidget (clear_button);
        sidebar_layout->addWidget (list_builder);


        const int current_count (next->columnCount());
        next->insertColumns (current_count, 2);

        next->setHeaderData
          ( current_count + 0
          , Qt::Horizontal
          , QVariant::fromValue (execution_monitor_proxy::current_states_column)
          , execution_monitor_proxy::column_type_role
          );
        next->setHeaderData
          ( current_count + 1
          , Qt::Horizontal
          , QVariant::fromValue (execution_monitor_proxy::gantt_column)
          , execution_monitor_proxy::column_type_role
          );
      }

      void execution_monitor::append_event (gspc::logging::message const& message)
      {
        base->append_event (message);
      }
    }
