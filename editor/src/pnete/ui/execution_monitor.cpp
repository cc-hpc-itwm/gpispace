#include <pnete/ui/execution_monitor.hpp>

#include <fhg/util/alphanum.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>

#include <QCheckBox>
#include <QColorDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QSlider>
#include <QSplitter>
#include <QString>
#include <QVBoxLayout>

#include <cmath>
#include <fstream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
  class function_call_appender : public fhg::log::Appender
  {
  public:
    typedef boost::function<void (const fhg::log::LogEvent&)> event_handler_t;

    function_call_appender (const event_handler_t& handler)
      : fhg::log::Appender ("event-handler")
      , m_handler (handler)
    { }

    void append (const fhg::log::LogEvent& evt)
    {
      m_handler (evt);
    }

    void flush()
    {
    }

  private:
    event_handler_t m_handler;
  };

  template<typename T>
    fhg::log::Appender::ptr_t appender_with
    ( void (T::* function)(const fhg::log::LogEvent&)
    , T* that
    )
  {
    return fhg::log::Appender::ptr_t
      (new function_call_appender (boost::bind (function, that, _1)));
  }
}

execution_monitor::execution_monitor (unsigned short port, QWidget* parent)
  : QWidget (parent)
  , m_follow_execution (true)
  , m_scene (new QGraphicsScene (this))
  , m_view (new QGraphicsView (m_scene))
  , m_component_scene (new QGraphicsScene (this))
  , m_component_view (new QGraphicsView (m_component_scene))
  , m_current_scale (1.0)
  , _automatically_sort_components (true)
  , _sort_gantt_trigger (false)
  , m_exe_server
    (appender_with (&execution_monitor::append_exe, this), m_io_service, port)
  , m_io_thread (boost::bind (&boost::asio::io_service::run, &m_io_service))
{
  m_view->setAlignment (Qt::AlignRight | Qt::AlignTop);
  m_view->setDragMode (QGraphicsView::ScrollHandDrag);
  m_view->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
  m_view->setTransformationAnchor (QGraphicsView::AnchorViewCenter);
  m_view->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
  m_view->setViewportUpdateMode (QGraphicsView::FullViewportUpdate);

  m_component_view->setAlignment (Qt::AlignRight | Qt::AlignTop);
  m_component_view->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
  m_component_view->setRenderHint (QPainter::Antialiasing);
  m_component_view->setTransformationAnchor (QGraphicsView::NoAnchor);
  m_component_view->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);

  connect ( m_view->verticalScrollBar(), SIGNAL (valueChanged (int))
          , m_component_view->verticalScrollBar(), SLOT (setValue (int))
          );


  QSplitter* task_view_widget (new QSplitter (Qt::Horizontal, this));

  task_view_widget->addWidget (m_component_view);
  task_view_widget->addWidget (m_view);

  task_view_widget->setSizes (QList<int>() << 0 << 1);

  QGroupBox* legend_box (new QGroupBox (tr ("Legend"), this));

  QSignalMapper* legend_label_click (new QSignalMapper (this));
  connect ( legend_label_click, SIGNAL (mapped (QString))
          , this, SLOT (change_gantt_color (QString))
          );

  {
    QVBoxLayout* legend_box_layout (new QVBoxLayout (legend_box));

    QSettings settings;
    settings.beginGroup ("gantt");

    foreach ( const QString& state
            , QStringList()
            << "created" << "started" << "finished" << "failed" << "cancelled"
            )
    {
      int r, g, b;
      settings.value (state).value<QColor>().getRgb (&r, &g, &b);

      QPushButton* label (new QPushButton (state, this));
      label->setStyleSheet
        (QString ("background-color: rgb(%1, %2, %3)").arg (r).arg (g).arg (b));
      legend_box_layout->addWidget (label);

      connect (label, SIGNAL (clicked()), legend_label_click, SLOT (map()));
      legend_label_click->setMapping (label, state);
    }

    settings.endGroup();
  }

  QGroupBox* control_box (new QGroupBox (tr ("Control"), this));

  QPushButton* clear_log_button (new QPushButton (tr ("Clear"), this));
  clear_log_button->setToolTip (tr ("Clear all events"));
  connect ( clear_log_button, SIGNAL (clicked())
          , this, SLOT (clearActivityLog())
          );

  QCheckBox* follow_logging_cb (new QCheckBox (tr ("follow"), this));
  follow_logging_cb->setChecked (true);
  follow_logging_cb->setToolTip ( tr ( "Follow the stream of log events and "
                                     "automatically scroll the view"
                                     )
                                );
  connect ( follow_logging_cb, SIGNAL (toggled (bool))
          , this, SLOT (toggleFollowTaskView (bool))
          );

  QCheckBox* automatically_sort_gantt
    (new QCheckBox (tr ("automatically sort by component"), this));
  automatically_sort_gantt->setChecked (true);

  connect ( automatically_sort_gantt, SIGNAL (toggled (bool))
          , this, SLOT (toggle_automatically_sort_components (bool))
          );

  QPushButton* sort_gantt (new QPushButton (tr ("sort by component"), this));
  connect ( sort_gantt, SIGNAL (clicked())
          , this, SLOT (sort_gantt_by_component())
          );

  QSlider* zoom_slider (new QSlider (this));
  zoom_slider->setMinimum (1);
  zoom_slider->setMaximum (800);
  zoom_slider->setValue (100);
  zoom_slider->setOrientation (Qt::Horizontal);
  zoom_slider->setTickPosition (QSlider::TicksAbove);
  zoom_slider->setTickInterval (100);

  connect ( zoom_slider, SIGNAL (valueChanged (int))
          , this, SLOT (changeTaskViewZoom (int))
          );


  QVBoxLayout* control_box_layout (new QVBoxLayout (control_box));
  control_box_layout->addWidget (clear_log_button);
  control_box_layout->addWidget (follow_logging_cb);
  control_box_layout->addWidget (sort_gantt);
  control_box_layout->addWidget (automatically_sort_gantt);
  control_box_layout->addWidget (zoom_slider);
  zoom_slider->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

  QVBoxLayout* execution_sidebar_layout (new QVBoxLayout);
  execution_sidebar_layout->addWidget (legend_box);
  execution_sidebar_layout->addStretch();
  execution_sidebar_layout->addWidget (control_box);

  QGridLayout* layout (new QGridLayout (this));
  layout->addWidget (task_view_widget, 0, 0);
  layout->addLayout (execution_sidebar_layout, 0, 1);


  connect (&_advance_timer, SIGNAL (timeout()), this, SLOT (advance()));

  _advance_timer.start (20 /*ms*/);
}

