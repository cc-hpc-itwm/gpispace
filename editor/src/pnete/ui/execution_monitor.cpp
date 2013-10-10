#include <pnete/ui/execution_monitor.hpp>

#include <pnete/ui/execution_monitor_worker_model.hpp>
#include <pnete/ui/execution_monitor_detail.hpp>

#include <util/qt/boost_connect.hpp>
#include <util/qt/dual_list_selector.hpp>
#include <util/qt/mini_button.hpp>
#include <util/qt/mvc/alphanum_sort_proxy.hpp>
#include <util/qt/mvc/delegating_header_view.hpp>
#include <util/qt/mvc/filter_ignoring_branch_nodes_proxy.hpp>
#include <util/qt/mvc/flat_to_tree_proxy.hpp>
#include <util/qt/restricted_tree_column_sorter.hpp>
#include <util/qt/variant.hpp>

#include <boost/foreach.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <QAction>
#include <QColorDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QSettings>
#include <QTreeView>
#include <QVBoxLayout>

Q_DECLARE_METATYPE (sdpa::daemon::NotificationEvent::state_t)

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        using util::qt::mvc::transform_functions_model;

        struct nth_substring_of_name
          : public transform_functions_model::transform_function
        {
          nth_substring_of_name()
            : _n (0)
            , _sep ('\0')
          { }
          nth_substring_of_name (int n, char sep)
            : _n (n)
            , _sep (sep)
          { }

          virtual QString operator() (QModelIndex index) const
          {
            return util::qt::value<QString>
              (index.data (worker_model::name_role)).split (_sep)[_n];
          }

          template<class Archive>
            void serialize (Archive& ar, const unsigned int)
          {
            ar & _n & _sep;
            boost::serialization::void_cast_register
              ( static_cast<nth_substring_of_name*> (NULL)
              , static_cast<transform_functions_model::transform_function*> (NULL)
              );
          }

        private:
          int _n;
          char _sep;
        };
      }
    }
  }
}

BOOST_CLASS_EXPORT (fhg::pnete::ui::nth_substring_of_name)

