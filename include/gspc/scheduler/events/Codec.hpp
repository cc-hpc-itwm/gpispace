// Copyright (C) 2010,2013,2015,2019,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>


  namespace gspc::scheduler::events
  {
    class SchedulerEvent;

    class Codec
    {
    public:
      template<typename Event, typename... Args>
        std::string encode (Args&&... args) const;

      std::string encode (gspc::scheduler::events::SchedulerEvent const* e) const;
      gspc::scheduler::events::SchedulerEvent* decode (std::string const& s) const;
    };
  }


#include <gspc/scheduler/events/Codec.ipp>