execution_monitor::~execution_monitor()
{
  m_io_service.stop();
  m_io_thread.join();
}

namespace
{
  void color_for_state (const QString& state, const QColor& color)
  {
    QSettings settings;

    settings.setValue ("gantt/" + state, color);
  }

  QColor color_for_state (const QString& state)
  {
    QSettings settings;

    return settings.value ("gantt/" + state).value<QColor>();
  }

  QColor color_for_state (const sdpa::daemon::NotificationEvent::state_t& state)
  {
    typedef sdpa::daemon::NotificationEvent event;

    switch (state)
    {
#define CHOICE(enummed,stringed)                    \
      case event::STATE_ ## enummed:                \
        return color_for_state (stringed)

      CHOICE (CREATED, "created");
      CHOICE (STARTED, "started");
      CHOICE (FINISHED, "finished");
      CHOICE (FAILED, "failed");
      CHOICE (CANCELLED, "cancelled");

#undef CHOICE
    case event::STATE_IGNORE:
      ;
    }

    throw std::runtime_error ("invalid state");
  }
}

class Task : public QGraphicsRectItem
{
public:
  Task ( const QString& component
       , const QString& name
       , const QString& id
       , QGraphicsItem* parent = NULL
       )
    : QGraphicsRectItem (parent)
    , _do_advance (true)
    , _state (sdpa::daemon::NotificationEvent::STATE_CREATED)
  {
    setToolTip (QObject::tr ("%1 on %2 (id = %3)").arg (name, component, id));
    setRect (0.0, 0.0, 0.5, 8.0);
    reset_color();
  }

