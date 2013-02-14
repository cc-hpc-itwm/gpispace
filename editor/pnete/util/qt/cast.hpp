// bernd.loerwald@itwm.fraunhofer.de

#ifndef UTIL_QT_CAST_HPP
#define UTIL_QT_CAST_HPP

#include <QWidget>
#include <QObject>
#include <QGraphicsItem>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T> T throwing_qobject_cast (QObject* from)
      {
        T x (qobject_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qobject_cast failed");
        }

        return x;
      }
      template<typename T> T throwing_qobject_cast (const QObject* from)
      {
        T x (qobject_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qobject_cast failed");
        }

        return x;
      }
      template<typename T> T throwing_qgraphicsitem_cast (QGraphicsItem* from)
      {
        T x (qgraphicsitem_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qgraphicsitem_cast failed");
        }

        return x;
      }
    }

    class scoped_signal_block
    {
    private:
      QObject* _ob;
      const bool _old;
    public:
      scoped_signal_block (QObject* ob)
        : _ob (ob)
        , _old (_ob->blockSignals (true))
      {}
      ~scoped_signal_block()
      {
        _ob->blockSignals (_old);
      }
    };
  }
}

#endif
