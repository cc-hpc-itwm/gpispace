// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
        scoped_signal_block (scoped_signal_block const&) = delete;
        scoped_signal_block (scoped_signal_block&&) = delete;
        scoped_signal_block& operator= (scoped_signal_block const&) = delete;
        scoped_signal_block& operator= (scoped_signal_block&&) = delete;
      };
    }
  }
}
