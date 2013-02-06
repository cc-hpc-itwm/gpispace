#include "monitorwindow.hpp"
#include "logeventwrapper.hpp"
#include "windowappender.hpp"
#include "task.h"
#include <QHeaderView>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QSplitter>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>

#include <boost/lexical_cast.hpp>
#include <list>
#include <sstream>
#include <cmath>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/access.hpp>
#include <boost/bind.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/loader/putget.hpp>
#include <we/type/value.hpp>

using namespace std;
using namespace boost;

enum external_events
{
  EXTERNAL_EVENT_LOGGING = QEvent::User + 1,
};

MonitorWindow::MonitorWindow( unsigned short exe_port
                            , unsigned short log_port
                            , QWidget *parent
                            )
  : QMainWindow(parent)
  , m_log_server ( fhg::log::Appender::ptr_t
                   ( new WindowAppender
                     ( boost::bind
                        ( &MonitorWindow::handle_external_event
                        , this
                        , EXTERNAL_EVENT_LOGGING
                        , _1
                        )
                     )
                   )
                 , m_io_service, log_port
                 )
  , m_exe_server ( fhg::log::Appender::ptr_t
                   ( new WindowAppender
                     (boost::bind (&MonitorWindow::append_exe, this, _1))
                   )
                 , m_io_service, exe_port
                 )
  , m_io_thread (boost::bind (&boost::asio::io_service::run, &m_io_service))
  , m_follow_logging (true)
  , m_follow_execution (true)
  , m_current_scale (1.0)
{
{

  QWidget* centralWidget = new QWidget(this);
  centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
  QGridLayout* gridLayout_9 = new QGridLayout(centralWidget);
  gridLayout_9->setSpacing(6);
  gridLayout_9->setContentsMargins(11, 11, 11, 11);
  gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
  QTabWidget* SDPAGUI = new QTabWidget(centralWidget);
  SDPAGUI->setObjectName(QString::fromUtf8("SDPAGUI"));
  SDPAGUI->setEnabled(true);
  SDPAGUI->setAutoFillBackground(true);
  SDPAGUI->setDocumentMode(false);
  QWidget* logging_tab = new QWidget();
  logging_tab->setObjectName(QString::fromUtf8("logging_tab"));
  QGridLayout* gridLayout_4 = new QGridLayout(logging_tab);
  gridLayout_4->setSpacing(6);
  gridLayout_4->setContentsMargins(11, 11, 11, 11);
  gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
  QGroupBox* groupBox_3 = new QGroupBox(logging_tab);
  groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
  QGridLayout* gridLayout_3 = new QGridLayout(groupBox_3);
  gridLayout_3->setSpacing(6);
  gridLayout_3->setContentsMargins(11, 11, 11, 11);
  gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));

  m_log_table = new QTableWidget(groupBox_3);
  m_log_table->setAlternatingRowColors(false);
  m_log_table->setAutoFillBackground(false);
  m_log_table->setColumnCount(4);
  m_log_table->setCornerButtonEnabled(false);
  m_log_table->setEditTriggers(QAbstractItemView::SelectedClicked);
  m_log_table->setEnabled(true);
  m_log_table->setGridStyle(Qt::NoPen);
  m_log_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_log_table->setLineWidth(0);
  m_log_table->setRowCount(0);
  m_log_table->setSelectionMode(QAbstractItemView::NoSelection);
  m_log_table->setShowGrid(false);
  m_log_table->setSortingEnabled(false);
  m_log_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_log_table->setWordWrap(false);

  m_log_table->verticalHeader()->setVisible (false);

  m_log_table->setHorizontalHeaderLabels ( QStringList()
                                         << tr ("Time")
                                         << tr ("Source")
                                         << tr ("Location")
                                         << tr ("Message")
                                         );
  m_log_table->horizontalHeader()->setStretchLastSection(true);

  gridLayout_3->addWidget(m_log_table, 0, 0, 1, 1);
  gridLayout_4->addWidget(groupBox_3, 0, 0, 1, 1);

  QVBoxLayout* verticalLayout_5 = new QVBoxLayout();
  verticalLayout_5->setSpacing(6);
  verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
  QGroupBox* groupBox_5 = new QGroupBox(logging_tab);
  groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
  QVBoxLayout* verticalLayout_3 = new QVBoxLayout(groupBox_5);
  verticalLayout_3->setSpacing(6);
  verticalLayout_3->setContentsMargins(11, 11, 11, 11);
  verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
  QDial* log_filter_dial = new QDial(groupBox_5);
  log_filter_dial->setObjectName(QString::fromUtf8("log_filter_dial"));
  log_filter_dial->setMaximum(5);
  log_filter_dial->setValue(2);
  log_filter_dial->setSliderPosition(2);
  log_filter_dial->setOrientation(Qt::Horizontal);
  log_filter_dial->setInvertedAppearance(false);
  log_filter_dial->setInvertedControls(false);

  verticalLayout_3->addWidget(log_filter_dial);

  m_level_filter_selector = new QComboBox(groupBox_5);
  m_level_filter_selector->setObjectName(QString::fromUtf8("m_level_filter_selector"));

  verticalLayout_3->addWidget(m_level_filter_selector);

  verticalLayout_5->addWidget(groupBox_5);

  QSpacerItem* verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

  verticalLayout_5->addItem(verticalSpacer_2);

  QGroupBox* groupBox_4 = new QGroupBox(logging_tab);
  groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
  QVBoxLayout* verticalLayout_6 = new QVBoxLayout(groupBox_4);
  verticalLayout_6->setSpacing(6);
  verticalLayout_6->setContentsMargins(11, 11, 11, 11);
  verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
  QPushButton* clear_log_button = new QPushButton(groupBox_4);
  clear_log_button->setObjectName(QString::fromUtf8("clear_log_button"));

  verticalLayout_6->addWidget(clear_log_button);

  QCheckBox* follow_logging_cb = new QCheckBox(groupBox_4);
  follow_logging_cb->setObjectName(QString::fromUtf8("follow_logging_cb"));
  follow_logging_cb->setChecked(true);

  verticalLayout_6->addWidget(follow_logging_cb);

  m_drop_filtered = new QCheckBox(groupBox_4);
  m_drop_filtered->setCheckState(Qt::Checked);
  m_drop_filtered->setToolTip(tr ("Drop filtered events instead of keeping them"));
  m_drop_filtered->setText(tr ("drop filtered"));

  verticalLayout_6->addWidget(m_drop_filtered);


  verticalLayout_5->addWidget(groupBox_4);


  gridLayout_4->addLayout(verticalLayout_5, 0, 1, 1, 1);

  SDPAGUI->addTab(logging_tab, QString());
  QWidget* execution_tab = new QWidget();
  execution_tab->setObjectName(QString::fromUtf8("execution_tab"));
  QGridLayout* gridLayout_5 = new QGridLayout(execution_tab);
  gridLayout_5->setSpacing(6);
  gridLayout_5->setContentsMargins(11, 11, 11, 11);
  gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
  QGroupBox* groupBox_6 = new QGroupBox(execution_tab);
  groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
  QGridLayout* gridLayout_7 = new QGridLayout(groupBox_6);
  gridLayout_7->setSpacing(6);
  gridLayout_7->setContentsMargins(11, 11, 11, 11);
  gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
  task_view_widget = new QWidget(groupBox_6);
  task_view_widget->setObjectName(QString::fromUtf8("task_view_widget"));
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(task_view_widget->sizePolicy().hasHeightForWidth());
  task_view_widget->setSizePolicy(sizePolicy);

  gridLayout_7->addWidget(task_view_widget, 0, 0, 1, 1);

  QGroupBox* groupBox = new QGroupBox(groupBox_6);
  groupBox->setObjectName(QString::fromUtf8("groupBox"));
  QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
  groupBox->setSizePolicy(sizePolicy1);
  groupBox->setMaximumSize(QSize(150, 16777215));
  QVBoxLayout* verticalLayout_2 = new QVBoxLayout(groupBox);
  verticalLayout_2->setSpacing(6);
  verticalLayout_2->setContentsMargins(11, 11, 11, 11);
  verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
  QLabel* label_2 = new QLabel(groupBox);
  label_2->setObjectName(QString::fromUtf8("label_2"));
  label_2->setAutoFillBackground(false);
  label_2->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 0)"));

  verticalLayout_2->addWidget(label_2);

  QLabel* label = new QLabel(groupBox);
  label->setObjectName(QString::fromUtf8("label"));
  label->setAutoFillBackground(false);
  label->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 255, 0);"));

  verticalLayout_2->addWidget(label);

  QLabel* label_3 = new QLabel(groupBox);
  label_3->setObjectName(QString::fromUtf8("label_3"));
  label_3->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));

  verticalLayout_2->addWidget(label_3);

  QSpacerItem* verticalSpacer = new QSpacerItem(118, 68, QSizePolicy::Minimum, QSizePolicy::Expanding);

  verticalLayout_2->addItem(verticalSpacer);

  QGroupBox* groupBox_2 = new QGroupBox(groupBox);
  groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
  QVBoxLayout* verticalLayout_4 = new QVBoxLayout(groupBox_2);
  verticalLayout_4->setSpacing(6);
  verticalLayout_4->setContentsMargins(11, 11, 11, 11);
  verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
  QCheckBox* m_cb_follow_task_view = new QCheckBox(groupBox_2);
  m_cb_follow_task_view->setObjectName(QString::fromUtf8("m_cb_follow_task_view"));
  m_cb_follow_task_view->setChecked(true);
  m_cb_follow_task_view->setTristate(false);

  verticalLayout_4->addWidget(m_cb_follow_task_view);

  QVBoxLayout* verticalLayout = new QVBoxLayout();
  verticalLayout->setSpacing(6);
  verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
  QSlider* m_task_view_zoom_slider = new QSlider(groupBox_2);
  m_task_view_zoom_slider->setObjectName(QString::fromUtf8("m_task_view_zoom_slider"));
  m_task_view_zoom_slider->setMinimum(1);
  m_task_view_zoom_slider->setMaximum(800);
  m_task_view_zoom_slider->setValue(100);
  m_task_view_zoom_slider->setOrientation(Qt::Horizontal);
  m_task_view_zoom_slider->setTickPosition(QSlider::TicksAbove);
  m_task_view_zoom_slider->setTickInterval(100);

  verticalLayout->addWidget(m_task_view_zoom_slider);


  verticalLayout_4->addLayout(verticalLayout);

  QPushButton* pushButton = new QPushButton(groupBox_2);
  pushButton->setObjectName(QString::fromUtf8("pushButton"));

  verticalLayout_4->addWidget(pushButton);


  verticalLayout_2->addWidget(groupBox_2);


  gridLayout_7->addWidget(groupBox, 0, 1, 1, 1);


  gridLayout_5->addWidget(groupBox_6, 0, 0, 1, 1);

  SDPAGUI->addTab(execution_tab, QString());

  gridLayout_9->addWidget(SDPAGUI, 0, 0, 1, 1);

  setCentralWidget(centralWidget);

  setMenuBar (new QMenuBar (this));

  QMenu* menuFile (menuBar()->addMenu (tr ("File")));
  menuFile->addAction (tr ("Save"), this, SLOT (save()), QKeySequence::Save);
  menuFile->addSeparator();
  menuFile->addAction (tr ("Exit"), this, SLOT (close()), QKeySequence::Quit);



        setWindowTitle(tr ("SDPA Graphical Monitor"));

        groupBox_3->setTitle(tr ("Event Log"));
        groupBox_5->setTitle(tr ("Filter"));
        m_level_filter_selector->clear();
        m_level_filter_selector->insertItems(0, QStringList()
         << tr ("Trace")
         << tr ("Debug")
         << tr ("Info")
         << tr ("Warn")
         << tr ("Error")
         << tr ("Fatal")
        );
        m_level_filter_selector->setToolTip(tr ("Filter events according to level"));
        groupBox_4->setTitle(tr ("Control"));
        clear_log_button->setToolTip(tr ("Clear all events"));
        clear_log_button->setText(tr ("Clear"));
        follow_logging_cb->setToolTip(tr ("Follow the stream of log events and automatically scroll the view, drop events otherwise"));
        follow_logging_cb->setText(tr ("follow"));
        SDPAGUI->setTabText(SDPAGUI->indexOf(logging_tab), tr ("Logging"));
        groupBox_6->setTitle(tr ("Activity Log"));
        groupBox->setTitle(tr ("Legend"));
        label_2->setText(tr ("Running"));
        label->setText(tr ("Finished"));
        label_3->setText(tr ("Failed"));
        groupBox_2->setTitle(tr ("Control"));
        m_cb_follow_task_view->setText(tr ("follow"));
        pushButton->setText(tr ("Clear"));
        SDPAGUI->setTabText(SDPAGUI->indexOf(execution_tab), tr ("Execution Monitor"));

        QObject::connect(clear_log_button, SIGNAL(clicked()), this, SLOT(clearLogging()));

        QObject::connect(follow_logging_cb, SIGNAL(toggled(bool)), this, SLOT(toggleFollowLogging(bool)));
        QObject::connect(m_level_filter_selector, SIGNAL(currentIndexChanged(int)), this, SLOT(levelFilterChanged(int)));
        QObject::connect(log_filter_dial, SIGNAL(valueChanged(int)), m_level_filter_selector, SLOT(setCurrentIndex(int)));
        QObject::connect(m_level_filter_selector, SIGNAL(currentIndexChanged(int)), log_filter_dial, SLOT(setValue(int)));
        QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(clearActivityLog()));
        QObject::connect(m_cb_follow_task_view, SIGNAL(toggled(bool)), this, SLOT(toggleFollowTaskView(bool)));
        QObject::connect(m_task_view_zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(changeTaskViewZoom(int)));

        SDPAGUI->setCurrentIndex(2);
        m_level_filter_selector->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(this);
    }



    m_scene = new QGraphicsScene (this);
    m_view = new QGraphicsView (m_scene);

    m_view->setTransformationAnchor (QGraphicsView::AnchorViewCenter);
    m_view->setAlignment (Qt::AlignRight | Qt::AlignTop);
    m_view->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_view->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_view->setDragMode (QGraphicsView::ScrollHandDrag);


    m_component_scene = new QGraphicsScene (this);
    m_component_view = new QGraphicsView (m_component_scene);

    {
      QSplitter *splitter = new QSplitter (Qt::Horizontal);
      QHBoxLayout *l = new QHBoxLayout;
      l->setSpacing(0);
      l->setContentsMargins(0,0,0,0);
      /*
      m_component_view->setSizePolicy( QSizePolicy( QSizePolicy::Minimum
                                                  , QSizePolicy::Minimum
                                                  )
                                     );
      m_view->setSizePolicy(QSizePolicy( QSizePolicy::Maximum
                                      , QSizePolicy::Maximum)
                          );
      m_view->setMinimumWidth(300);
      */
      splitter->addWidget(m_component_view);
      splitter->addWidget(m_view);
      splitter->setLayout(l);

      QList<int> sizes;
      sizes << 0 << 1;
      splitter->setSizes(sizes);

      l = new QHBoxLayout;
      l->addWidget(splitter);
      task_view_widget->setLayout(l);
    }

    m_component_view->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_component_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_component_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_component_view->setRenderHint(QPainter::Antialiasing);
    m_component_view->setTransformationAnchor(QGraphicsView::NoAnchor);

    //    m_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    //    m_view->setAlignment(Qt::AlignRight | Qt::AlignTop);
    //    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //m_view->setRenderHint(QPainter::Antialiasing);

    //    m_view->setCacheMode(QGraphicsView::CacheNone);
    //m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //m_view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    //    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //    m_view->setDragMode(QGraphicsView::ScrollHandDrag);

    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(advance()));
    QObject::connect( m_view->verticalScrollBar(), SIGNAL(valueChanged(int))
                    , m_component_view->verticalScrollBar(), SLOT(setValue(int))
                    );

    static const int updates_per_second (30);

    m_timer.start (1000 / updates_per_second);
}

