// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/parallel_reduce/module_call/JobExecutor.hpp>

#include <test/parallel_reduce/module_call/remote_function/Client.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace gspc
{
  namespace test
  {
    namespace parallel_reduce
    {
      namespace module_call
      {
        void JobExecutor::release (Task task, std::string worker_name)
        {
          if (! (task == _task && worker_name == _worker_name))
          {
            throw std::invalid_argument
              (str ( boost::format
                       ("JobExecutor: wrong release: {%1%,%2%} != {%3%,%4%}")
                   % task
                   % worker_name
                   % _task
                   % _worker_name
                   )
              );
          }

          _released.set_value();
        }

        JobExecutor::JobExecutor
          ( std::pair<std::string, unsigned short> job_server
          , Task task
          , std::string worker_name
          )
            : _task {task}
            , _worker_name {worker_name}
        {
          remote_function::Client<protocol::running> {job_server}
            (task, _worker_name, _provider.address());
        }

        void JobExecutor::wait_for_release() &&
        {
          return _released.get_future().get();
        }
      }
    }
  }
}