  void update_task_state (sdpa::daemon::NotificationEvent::state_t state)
  {
    _state = state;
    _do_advance = _state < sdpa::daemon::NotificationEvent::STATE_FINISHED;
    reset_color();
  }

  void advance (const qreal scene_width)
  {
    if (_do_advance)
    {
      static const qreal height (8.0);

      setRect (0.0, 0.0, std::floor (scene_width - pos().x() + 0.5), height);
    }
  }

  void reset_color()
  {
    setBrush (color_for_state (_state));
    update();
  }

  enum { Type = UserType + 1 };
  int type() const { return Type; }

private:
  bool _do_advance;
  sdpa::daemon::NotificationEvent::state_t _state;
};

void execution_monitor::change_gantt_color (const QString& state)
{
  QColor new_color
    ( QColorDialog::getColor ( color_for_state (state)
                             , this
                             , tr ("Select new color for state %1").arg (state)
                             )
    );

  if (new_color.isValid())
  {
    color_for_state (state, new_color);

    int r, g, b;
    new_color.getRgb (&r, &g, &b);

    //! \todo This is _really_ ugly code. Any way to solve this more nicely?
    qobject_cast<QWidget*>
      (qobject_cast<QSignalMapper*> (sender())->mapping (state))->setStyleSheet
      (QString ("background-color: rgb(%1, %2, %3)").arg (r).arg (g).arg (b));

    {
      foreach (QGraphicsItem* item, m_scene->items())
      {
        if (Task* task = qgraphicsitem_cast<Task*> (item))
        {
          task->reset_color();
        }
      }
    }
  }
}

void execution_monitor::advance()
{
  std::list<scene_update_t> updates;
  std::list<component_update_t> component_updates;
  {
    boost::unique_lock<boost::recursive_mutex> lock (_scene_updates_lock);
    updates.swap (_scene_updates);
    std::swap (component_updates, _component_updates);
  }

  BOOST_FOREACH (const scene_update_t& update, updates)
  {
    update.second->addItem (update.first);
  }

  BOOST_FOREACH (const component_update_t& update, component_updates)
  {
    update.first->setParentItem (_component_dummies[update.second]);
  }

  QRectF scene_rect (m_scene->sceneRect());
  scene_rect.setWidth (scene_rect.width() + 1.0);
  scene_rect.setHeight (m_components.size() * 8);
  m_scene->setSceneRect (scene_rect);

  //! \todo do not call advance on all last tasks, but just advance
  // all 'active' elements.  i.e.  keep a list of currently active
  // elements
  BOOST_FOREACH (const std::string& component, m_components)
  {
    if (!m_tasks_list[component].empty())
    {
      m_tasks_list[component].back()->advance (scene_rect.width());
    }
  }

  if (_sort_gantt_trigger)
  {
    sort_gantt_by_component();
    _sort_gantt_trigger = false;
  }

  if (m_follow_execution)
  {
    m_view->horizontalScrollBar()->setValue
      (m_view->horizontalScrollBar()->maximum());
  }
}

