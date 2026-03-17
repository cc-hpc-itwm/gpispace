// Copyright (C) 2010,2013-2015,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/EventHandler.hpp>

#include <gspc/com/address.hpp>

#include <boost/serialization/access.hpp>
#include <memory>


  namespace gspc::scheduler::events
  {
    class SchedulerEvent
    {
    public:
      using Ptr = std::shared_ptr<SchedulerEvent>;

      virtual ~SchedulerEvent() = default;
      SchedulerEvent (SchedulerEvent const&) = delete;
      SchedulerEvent (SchedulerEvent&&) = default;
      SchedulerEvent& operator= (SchedulerEvent const&) = delete;
      SchedulerEvent& operator= (SchedulerEvent&&) = delete;

      virtual void handleBy
        (gspc::com::p2p::address_t const& source, EventHandler*) = 0;

    protected:
      SchedulerEvent() = default;

    private:
      friend class ::boost::serialization::access;
      template <class Archive>
      void serialize (Archive &, unsigned int)
      {
      }
    };
  }


#include <gspc/scheduler/events/Serialization.hpp>
