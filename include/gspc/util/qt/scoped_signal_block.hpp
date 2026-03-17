// Copyright (C) 2012-2013,2015,2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>



    namespace gspc::util::qt
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