void execution_monitor::append_exe (const fhg::log::LogEvent& event)
{
  static const qreal task_height (8.0);

  const sdpa::daemon::NotificationEvent notification (event.message());

  const sdpa::daemon::NotificationEvent::state_t task_state
    (notification.activity_state());

  if (task_state == sdpa::daemon::NotificationEvent::STATE_IGNORE)
  {
    return;
  }

  const std::string& component (notification.component());

  const boost::unique_lock<boost::recursive_mutex> lock (m_task_struct_mutex);

  const std::vector<std::string>::iterator comp
    (std::find (m_components.begin(), m_components.end(), component));
  const bool is_new_component (comp == m_components.end());

  const qreal x_coord (m_scene->width());
  const qreal y_coord ( !is_new_component
                      ? std::distance (m_components.begin(), comp) * task_height
                      : m_components.size() * task_height
                      );

  if (is_new_component)
  {
    m_components.push_back (component);

    QGraphicsSimpleTextItem* label
      (new QGraphicsSimpleTextItem (QString::fromStdString (component)));

    QFont font (label->font());
    font.setPointSize (6);
    label->setFont (font);
    label->setPos (0, y_coord);

    boost::unique_lock<boost::recursive_mutex> lock (_scene_updates_lock);

    _scene_updates.push_back (std::make_pair (label, m_component_scene));
    _component_labels[component] = label;

    {
      QGraphicsItemGroup* dummy (new QGraphicsItemGroup);
      dummy->setPos (0, y_coord);
      _scene_updates.push_back (std::make_pair (dummy, m_scene));
      _component_dummies[component] = dummy;
    }
  }

  const std::string& activity_id (notification.activity_id());

  if ( m_tasks_grid[component].find (activity_id)
     == m_tasks_grid[component].end()
     )
  {
    Task* task ( new Task ( QString::fromStdString (component)
                          , QString::fromStdString (notification.activity_name())
                          , QString::fromStdString (activity_id)
                          )
               );
    task->setX (x_coord);
    m_tasks_grid[component][activity_id] = task;


    // new task, make sure to close previous task -> asume finished
    if (!m_tasks_list[component].empty())
    {
      m_tasks_list[component].back()->update_task_state
        (sdpa::daemon::NotificationEvent::STATE_FINISHED);
    }
    m_tasks_list[component].push_back (task);

    boost::unique_lock<boost::recursive_mutex> lock (_scene_updates_lock);
    _component_updates.push_back (std::make_pair (task, component));
  }

  m_tasks_grid[component][activity_id]->update_task_state (task_state);

  if (_automatically_sort_components && is_new_component)
  {
    _sort_gantt_trigger = true;
  }
}

void execution_monitor::toggle_automatically_sort_components (bool new_value)
{
  _automatically_sort_components = new_value;
}

void execution_monitor::sort_gantt_by_component()
{
  const boost::unique_lock<boost::recursive_mutex> lock (m_task_struct_mutex);

  static const qreal task_height (8.0);

  std::sort
    (m_components.begin(), m_components.end(), fhg::util::alphanum::less());

  int row (0);
  BOOST_FOREACH (const std::string& component, m_components)
  {
    const qreal y_coord (row++ * task_height);

    _component_labels[component]->setY (y_coord);
    _component_dummies[component]->setY (y_coord);
  }
}

void execution_monitor::clearActivityLog()
{
  boost::unique_lock<boost::recursive_mutex> struct_lock (m_task_struct_mutex);

  delete m_scene;
  delete m_component_scene;
  m_scene = new QGraphicsScene (this);
  m_component_scene = new QGraphicsScene (this);

  m_view->setScene (m_scene);
  m_component_view->setScene (m_component_scene);

  m_tasks_grid.clear();
  m_tasks_list.clear();
  m_components.clear();

  {
    boost::unique_lock<boost::recursive_mutex> update_lock (_scene_updates_lock);
    while (!_scene_updates.empty())
    {
      delete _scene_updates.front().first;
      _scene_updates.pop_front();
    }
    while (!_component_updates.empty())
    {
      delete _component_updates.front().first;
      _component_updates.pop_front();
    }
  }
}

void execution_monitor::toggleFollowTaskView (bool follow)
{
  m_follow_execution = follow;
}

void execution_monitor::changeTaskViewZoom (int to)
{
  qreal target = (to / 100.0);
  qreal factor = target / m_current_scale;

  m_view->scale (factor, factor);
  m_component_view->scale (factor, factor);

  m_current_scale = target;
}
