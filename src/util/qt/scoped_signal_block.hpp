#pragma once

#include <QObject>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class scoped_signal_block
      {
      private:
        QObject* _object;
        const bool _old_state;
      public:
        scoped_signal_block (QObject* object)
          : _object (object)
          , _old_state (_object->blockSignals (true))
        {}
        ~scoped_signal_block()
        {
          _object->blockSignals (_old_state);
        }
      };
    }
  }
}
