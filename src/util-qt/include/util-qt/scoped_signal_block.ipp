// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      inline scoped_signal_block::scoped_signal_block (QObject* object)
        : _object (object)
        , _old_state (_object->blockSignals (true))
      {}
      inline scoped_signal_block::~scoped_signal_block()
      {
        _object->blockSignals (_old_state);
      }
    }
  }
}
