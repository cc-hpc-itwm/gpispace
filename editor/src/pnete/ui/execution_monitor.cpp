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

#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <QAction>
#include <QTreeView>
#include <QVBoxLayout>

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


        layout()->addWidget (tree);
        layout()->addWidget (add_column_button);
        layout()->addWidget (remove_column_button);
        layout()->addWidget (list_builder);

        add_column->trigger();
      }
    }
  }
}
