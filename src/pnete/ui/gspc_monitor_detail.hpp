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

#include <pnete/ui/gspc_monitor_client.hpp>

#include <QColor>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>

#include <boost/optional.hpp>

#include <functional>

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
        boost::optional<QString> _descriptive_name;

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
        Q_OBJECT

      public:
        legend (monitor_client*, QWidget* parent = nullptr);

        const state_description& state (const boost::optional<QString>&) const;

      public slots:
        void update (const QString&);
        void states_add (const QString&, const QStringList&);
        void states_layout_hint_border (const QString&, const QColor&);
        void states_layout_hint_character (const QString&, const char&);
        void states_layout_hint_color (const QString&, const QColor&);
        void states_layout_hint_hidden (const QString&, const bool&);
        void states_layout_hint_descriptive_name (const QString&, const QString&);

      signals:
        void state_pixmap_changed (const QString&);

      private:
        QMap<QString, state_description> _states;
        QMap<QString, QWidget*> _state_legend;
      };

      // ------------------------------------------------------------------------

      class log_widget : public QTableWidget
      {
        Q_OBJECT

      public:
        log_widget (QWidget* parent = nullptr);

        void critical (QString host, const QString&, QStringList);
        void information (QString host, const QString&, QStringList);
        void warning (QString host, const QString&, QStringList);

      public slots:
        void follow (bool);
        void clearContents ();
      };

      // ------------------------------------------------------------------------

      class node_state_widget : public QWidget
      {
        Q_OBJECT

      public:
        node_state_widget ( const QString& window_title
                          , legend*
                          , log_widget*
                          , monitor_client*
                          , QWidget* parent = nullptr
                          );

        virtual int heightForWidth (int) const override;

      protected:
        virtual void paintEvent (QPaintEvent*) override;
        virtual bool event (QEvent*) override;

        virtual void mouseReleaseEvent (QMouseEvent*) override;

      private:
        //! \note Moc breaks with namespaces..
        typedef monitor_client::action_argument_data action_argument_data;
        typedef monitor_client::action_result_code action_result_code;

      private slots:
        void refresh_stati();
        void nodes (QStringList);
        void nodes_details (const QString&, const boost::optional<QString>&);
        void nodes_state (const QString&, const boost::optional<QString>&);
        void states_actions_long_text (const QString&, const QString&);
        void states_actions_requires_confirmation (const QString&, bool);
        void states_actions_arguments
          (const QString&, const QList<action_argument_data>&);
        void states_actions_expected_next_state (const QString&, const QString&);

        void update_nodes_with_state (const QString&);
        void trigger_action (const QStringList& hosts, const QSet<int>& host_ids, const QString& action);

        void action_result ( const QString& host
                           , const QString& action
                           , const action_result_code&
                           , const boost::optional<QString>& message
                           , QList<QPair<QString, QString>> additional_data
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

        void sort_by (std::function<bool (const node_type&, const node_type&)>);

        QMap<QString, QString> _long_action;
        QSet<QString> _action_requires_confirmation;
        QMap<QString, QList<monitor_client::action_argument_data>> _action_arguments;
        QMap<QString, QString> _action_expects_next_state;

        QString full_action_name (QString, const QSet<int>& host_ids) const;

        QSet<QString> _pending_updates;
        QSet<QString> _nodes_to_update;
        QSet<QString> _ignore_next_nodes_state;

        void update_node (int node);
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
