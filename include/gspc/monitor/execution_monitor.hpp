// Copyright (C) 2011,2013-2015,2018-2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/message.hpp>

#include <QSplitter>

#include <vector>



    namespace gspc::monitor
    {
      class worker_model;

      class execution_monitor : public QSplitter
      {
        Q_OBJECT

      public:
        execution_monitor();

        void append_event (gspc::logging::message const&);

      private:
        worker_model* base;
      };
    }
