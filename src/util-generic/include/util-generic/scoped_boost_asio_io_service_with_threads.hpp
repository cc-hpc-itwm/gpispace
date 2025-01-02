// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <list>

namespace fhg
{
  namespace util
  {
    struct scoped_boost_asio_io_service_with_threads : ::boost::asio::io_service
    {
    public:
      scoped_boost_asio_io_service_with_threads (std::size_t count)
        : ::boost::asio::io_service (count)
        , _threads()
        , _work (*this)
      {
        while (count --> 0)
        {
          _threads.emplace_back ([this] { run(); });
        }
      }

    private:
      std::list<::boost::strict_scoped_thread<>> _threads;
      ::boost::asio::io_service::work _work;
    };

    struct scoped_boost_asio_io_service_with_threads_and_deferred_startup
       : ::boost::asio::io_service
    {
    public:
      scoped_boost_asio_io_service_with_threads_and_deferred_startup
          (std::size_t count)
        : ::boost::asio::io_service (count)
        , _count (count)
        , _threads()
        , _work (*this)
      {
        if (count == 0)
        {
          throw std::invalid_argument
            ( "count shall be > 0 due to start_in_threads_and_current_thread()"
              " invoking at least one run() in the current thread."
            );
        }
        notify_fork (::boost::asio::io_service::fork_prepare);
      }

      void post_fork_child()
      {
        notify_fork (::boost::asio::io_service::fork_child);
      }
      void post_fork_parent()
      {
        notify_fork (::boost::asio::io_service::fork_parent);
      }

      void start_in_threads_and_current_thread()
      {
        while (_count --> 1)
        {
          _threads.emplace_back ([this] { run(); });
        }

        run();
      }

    private:
      std::size_t _count;
      std::list<::boost::strict_scoped_thread<>> _threads;
      ::boost::asio::io_service::work _work;
    };
  }
}
