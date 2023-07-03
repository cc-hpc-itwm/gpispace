// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QObject>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      //! RAII wrapper for QObject::blockSignals(): Block signals of a
      //! given object until end of scope. Correctly handles nested
      //! blocking.
      struct scoped_signal_block
      {
        scoped_signal_block (QObject*);
        ~scoped_signal_block();

        scoped_signal_block() = delete;
        scoped_signal_block (scoped_signal_block const&) = delete;
        scoped_signal_block (scoped_signal_block&&) = delete;
        scoped_signal_block& operator= (scoped_signal_block const&) = delete;
        scoped_signal_block& operator= (scoped_signal_block&&) = delete;

      private:
        QObject* _object;
        bool _old_state;
      };
    }
  }
}

#include <util-qt/scoped_signal_block.ipp>
