#include <gspc/util/qt/do_after_event_loop.hpp>

#include <QtCore/QTimer>



    namespace gspc::util::qt
    {
      void do_after_event_loop (std::function<void()> fun)
      {
        auto* timer (new QTimer);
        timer->setSingleShot (true);
        QObject::connect ( timer
                         , &QTimer::timeout
                         , [fun, timer]
                           {
                             fun();
                             timer->deleteLater();
                           }
                         );
        timer->start();
      }
    }