MonitorWindow::~MonitorWindow()
{
  m_io_service.stop();
  m_io_thread.join();
}

void MonitorWindow::advance()
{
  scene_update_list_t updates;
  {
    lock_type lock (m_task_view_mutex);
    updates.swap (m_scene_updates);
  }

  BOOST_FOREACH (const scene_update_list_t::value_type& update, updates)
  {
    update.second->addItem (update.first);
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

  if (m_follow_execution)
  {
    m_view->horizontalScrollBar()->setValue
      (m_view->horizontalScrollBar()->maximum());
  }
}

namespace
{
  QColor severityToColor (const fhg::log::LogLevel lvl)
  {
    switch (lvl.lvl())
    {
    case fhg::log::LogLevel::TRACE:
      return QColor (205, 183, 158);
    case fhg::log::LogLevel::DEBUG:
      return QColor (139, 125, 107);
    case fhg::log::LogLevel::INFO:
      return QColor (25, 25, 25);
    case fhg::log::LogLevel::WARN:
      return QColor (255,140,0);
    case fhg::log::LogLevel::ERROR:
      return QColor (255,0,0);
    case fhg::log::LogLevel::FATAL:
      return QColor (165,42,42);
    default:
      return QColor (0,0,0);
    }
  }

  template <typename T>
    void decode (const std::string& strMsg, T& t)
  {
    std::stringstream sstr(strMsg);
    boost::archive::text_iarchive ar(sstr);
    ar >> t;
  }
}

void MonitorWindow::append_exe (fhg::log::LogEvent const &evt)
{
  sdpa::daemon::NotificationEvent notification;
  try
  {
    decode(evt.message(), notification);
  }
  catch (const std::exception &ex)
  {
    return;
  }

  if (notification.activity_state() != sdpa::daemon::NotificationEvent::STATE_IGNORE)
  {
    we::mgmt::type::activity_t act (notification.activity());

    UpdateExecutionView(notification, act);
  }
  else
  {
    //    qDebug() << "activity " << notification.activity_id().c_str() << " failed!";
  }
}

void MonitorWindow::UpdateExecutionView
  ( const sdpa::daemon::NotificationEvent& event
  , const we::mgmt::type::activity_t& activity
  )
{
  static const qreal task_height (8.0);

  const std::string& component (event.component());
  std::string activity_name (event.activity_name());
  const std::string& activity_id (event.activity_id());

  try
  {
    const we::type::module_call_t mod_call
      (boost::get<we::type::module_call_t> (activity.transition().data()));
    activity_name = mod_call.module() + ":" + mod_call.function();
  }
  catch (boost::bad_get const &)
  {
    // do nothing, i.e. take the activity name as it was
  }

  const lock_type lock (m_task_struct_mutex);

  const std::vector<std::string>::iterator comp
    (std::find (m_components.begin(), m_components.end(), component));

  const qreal x_coord (m_scene->width());
  const qreal y_coord ( comp != m_components.end()
                      ? std::distance (m_components.begin(), comp) * task_height
                      : m_components.size() * task_height
                      );

  if (comp == m_components.end())
  {
    m_components.push_back (component);

    QGraphicsSimpleTextItem* label
      (new QGraphicsSimpleTextItem (QString::fromStdString (component)));

    QFont font (label->font());
    font.setPointSize (6);
    label->setFont (font);
    label->setPos (0, y_coord);

    m_scene_updates.push_back (std::make_pair (label, m_component_scene));
  }

  if ( m_tasks_grid[component].find (activity_id)
     == m_tasks_grid[component].end()
     )
  {
    Task* task ( new Task ( QString::fromStdString (component)
                          , QString::fromStdString (activity_name)
                          , QString::fromStdString (activity_id)
                          )
               );
    task->setPos (x_coord, y_coord);
    m_tasks_grid[component][activity_id] = task;

    // new task, make sure to close previous task -> asume finished
    if (!m_tasks_list[component].empty())
    {
      m_tasks_list[component].back()->update_task_state
        (sdpa::daemon::NotificationEvent::STATE_FINISHED);
    }
    m_tasks_list[component].push_back (task);

    lock_type lock (m_task_view_mutex);
    m_scene_updates.push_back (std::make_pair (task, m_scene));
  }

  m_tasks_grid[component][activity_id]->update_task_state
    (event.activity_state());
}

void MonitorWindow::clearActivityLog()
{
  lock_type struct_lock(m_task_struct_mutex);

  delete m_scene; delete m_component_scene;
  m_scene = new QGraphicsScene (this);
  m_component_scene = new QGraphicsScene();

  m_view->setScene(m_scene);
  m_component_view->setScene(m_component_scene);

  m_tasks_grid.clear();
  m_tasks_list.clear();
  m_components.clear();

  {
    lock_type update_lock(m_task_view_mutex);
    while (!m_scene_updates.empty())
    {
      QGraphicsItem *t = m_scene_updates.front().first;
      delete t;
      m_scene_updates.pop_front();
    }
  }
}

static const int TABLE_COL_TIME = 0;
static const int TABLE_COL_SOURCE = 1;
static const int TABLE_COL_LOCATION = 2;
static const int TABLE_COL_MESSAGE = 3;

void MonitorWindow::append_log (fhg::log::LogEvent const &evt)
{
  if (not m_follow_logging)
  {
    return;
  }

  if ( evt.severity() < m_level_filter_selector->currentIndex ()
     && m_drop_filtered->isChecked()
     )
  {
    return;
  }

  // TODO:
  //    store in internal list -> data
  //    update view (different views)
  //    respect filtering etc.
  //    maximum number of events (circular buffer like)

  if (m_log_events.size () > (1 << 15))
  {
    clearLogging ();
  }

  m_log_events.push_back (evt);

  int row (m_log_table->rowCount ());
  m_log_table->setRowCount (row+1);

  QColor bg(severityToColor(evt.severity()));
  QBrush fg(severityToColor(evt.severity()));

  {
    char buf[128]; memset (buf, 0, sizeof(buf));
    time_t tm = (evt.tstamp());
    ctime_r (&tm, buf);

    QTableWidgetItem *i(new QTableWidgetItem (buf));
    i->setForeground(fg);
    m_log_table->setItem(row, TABLE_COL_TIME, i);
  }

  {
    std::ostringstream sstr;
    sstr << evt.pid() << "@" << evt.logged_on();
    std::string logged_on;
    QTableWidgetItem *i(new QTableWidgetItem (sstr.str().c_str()));
    i->setForeground(fg);
    m_log_table->setItem(row, TABLE_COL_SOURCE, i);
  }

  {
    QTableWidgetItem *i
        (new QTableWidgetItem (( evt.file ()
                               + ":"
                               + boost::lexical_cast<std::string>(evt.line())).c_str()
                               )
        );
    i->setForeground (fg);
    m_log_table->setItem (row, TABLE_COL_LOCATION, i);
  }
  {
    QTableWidgetItem *i
        (new QTableWidgetItem (evt.message().c_str()));
    i->setForeground (fg);
    m_log_table->setItem (row, TABLE_COL_MESSAGE, i);
  }
  if (m_follow_logging)
    m_log_table->scrollToBottom ();
  m_log_table->resizeRowToContents (row);

  if (evt.severity() < m_level_filter_selector->currentIndex())
    m_log_table->setRowHidden (row, true);
  else
    m_log_table->setRowHidden (row, false);
}

void
MonitorWindow::handle_external_event ( int type
                                     , const fhg::log::LogEvent & evt
                                     )
{
  QApplication::postEvent (this, new LogEventWrapper(type, evt));
}

bool MonitorWindow::event (QEvent* event)
{
  switch (event->type())
  {
  case EXTERNAL_EVENT_LOGGING:
    event->accept();
    append_log (static_cast<LogEventWrapper*> (event)->log_event);
    return true;

  default:
    return QWidget::event (event);
  }
}

void MonitorWindow::clearLogging ()
{
        m_log_events.clear ();
        m_log_table->clearContents ();
        m_log_table->setRowCount (0);
}

void MonitorWindow::toggleFollowLogging (bool follow)
{
  m_follow_logging = follow;
}

void MonitorWindow::toggleFollowTaskView (bool follow)
{
  m_follow_execution = follow;
}

void MonitorWindow::levelFilterChanged (int lvl)
{
  for (size_t i = 0; i < m_log_events.size (); ++i)
    if (m_log_events[i].severity() < m_level_filter_selector->currentIndex())
      m_log_table->setRowHidden (i, true);
    else
      m_log_table->setRowHidden (i, false);
}

void MonitorWindow::changeTaskViewZoom(int to)
{
  qreal target = (to / 100.0);
  qreal factor = target / m_current_scale;

  m_view->scale (factor, factor);
  m_component_view->scale (factor, factor);

  m_current_scale = target;
}

void MonitorWindow::save ()
{
  QString fname = QFileDialog::getSaveFileName ( this
                                               , "Save log messages"
                                               , m_logfile
                                               );

  if (fname.isEmpty ())
    return;

  try
  {
    std::ofstream ofs (fname.toStdString ().c_str ());
    boost::archive::text_oarchive oa (ofs);
    oa & m_log_events;
    m_logfile = fname;
  }
  catch (std::exception const & ex)
  {
    QMessageBox::critical (this, "Could not save file", ex.what ());
  }
}
