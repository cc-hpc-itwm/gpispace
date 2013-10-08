// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_NODE_STATE_WIDGET_HPP
#define FHG_PNETE_UI_NODE_STATE_WIDGET_HPP

#include <pnete/ui/monitor_client.hpp>

#include <boost/optional.hpp>

#include <QColor>
#include <QDateTime>
#include <QList>
#include <QListWidget>
#include <QMap>
#include <QPixmap>
#include <QStringList>
#include <QTimer>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      struct state_description
      {
        QStringList _actions;
        boost::optional<char> _character;
        QColor _brush;
        QColor _pen;
        bool _hidden;

        QPixmap _pixmap;

        state_description ( const QStringList& actions = QStringList()
                          , boost::optional<char> = boost::none
                          , QColor brush = Qt::lightGray
                          , QColor pen = Qt::black
                          );
        void reset();
      };

      // ------------------------------------------------------------------------

      class legend : public QWidget
      {
        Q_OBJECT;

      public:
        legend (monitor_client*, QWidget* parent = NULL);

        const state_description& state (const boost::optional<QString>&) const;

      public slots:
        void update (const QString&);
        void states_add (const QString&, const QStringList&);
        void states_layout_hint_border (const QString&, const QColor&);
        void states_layout_hint_character (const QString&, const char&);
        void states_layout_hint_color (const QString&, const QColor&);
        void states_layout_hint_hidden (const QString&, const bool&);

      signals:
        void state_pixmap_changed (const QString&);

      private:
        QMap<QString, state_description> _states;
        QMap<QString, QWidget*> _state_legend;
      };

      // ------------------------------------------------------------------------

      class log_widget : public QListWidget
      {
        Q_OBJECT;

      public:
        log_widget (QWidget* parent = NULL);

        void critical (const QString&);
        void information (const QString&);
        void warning (const QString&);

      public slots:
        void follow (bool);
      };

      // ------------------------------------------------------------------------

      class node_state_widget : public QWidget
      {
        Q_OBJECT;

      public:
        node_state_widget ( const QString& window_title
                          , legend*
                          , log_widget*
                          , monitor_client*
                          , QWidget* parent = NULL
                          );

        virtual int heightForWidth (int) const;

      protected:
        virtual void paintEvent (QPaintEvent*);
        virtual bool event (QEvent*);

        virtual void mouseReleaseEvent (QMouseEvent*);

      private:
        //! \note Moc breaks with namespaces..
        typedef monitor_client::action_argument_data action_argument_data;
        typedef monitor_client::action_result_code action_result_code;

      private slots:
        void refresh_stati();
        void nodes (QStringList);
        void nodes_details (const QString&, const QString&);
        void nodes_state (const QString&, const boost::optional<QString>&);
        void states_actions_long_text (const QString&, const QString&);
        void states_actions_arguments
          (const QString&, const QList<action_argument_data>&);
        void states_actions_expected_next_state (const QString&, const QString&);

        void update_nodes_with_state (const QString&);
        void trigger_action (const QStringList& hosts, const QString& action);

        void action_result ( const QString&
                           , const QString&
                           , const action_result_code&
                           , const boost::optional<QString>&
                           );

        void sort_by_name();
        void sort_by_state();

        void select_all();
        void clear_selection();

      private:
        struct node_type
        {
          node_type (const QString& hostname)
            : _state (boost::none)
            , _hostname (hostname)
            , _details (boost::none)
            , _watched (false)
            , _expects_state_change (boost::none)
          { }

          boost::optional<QString> _state;
          QDateTime _state_update_time;
          QString _hostname;
          boost::optional<QString> _details;
          bool _watched;
          void watched (bool w) { _watched = w; }
          boost::optional<QString> _expects_state_change;
        };

        void sort_by (boost::function<bool (const node_type&, const node_type&)>);

        QMap<QString, QString> _long_action;
        QMap<QString, QList<monitor_client::action_argument_data> > _action_arguments;
        QMap<QString, QString> _action_expects_next_state;

        QSet<QString> _pending_updates;
        QSet<QString> _nodes_to_update;
        QSet<QString> _ignore_next_nodes_state;

        void update (int node);
        void update();

        QList<node_type> _nodes;
        QMap<QString, size_t> _node_index_by_hostname;
        QList<int> _selection;

        void add_to_selection (const int&);
        void remove_from_selection (const int&);

        boost::optional<int> _last_manual_selection;

        legend* _legend_widget;
        log_widget* _log;

        const state_description& state (const boost::optional<QString>&) const;
        boost::optional<size_t> node_index_by_name (const QString&) const;
        void rebuild_node_index();
        const node_type& node (int) const;
        node_type& node (int);
        boost::optional<int> node_at (const QPoint&) const;
        int node_count() const;

        monitor_client* _monitor_client;

        QString _window_title;
      };
    }
  }
}

#endif