template<typename T>
  QDataStream& operator<< (QDataStream& stream, const boost::shared_ptr<T>& ptr)
{
  std::ostringstream os;
  boost::archive::text_oarchive oa (os);
  oa << ptr;
  stream << QString::fromStdString (os.str());
  return stream;
}
template<typename T>
  QDataStream& operator>> (QDataStream& stream, boost::shared_ptr<T>& ptr)
{
  QString serialized;
  stream >> serialized;
  std::istringstream is (serialized.toStdString());
  boost::archive::text_iarchive ia (is);
  ia >> ptr;
  return stream;
}

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        using util::qt::mvc::transform_functions_model;

        //! \todo Other defaults? User defined functions?
        QMap<int, QVariant> item
          (QString name, transform_functions_model::transform_function* f)
        {
          QMap<int, QVariant> roles;
          roles[Qt::DisplayRole] = name;
          roles[transform_functions_model::function_role]
            = QVariant::fromValue
            (boost::shared_ptr<transform_functions_model::transform_function> (f));
          return roles;
        }

        void add_defaults (transform_functions_model* model)
        {
          model->insertRows (0, 2);
          model->setItemData
            ( model->index (0, 0)
            , item ("by worker type", new nth_substring_of_name (0, '-'))
            );
          model->setItemData
            ( model->index (1, 0)
            , item ("by node", new nth_substring_of_name (1, '-'))
            );
        }

        void set_filter_regexp
          (util::qt::mvc::filter_ignoring_branch_nodes_proxy* model, QString regexp)
        {
          model->setFilterRegExp (regexp);
        }
        QString get_filter_regexp
          (util::qt::mvc::filter_ignoring_branch_nodes_proxy* model)
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
          typedef sdpa::daemon::NotificationEvent event;

          switch (state)
          {
          case event::STATE_CREATED: return "created";
          case event::STATE_STARTED: return "started";
          case event::STATE_FINISHED: return "finished";
          case event::STATE_FAILED: return "failed";
          case event::STATE_CANCELLED: return "cancelled";
          }
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
          return util::qt::value<QColor> (value);
        }

        QMap<worker_model::state_type, QColor> get_or_set_with_defaults()
        {
          QSettings settings;
          settings.beginGroup ("gantt");

          QMap<worker_model::state_type, QColor> color_for_state;

          typedef sdpa::daemon::NotificationEvent event;

#define INIT(enummed, dflt)                                             \
          color_for_state[event:: STATE_ ## enummed]                    \
            = get_or_set_with_default                                   \
              (settings, to_string (event:: STATE_ ## enummed), dflt)

          INIT (CREATED, QColor (128, 128, 128));
          INIT (STARTED, QColor (255, 255, 0));
          INIT (FINISHED, QColor (0, 200, 0));
          INIT (FAILED, QColor (255, 0, 0));
          INIT (CANCELLED, QColor (165, 42, 42));

#undef INIT

          settings.endGroup();

          return color_for_state;
        }

        void style_button (QPushButton* button, QColor color)
        {
          int r, g, b;
          color.getRgb (&r, &g, &b);

          button->setStyleSheet
            (QString ("background-color: rgb(%1, %2, %3)").arg (r).arg (g).arg (b));
        }

        void maybe_style_button
          ( QPushButton* button, worker_model::state_type state_req
          , worker_model::state_type state, QColor color
          )
        {
          if (state_req == state)
          {
            style_button (button, color);
          }
        }

        void change_gantt_color ( execution_monitor_delegate* delegate
                                , QPushButton* button
                                , worker_model::state_type state
                                , QWidget* parent
                                )
        {
          const QString stringified_state (to_string (state));

          QColor new_color
            ( QColorDialog::getColor
              ( delegate->color_for_state (state)
              , parent
              , QObject::tr ("Select new color for state %1").arg (stringified_state)
              )
            );

          if (new_color.isValid())
          {
            delegate->color_for_state (state, new_color);

            style_button (button, new_color);

            QSettings settings;
            settings.setValue ("gantt/" + stringified_state, new_color);
          }
        }
      }

      execution_monitor::execution_monitor (unsigned short port, QWidget* parent)
        : QWidget (parent)
      {
        new QVBoxLayout (this);

        util::qt::mvc::transform_functions_model* available_transform_functions
          (new util::qt::mvc::transform_functions_model);
        util::qt::mvc::transform_functions_model* transform_functions
          (new util::qt::mvc::transform_functions_model);
        add_defaults (available_transform_functions);
        qRegisterMetaTypeStreamOperators
          <boost::shared_ptr<fhg::util::qt::mvc::transform_functions_model::transform_function> >
          ("boost::shared_ptr<fhg::util::qt::mvc::transform_functions_model::transform_function>");

        QAbstractItemModel* next (NULL);

        worker_model* base (new worker_model (port, this));
        next = base;

        util::qt::mvc::flat_to_tree_proxy* transformed_to_tree
          (new util::qt::mvc::flat_to_tree_proxy (next, transform_functions, this));
        next = transformed_to_tree;

        util::qt::mvc::filter_ignoring_branch_nodes_proxy* filtered_by_user
          (new util::qt::mvc::filter_ignoring_branch_nodes_proxy (next, this));
        next = filtered_by_user;

        execution_monitor_proxy* with_view_meta_information
          (new execution_monitor_proxy (next, this));
        next = with_view_meta_information;

        util::qt::mvc::alphanum_sort_proxy* alphanum_sorted
          (new util::qt::mvc::alphanum_sort_proxy (next, this));
        next = alphanum_sorted;

        filtered_by_user->setFilterKeyColumn (0);


        QTreeView* tree (new QTreeView (this));
        tree->setModel (next);

        execution_monitor_delegate* delegate
          ( new execution_monitor_delegate
            ( boost::bind (set_filter_regexp, filtered_by_user, _1)
            , boost::bind (get_filter_regexp, filtered_by_user)
            , get_or_set_with_defaults()
            , tree
            )
          );
        tree->setItemDelegate (delegate);

        util::qt::mvc::delegating_header_view* header_view
          (new util::qt::mvc::delegating_header_view (tree));
        tree->setHeader (header_view);

        tree->setUniformRowHeights (true);
        tree->expandAll();

        header_view->setStretchLastSection (true);
        header_view->setClickable (true);
        header_view->setSortIndicatorShown (true);
        header_view->delegate (delegate);

        new util::qt::restricted_tree_column_sorter (tree, QSet<int>() << 0, this);


        QAction* add_column
          (new QAction (QObject::tr ("add_column_action"), next));
        QAction* remove_column
          (new QAction (QObject::tr ("remove_column_action"), next));

        util::qt::boost_connect<void()>
          (add_column, SIGNAL (triggered()), boost::bind (&add_columns, 1, next));
        util::qt::boost_connect<void()>
          (remove_column, SIGNAL (triggered()), boost::bind (&add_columns, -1, next));

        util::qt::boost_connect<void()>
          ( next, SIGNAL (columnsInserted (QModelIndex, int, int))
          , boost::bind (&disable_if_column_adding_not_possible, add_column, 1, next)
          );
        util::qt::boost_connect<void()>
          ( next, SIGNAL (columnsRemoved (QModelIndex, int, int))
          , boost::bind (&disable_if_column_adding_not_possible, add_column, 1, next)
          );
        util::qt::boost_connect<void()>
          ( next, SIGNAL (columnsInserted (QModelIndex, int, int))
          , boost::bind (&disable_if_column_adding_not_possible, remove_column, -1, next)
          );
        util::qt::boost_connect<void()>
          ( next, SIGNAL (columnsRemoved (QModelIndex, int, int))
          , boost::bind (&disable_if_column_adding_not_possible, remove_column, -1, next)
          );

        disable_if_column_adding_not_possible (add_column, 1, next);
        disable_if_column_adding_not_possible (remove_column, -1, next);

        //! \note This will only work as long as there is a
        //! freedesktop-style theme installed and found, which will
        //! not be the case on windows, osx, some unix setups.
        //! \todo fallback-icons
        add_column->setIcon (QIcon::fromTheme ("list-add"));
        remove_column->setIcon (QIcon::fromTheme ("list-remove"));

        util::qt::mini_button* add_column_button
          (new util::qt::mini_button (add_column, this));
        util::qt::mini_button* remove_column_button
          (new util::qt::mini_button (remove_column, this));


        util::qt::dual_list_selector* list_builder
          ( new util::qt::dual_list_selector ( available_transform_functions
                                             , transform_functions
                                             , this
                                             )
          );



        QGroupBox* legend_box (new QGroupBox (tr ("Legend"), this));

        {
          QVBoxLayout* legend_box_layout (new QVBoxLayout (legend_box));

          static const QList<worker_model::state_type> all_states
            ( QList<worker_model::state_type>()
            << sdpa::daemon::NotificationEvent::STATE_CREATED
            << sdpa::daemon::NotificationEvent::STATE_STARTED
            << sdpa::daemon::NotificationEvent::STATE_FINISHED
            << sdpa::daemon::NotificationEvent::STATE_FAILED
            << sdpa::daemon::NotificationEvent::STATE_CANCELLED
            );

          BOOST_FOREACH (const worker_model::state_type& state, all_states)
          {
            QPushButton* label (new QPushButton (to_string (state), this));
            style_button (label, delegate->color_for_state (state));
            legend_box_layout->addWidget (label);

            util::qt::boost_connect<void()>
              ( label
              , SIGNAL (clicked())
              , this
              , boost::bind (&change_gantt_color, delegate, label, state, this)
              );

            util::qt::boost_connect<void (worker_model::state_type, QColor)>
              ( delegate
              , SIGNAL (color_for_state_changed (worker_model::state_type, QColor))
              , this
              , boost::bind (&maybe_style_button, label, state, _1, _2)
              );
          }
        }

        layout()->addWidget (tree);
        layout()->addWidget (add_column_button);
        layout()->addWidget (remove_column_button);
        layout()->addWidget (list_builder);
        layout()->addWidget (legend_box);

        add_column->trigger();
      }
    }
  }
}
