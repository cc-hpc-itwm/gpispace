// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/scoped_disconnect.hpp>

#include <QObject>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      scoped_disconnect::scoped_disconnect ( const QObject* const sender
                                           , const char* const signal
                                           , const QObject* const receiver
                                           , const char* const method
                                           )
        : _sender (sender)
        , _signal (signal)
        , _receiver (receiver)
        , _method (method)
        , _was_disconnected
            (QObject::disconnect (_sender, _signal, _receiver, _method))
      {
      }

      scoped_disconnect::~scoped_disconnect()
      {
        if (_was_disconnected)
        {
          QObject::connect (_sender, _signal, _receiver, _method);
        }
      }
    }
  }
}
